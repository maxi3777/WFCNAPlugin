// Copyright Bohdon Sayre. All Rights Reserved.
// Modified portions Copyright maxi3777, 2026

#pragma once

#include "CoreMinimal.h"
#include "WFCTileAsset.h" 
#include "WFCTileAsset2D.h"
#include "WFCTileDefType.h"
#include "WFCTileAsset3DHex.generated.h"


UENUM(BlueprintType)
enum class EWFCHexRotationType : uint8
{
    /** 不旋转 (0°) */
    One,
    /** 中心对称 (0°, 180°) */
    Two,
    /** 三角对称 (0°, 120°, 240°) */
    Three,
    /** 全旋转 (0°, 60°, 120°, 180°, 240°, 300°) */
    Six
};

/**
 * 六棱柱的8个邻居方向
 * 0-5: 水平六边形方向 (E, SE, SW, W, NW, NE)
 * 6: 上 (+Z)
 * 7: 下 (-Z)
 */

UENUM(BlueprintType)
enum class EWFCTile3DHexEdge : uint8
{
    East      UMETA(DisplayName = "+X"),
    SouthEast UMETA(DisplayName = "+X+Y"),
    SouthWest UMETA(DisplayName = "-X+Y"),
    West      UMETA(DisplayName = "-X"),
    NorthWest UMETA(DisplayName = "-X-Y"),
    NorthEast UMETA(DisplayName = "+X-Y"),
    Top       UMETA(DisplayName = "+Z"),
    Bottom    UMETA(DisplayName = "-Z"),
    MAX       UMETA(Hidden),
};



USTRUCT(BlueprintType)
struct FWFCTileDef3DHex
{
    GENERATED_BODY()

    FWFCTileDef3DHex()
    : Location(FIntVector::ZeroValue)
    {
        EdgeTypes = {
            {EWFCTile3DHexEdge::East, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::SouthEast, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::SouthWest, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::West, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::NorthWest, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::NorthEast, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::Top, FGameplayTag::EmptyTag},
            {EWFCTile3DHexEdge::Bottom, FGameplayTag::EmptyTag},
        };
    }

    /** 该部分在大型 Tile 中的局部 Offset 坐标 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FIntVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FActorInfoEntry> ActorsInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FStaticMeshInfoEntry> StaticMeshesInfo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UWFCUnit> WFCUnit;

    /** 8个方向的边缘类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Categories = "WFC.EdgeType"))
    TMap<EWFCTile3DHexEdge, FGameplayTag> EdgeTypes;
};

/**
 * 适用于六棱柱网格的地块资产
 */
UCLASS(BlueprintType)
class WFCNA_API UWFCTileAsset3DHex : public UWFCTileAsset
{
    GENERATED_BODY()

public:
    UWFCTileAsset3DHex();

    /** 大型 Tile 的尺寸 (Offset Coordinates) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    FIntVector Dimensions;

    /** 旋转对称性设置 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWFCHexRotationType RotationType;

    /** Tile 定义列表 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "Location"))
    TArray<FWFCTileDef3DHex> TileDefs;

    // --- Interface ---
    
    /** 根据 Offset 坐标获取 TileDef (需要遍历查找) */
    FWFCTileDef3DHex GetTileDefByLocation(FIntVector Location, int32& Index) const;
    
    FWFCTileDef3DHex GetTileDefByIndex(int32 Index) const;

    // --- UWFCTileAsset Overrides ---
    virtual int32 GetNumRotations() const override;
    virtual void GetAllowedRotations(TArray<int32>& OutRotations) const override;
    virtual int32 GetNumTileDefs() const override { return TileDefs.Num(); }
    
    // 关键：获取旋转后的 Edge Tag
    virtual FGameplayTag GetTileDefEdgeType(int32 TileDefIndex, FWFCGridDirection Direction) const override;
    
    // 关键：获取大型 Tile 内部旋转后的邻居索引
    virtual int32 GetTileDefInDirection(int32 TileDefIndex, FWFCGridDirection Direction) const override;
    
    virtual TArray<FActorInfoEntry> GetTileDefActorsInfo(int32 TileDefIndex) const override;
    virtual TArray<FStaticMeshInfoEntry> GetTileDefMeshesInfo(int32 TileDefIndex) const override;
    virtual TSubclassOf<UWFCUnit> GetTileDefUnit(int32 TileDefIndex) const override;
    virtual bool IsInteriorEdge(int32 TileDefIndex, FWFCGridDirection Direction) const override;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif
};