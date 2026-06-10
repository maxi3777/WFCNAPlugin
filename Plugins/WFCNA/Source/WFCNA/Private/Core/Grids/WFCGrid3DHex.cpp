// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Grids/WFCGrid3DHex.h"

#include "Core/Grids/AsyncGrid3DHex.h"

UWFCGrid3DHexConfig::UWFCGrid3DHexConfig()
    : Dimensions(FIntVector(10, 10, 5)),
      CellSize(FVector(100.f, 100.f, 100.f)) // X=Width, Y=Height
{
    GridClass = UWFCGrid3DHex::StaticClass();
}

UWFCGrid3DHex::UWFCGrid3DHex()
    : Dimensions(FIntVector(10, 10, 5)),
      CellSize(FVector(100.f, 100.f, 100.f))
{
}

void UWFCGrid3DHex::Initialize(const UWFCGridConfig* Config)
{
    Super::Initialize(Config);
    const UWFCGrid3DHexConfig* ConfigHex = Cast<UWFCGrid3DHexConfig>(Config);
    check(ConfigHex != nullptr);
    Dimensions = ConfigHex->Dimensions;
    CellSize = ConfigHex->CellSize;
}

int32 UWFCGrid3DHex::GetNumCells() const
{
    return Dimensions.X * Dimensions.Y * Dimensions.Z;
}

FString UWFCGrid3DHex::GetDirectionName(int32 Direction) const
{
    static const FString Names[] = {
        TEXT("East"), TEXT("SouthEast"), TEXT("SouthWest"), 
        TEXT("West"), TEXT("NorthWest"), TEXT("NorthEast"), 
        TEXT("Top"), TEXT("Bottom")
    };
    if (Direction >= 0 && Direction < 8) return Names[Direction];
    return TEXT("Unknown");
}

FString UWFCGrid3DHex::GetCellName(int32 CellIndex) const
{
    const FIntVector Loc = GetLocationForCellIndex(CellIndex);
    // 同时显示 Offset 和 Cube 方便调试
    FIntVector Cube = OffsetToCube(Loc);
    return FString::Printf(TEXT("Off(%d,%d,%d)|Cube(%d,%d,%d)"), Loc.X, Loc.Y, Loc.Z, Cube.X, Cube.Y, Cube.Z);
}

int32 UWFCGrid3DHex::GetOppositeDirection(FWFCGridDirection Direction) const
{
    if (Direction < 6) return (Direction + 3) % 6; // 水平方向对侧
    if (Direction == 6) return 7; // Top -> Bottom
    if (Direction == 7) return 6; // Bottom -> Top
    return INDEX_NONE;
}

FWFCGridDirection UWFCGrid3DHex::RotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
    if (!IsValidDirection(Direction)) return Direction;
    if (Direction >= 6) return Direction; // 垂直方向不受 Yaw 旋转影响
    
    // 0-5 顺时针旋转
    return (Direction + Rotation) % 6;
}

FWFCGridDirection UWFCGrid3DHex::InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const
{
    if (!IsValidDirection(Direction) || Direction >= 6) return Direction;
    // 逆旋转
    int32 InvRot = (6 - (Rotation % 6)) % 6;
    return (Direction + InvRot) % 6;
}

int32 UWFCGrid3DHex::CombineRotations(int32 RotationA, int32 RotationB) const
{
    return (RotationA + RotationB) % 6;
}

FIntVector UWFCGrid3DHex::GetDirectionVectorCube(int32 Direction)
{
    // Cube: X=q, Y=r, Z=s (约束: q+r+s=0)
    // 对应 Odd-r Pointy Top 的方向定义
    switch (Direction)
    {
    case 0: return FIntVector(1, 0, -1);  // East
    case 1: return FIntVector(0, 1, -1);  // SouthEast
    case 2: return FIntVector(-1, 1, 0);  // SouthWest
    case 3: return FIntVector(-1, 0, 1);  // West
    case 4: return FIntVector(0, -1, 1);  // NorthWest
    case 5: return FIntVector(1, -1, 0);  // NorthEast
    // 垂直方向仅用于标识，不参与 Cube 平面运算，这里返回 Zero
    default: return FIntVector(0, 0, 0); 
    }
}

FIntVector UWFCGrid3DHex::OffsetToCube(const FIntVector& Offset)
{
    // Odd-r conversion
    // q = col - (row - (row&1)) / 2
    // r = row
    // s = -q - r
    int32 q = Offset.X - (Offset.Y - (Offset.Y & 1)) / 2;
    int32 r = Offset.Y;
    int32 s = -q - r;
    return FIntVector(q, r, s);
}

FIntVector UWFCGrid3DHex::CubeToOffset(const FIntVector& Cube)
{
    // Odd-r conversion
    // col = q + (r - (r&1)) / 2
    // row = r
    int32 col = Cube.X + (Cube.Y - (Cube.Y & 1)) / 2;
    int32 row = Cube.Y;
    // Z 轴透传
    return FIntVector(col, row, 0); 
}

FIntVector UWFCGrid3DHex::RotateCube(const FIntVector& Cube, int32 RotationSteps)
{
    // 在六边形 Cube 坐标中，顺时针旋转 60度 相当于坐标分量交换并取反
    // (q, r, s) -> (-r, -s, -q)
    
    int32 q = Cube.X;
    int32 r = Cube.Y;
    int32 s = Cube.Z;

    int32 Steps = RotationSteps % 6;
    for(int32 i=0; i<Steps; ++i)
    {
        int32 new_q = -r;
        int32 new_r = -s;
        int32 new_s = -q;
        q = new_q; r = new_r; s = new_s;
    }
    return FIntVector(q, r, s);
}

FWFCCellIndex UWFCGrid3DHex::GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const
{
    if (!IsValidCellIndex(CellIndex) || !IsValidDirection(Direction)) return INDEX_NONE;

    FIntVector CurrentOffset = GetLocationForCellIndex(CellIndex);

    // 处理垂直方向 (简单 Z 轴加减)
    if (Direction == 6) // Top
    {
        FIntVector Next = CurrentOffset + FIntVector(0, 0, 1);
        return GetCellIndexForLocation(Next);
    }
    if (Direction == 7) // Bottom
    {
        FIntVector Next = CurrentOffset + FIntVector(0, 0, -1);
        return GetCellIndexForLocation(Next);
    }

    // 处理水平方向 (需要转 Cube -> 加向量 -> 转回 Offset)
    FIntVector Cube = OffsetToCube(CurrentOffset);
    FIntVector DirVec = GetDirectionVectorCube(Direction);
    FIntVector NextCube = Cube + DirVec; // 注意：这里 FIntVector 只有 XYZ，Cube 的 s 分量存在 Z 里，加法是分量相加，逻辑成立
    
    FIntVector NextOffset2D = CubeToOffset(NextCube);
    FIntVector NextOffset = FIntVector(NextOffset2D.X, NextOffset2D.Y, CurrentOffset.Z);

    return GetCellIndexForLocation(NextOffset);
}

int32 UWFCGrid3DHex::GetCellIndexForLocation(FIntVector GridLocation) const
{
    if (GridLocation.X < 0 || GridLocation.X >= Dimensions.X ||
        GridLocation.Y < 0 || GridLocation.Y >= Dimensions.Y ||
        GridLocation.Z < 0 || GridLocation.Z >= Dimensions.Z)
    {
        return INDEX_NONE;
    }
    return GridLocation.X + (GridLocation.Y * Dimensions.X) + (GridLocation.Z * Dimensions.X * Dimensions.Y);
}

FIntVector UWFCGrid3DHex::GetLocationForCellIndex(int32 CellIndex) const
{
    const int32 DimXY = Dimensions.X * Dimensions.Y;
    const int32 Z = CellIndex / DimXY;
    const int32 Remainder = CellIndex % DimXY;
    const int32 Y = Remainder / Dimensions.X;
    const int32 X = Remainder % Dimensions.X;
    return FIntVector(X, Y, Z);
}

FVector UWFCGrid3DHex::GetCellWorldLocation(int32 CellIndex, bool bCenter) const
{
    if (!IsValidCellIndex(CellIndex)) return FVector::ZeroVector;

    FIntVector Offset = GetLocationForCellIndex(CellIndex);
    
    // Odd-r Pointy Top 布局计算
    // X spacing = Width (CellSize.X)
    // Y spacing = Height * 0.75 (CellSize.Y * 0.75)
    
    float XPos = Offset.X * CellSize.X;
    float YPos = Offset.Y * (CellSize.Y * 0.75f);

    // 如果是奇数行，X 偏移半个宽度
    if (Offset.Y % 2 != 0)
    {
        XPos += CellSize.X * 0.5f;
    }

    float ZPos = Offset.Z * CellSize.Z;

    FVector WorldPos(XPos, YPos, ZPos);

    if (bCenter)
    {
        // 这里的 Center 定义为几何中心，Z 轴加一半高度
        WorldPos.Z += CellSize.Z * 0.5f;
        // XY 已经是中心点坐标逻辑 (Pointy Top 的 origin 通常在中心)
    }
    
    return WorldPos;
}

FTransform UWFCGrid3DHex::GetCellWorldTransform(int32 CellIndex, int32 Rotation) const
{
    FTransform Result = GetRotationTransform(Rotation);
    
    // 这里不能简单使用 FVector * Location，因为六边形不是正交网格
    FVector WorldLoc = GetCellWorldLocation(CellIndex, false); // false = Origin, not Center? 
    // 注意：GetCellWorldLocation 计算的是六边形中心点在世界空间的坐标
    // 通常 Tile 的 Pivot 也在中心点
    
    Result.SetTranslation(WorldLoc);
    return Result;
}

FTransform UWFCGrid3DHex::GetRotationTransform(int32 Rotation) const
{
    // 六边形旋转：每单位 60 度
    const float Angle = Rotation * 60.0f;
    const FRotator Rot(0.f, Angle, 0.f);
    return FTransform(Rot);
}

FIntVector UWFCGrid3DHex::GetDirectionVector(int32 Direction) const
{
    // 仅用于接口兼容，不建议直接使用，因为 Hex 的 Offset 坐标增量依赖于行号的奇偶性
    // 这里返回 Zero 强迫调用者使用 GetCellIndexInDirection
    return FIntVector::ZeroValue;
}

FVector UWFCGrid3DHex::GetCenterOffsetToOrigin(FIntVector GridLocation) const
{
    // 计算目标的实际世界坐标 (基于Odd-r布局)
    float XPos = GridLocation.X * CellSize.X;
    float YPos = GridLocation.Y * (CellSize.Y * 0.75f);
	
    // 注意：如果Y是负数，C++的 % 运算可能会返回负数，因此加上 FMath::Abs
    if (FMath::Abs(GridLocation.Y) % 2 == 1)
    {
        XPos += CellSize.X * 0.5f;
    }
    float ZPos = GridLocation.Z * CellSize.Z;

    // 偏移 = 原点中心(0) - 目标中心(Pos)
    return FVector(-XPos, -YPos, -ZPos);
}

TSharedPtr<FAsyncGrid> UWFCGrid3DHex::CreateAsyncGrid()
{
    return MakeShared<FAsyncGrid3DHex>(Dimensions);
}
