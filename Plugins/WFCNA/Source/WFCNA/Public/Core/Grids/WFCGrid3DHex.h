// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "Core/WFCGrid.h"
#include "WFCGrid3DHex.generated.h"

/**
 * 六棱柱网格配置 (Odd-r layout)
 */
UCLASS()
class WFCNA_API UWFCGrid3DHexConfig : public UWFCGridConfig
{
    GENERATED_BODY()

public:
    UWFCGrid3DHexConfig();

    /** 网格尺寸 (Columns, Rows, Layers) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntVector Dimensions;

    /** 
     * 单个六棱柱的物理尺寸
     * X: Width (六边形宽度, Pointy-top模式下为边对边距离)
     * Y: Height (六边形高度, Pointy-top模式下为角对角距离)
     * Z: Prism Height (棱柱高度)
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CellSize;
};

/**
 * 六棱柱网格逻辑实现
 * 使用 Offset coordinates "odd-r" layout (推移奇数行)
 */
UCLASS()
class WFCNA_API UWFCGrid3DHex : public UWFCGrid
{
    GENERATED_BODY()

public:
    UWFCGrid3DHex();

    virtual void Initialize(const UWFCGridConfig* Config) override;

    UPROPERTY(BlueprintReadWrite)
    FIntVector Dimensions;

    UPROPERTY(BlueprintReadWrite)
    FVector CellSize;

    // --- WFCGrid Interface ---
    virtual int32 GetNumCells() const override;
    FORCEINLINE virtual int32 GetNumDirections() const override { return 8; } // 6 Horizontal + 2 Vertical
    virtual FString GetDirectionName(int32 Direction) const override;
    virtual FString GetCellName(int32 CellIndex) const override;
    virtual FWFCGridDirection GetOppositeDirection(FWFCGridDirection Direction) const override;
    virtual FWFCGridDirection RotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
    virtual FWFCGridDirection InverseRotateDirection(FWFCGridDirection Direction, int32 Rotation) const override;
    virtual int32 CombineRotations(int32 RotationA, int32 RotationB) const override;
    virtual FWFCCellIndex GetCellIndexInDirection(FWFCCellIndex CellIndex, FWFCGridDirection Direction) const override;
    virtual FVector GetCellWorldLocation(int32 CellIndex, bool bCenter) const override;
    virtual FTransform GetCellWorldTransform(int32 CellIndex, int32 Rotation) const override;
    virtual FTransform GetRotationTransform(int32 Rotation) const override;
    virtual FIntVector GetDirectionVector(int32 Direction) const override; // 返回的是 Offset 坐标系的 Delta 吗？不，这里返回 Cube 坐标更安全用于逻辑

    /** Use for Nesting Generation */
    virtual FVector GetCenterOffsetToOrigin(FIntVector GridLocation) const override;
    
    // --- Hex Specific Helpers ---
    
    UFUNCTION(BlueprintPure)
    int32 GetCellIndexForLocation(FIntVector GridLocation) const;

    UFUNCTION(BlueprintPure)
    FIntVector GetLocationForCellIndex(int32 CellIndex) const;

    /** Offset (Col, Row) 转 Cube (q, r, s) */
    static FIntVector OffsetToCube(const FIntVector& Offset);

    /** Cube (q, r, s) 转 Offset (Col, Row) */
    static FIntVector CubeToOffset(const FIntVector& Cube);

    /** 旋转一个 Cube 坐标 (CW) */
    static FIntVector RotateCube(const FIntVector& Cube, int32 RotationSteps);

    /** 获取某个方向的 Cube 坐标增量 */
    static FIntVector GetDirectionVectorCube(int32 Direction);
    
    // --- Create AsyncGrid ---
    virtual TSharedPtr<FAsyncGrid> CreateAsyncGrid() override;
};