// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#include "WFCTileAsset3DHex.h"

#include "WFCTileAsset2D.h"
#include "Core/Grids/WFCGrid3DHex.h" // 需要引用 Grid 的静态数学函数
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "WFCEditor"

UWFCTileAsset3DHex::UWFCTileAsset3DHex()
    : Dimensions(FIntVector(1, 1, 1)),
      RotationType(EWFCHexRotationType::Six)
{
}

FWFCTileDef3DHex UWFCTileAsset3DHex::GetTileDefByLocation(FIntVector Location, int32& Index) const
{
    Index = INDEX_NONE;
    for (int32 Idx = 0; Idx < TileDefs.Num(); ++Idx)
    {
        if (TileDefs[Idx].Location == Location)
        {
            Index = Idx;
            return TileDefs[Idx];
        }
    }
    return FWFCTileDef3DHex();
}

FWFCTileDef3DHex UWFCTileAsset3DHex::GetTileDefByIndex(int32 Index) const
{
    return TileDefs.IsValidIndex(Index) ? TileDefs[Index] : FWFCTileDef3DHex();
}

int32 UWFCTileAsset3DHex::GetNumRotations() const
{
    switch (RotationType)
    {
    case EWFCHexRotationType::One: return 1;
    case EWFCHexRotationType::Two: return 2;
    case EWFCHexRotationType::Three: return 3;
    case EWFCHexRotationType::Six: return 6;
    default: return 6;
    }
}

void UWFCTileAsset3DHex::GetAllowedRotations(TArray<int32>& OutRotations) const
{
    OutRotations.Empty();
    switch (RotationType)
    {
    case EWFCHexRotationType::One:
        OutRotations = {0};
        break;
    case EWFCHexRotationType::Two:
        OutRotations = {0, 3}; // 0, 180度
        break;
    case EWFCHexRotationType::Three:
        OutRotations = {0, 2, 4}; // 0, 120, 240度
        break;
    case EWFCHexRotationType::Six:
    default:
        OutRotations = {0, 1, 2, 3, 4, 5}; // 所有60度倍数
        break;
    }
}

FGameplayTag UWFCTileAsset3DHex::GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const
{
    check(TileDefs.IsValidIndex(TileDefIndex));
    // 转换 Direction int 到 Enum，注意越界
    if (Direction >= 0 && Direction < (int32)EWFCTile3DHexEdge::MAX)
    {
        const EWFCTile3DHexEdge Edge = static_cast<EWFCTile3DHexEdge>(Direction);
        if (TileDefs[TileDefIndex].EdgeTypes.Contains(Edge))
        {
            return TileDefs[TileDefIndex].EdgeTypes[Edge];
        }
    }
    return FGameplayTag::EmptyTag;
}

int32 UWFCTileAsset3DHex::GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const
{
    check(TileDefs.IsValidIndex(TileDefIndex));
    
    FIntVector CurrentOffset = TileDefs[TileDefIndex].Location;

    // 核心难点：在 Hex Grid 中，Offset 坐标的邻居计算依赖于 Y 坐标的奇偶性。
    // 但是 UWFCGrid3DHex::GetDirectionVectorCube 是基于 Cube 坐标的，是通用的。
    
    // 垂直方向
    if (Direction == 6) // Top
    {
        //return GetTileDefByLocation(CurrentOffset + FIntVector(0, 0, 1), TileDefIndex).Location == (CurrentOffset + FIntVector(0, 0, 1)) ? TileDefIndex : INDEX_NONE;
        // Bugfix above: GetTileDefByLocation returns struct by value, need to check if valid.
        // Actually simpler:
        int32 FoundIdx;
        GetTileDefByLocation(CurrentOffset + FIntVector(0,0,1), FoundIdx);
        return FoundIdx;
    }
    if (Direction == 7) // Bottom
    {
        int32 FoundIdx;
        GetTileDefByLocation(CurrentOffset + FIntVector(0,0,-1), FoundIdx);
        return FoundIdx;
    }

    // 水平方向
    // 1. 转为 Cube
    FIntVector Cube = UWFCGrid3DHex::OffsetToCube(CurrentOffset);
    // 2. 获取方向增量
    FIntVector DirVec = UWFCGrid3DHex::GetDirectionVectorCube(Direction);
    // 3. 相加
    FIntVector NextCube = Cube + DirVec;
    // 4. 转回 Offset
    FIntVector NextOffset2D = UWFCGrid3DHex::CubeToOffset(NextCube);
    FIntVector NextOffset = FIntVector(NextOffset2D.X, NextOffset2D.Y, CurrentOffset.Z);

    int32 FoundIdx;
    GetTileDefByLocation(NextOffset, FoundIdx);
    return FoundIdx;
}

TArray<FActorInfoEntry> UWFCTileAsset3DHex::GetTileDefActorsInfo(int32 TileDefIndex) const
{
    check(TileDefs.IsValidIndex(TileDefIndex));
    return TileDefs[TileDefIndex].ActorsInfo;
}

TArray<FStaticMeshInfoEntry> UWFCTileAsset3DHex::GetTileDefMeshesInfo(int32 TileDefIndex) const
{
    check(TileDefs.IsValidIndex(TileDefIndex));
    return TileDefs[TileDefIndex].StaticMeshesInfo;
}

TSubclassOf<UWFCUnit> UWFCTileAsset3DHex::GetTileDefUnit(int32 TileDefIndex) const
{
    check(TileDefs.IsValidIndex(TileDefIndex));
    return TileDefs[TileDefIndex].WFCUnit;
}

bool UWFCTileAsset3DHex::IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const
{
    // 如果这个方向上有定义的 TileDef，那就是内部边
    return GetTileDefInDirection(TileDefIndex, Direction) != INDEX_NONE;
}

#if WITH_EDITOR
EDataValidationResult UWFCTileAsset3DHex::IsDataValid(FDataValidationContext& Context)
{
    EDataValidationResult Result = EDataValidationResult::Valid;

    // 检查是否有重复位置的 TileDef
    TSet<FIntVector> Locations;
    for (const auto& Def : TileDefs)
    {
        if (Locations.Contains(Def.Location))
        {
            Context.AddError(LOCTEXT("DuplicateLocation", "Found duplicate tile definition locations."));
            Result = EDataValidationResult::Invalid;
        }
        Locations.Add(Def.Location);
    }

    return Result;
}
#endif

#undef LOCTEXT_NAMESPACE