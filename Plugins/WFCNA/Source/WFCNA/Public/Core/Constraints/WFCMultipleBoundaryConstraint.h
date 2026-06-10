// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/WFCConstraint.h"
#include "WFCTileAsset.h"
#include "WFCMultipleBoundaryConstraint.generated.h"

/**
 * 对应 MultipleBoundaryConstraint 的状态快照
 */
UCLASS()
class WFCNA_API UWFCMultipleBoundaryConstraintSnapshot : public UWFCConstraintSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bDidApplyInitialConstraint;
};

/**
 * 多重边界约束：
 * 与 BoundaryConstraint 类似，但允许为水平方向、顶部和底部分别设置不同的边界边缘限制规则。
 */
UCLASS(DisplayName = "Multiple Boundary Constraint")
class WFCNA_API UWFCMultipleBoundaryConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	/** 水平方向四周边界的 Tag 限制 (对应 Direction 0 ~ NumDirections - 3) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boundary Constraints")
	FGameplayTagQuery HorizontalEdgeTypeQuery;

	/** 顶部边界的 Tag 限制 (对应 3D 倒数第二个 Direction) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boundary Constraints")
	FGameplayTagQuery TopEdgeTypeQuery;

	/** 底部边界的 Tag 限制 (对应 3D 最后一个 Direction) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boundary Constraints")
	FGameplayTagQuery BottomEdgeTypeQuery;

	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual UWFCConstraintSnapshot* CreateSnapshot(UObject* Outer) const override;
	virtual void ApplySnapshot(const UWFCConstraintSnapshot* Snapshot) override;
	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

	void AddProhibitedAdjacentBoundaryMapping(FWFCTileId TileId, FWFCGridDirection Direction);
	
	/** 核心比对逻辑，按方向提取对应的 Query 进行限制 */
	bool CanTileBeNextToBoundary(const FWFCModelAssetTile& Tile, FWFCGridDirection Direction) const;

protected:
	bool bIsInitialized;
	bool bDidApplyInitialConstraint;

	/** 预计算好的排斥列表：TileID -> 不允许碰壁的方向列表 */
	TMap<FWFCTileId, TArray<FWFCGridDirection>> TileBoundaryProhibitionMap;
};