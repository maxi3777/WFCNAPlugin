// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/WFCConstraint.h"
#include "WFCPositionConstraint.generated.h"

/**
 * 定义一个轴（X, Y 或 Z）的位置限制规则。
 */
USTRUCT(BlueprintType)
struct FWFCPositionAxisRule
{
	GENERATED_BODY()

	FWFCPositionAxisRule()
		: Range(FIntPoint::ZeroValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery AllowedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint Range;
};

/** 状态快照 */
UCLASS()
class WFCNA_API UWFCPositionConstraintSnapshot : public UWFCConstraintSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bDidApplyInitialConstraint;
};

/**
 * UObject 载体类：在主线程负责读取蓝图配置、执行坐标计算，
 * 并生成供异步纯 C++ 类使用的预计算数据。
 */
UCLASS(Abstract, DisplayName = "Position Constraint")
class WFCNA_API UWFCPositionConstraint : public UWFCConstraint
{
	GENERATED_BODY()

public:
	virtual void Initialize(UWFCGenerator* InGenerator) override;
	virtual void Reset() override;
	virtual UWFCConstraintSnapshot* CreateSnapshot(UObject* Outer) const override;
	virtual void ApplySnapshot(const UWFCConstraintSnapshot* Snapshot) override;

	virtual TUniquePtr<FAsyncConstraint> CreateAsyncConstraint(FAsyncGenerator* InGenerator, TSharedPtr<FAsyncGrid> InGrid, TSharedPtr<FAsyncModel> InModel) override;

protected:
	bool bIsInitialized;
	bool bDidApplyInitialConstraint;

	/** 缓存的待剔除列表：[CellIndex] ->[TileIds] */
	TMap<FWFCCellIndex, TArray<FWFCTileId>> TilesToBan;

	virtual void CalculateTilesToBan() {}

	void GetTilesFailingRule(const FWFCPositionAxisRule& Rule, TArray<FWFCTileId>& OutFailingTiles) const;
};

/**
 * 适用于 2D 网格的位置限制约束 (X, Y)
 */
UCLASS(Abstract, DisplayName = "Position 2D Constraint")
class WFCNA_API UWFCPosition2DConstraint : public UWFCPositionConstraint
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Position Constraint")
	FWFCPositionAxisRule RuleX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Position Constraint")
	FWFCPositionAxisRule RuleY;

protected:
	virtual void CalculateTilesToBan() override;
};

/**
 * 适用于 3D 网格的位置限制约束 (X, Y, Z) (兼容 Hex 网格)
 */
UCLASS(Abstract, DisplayName = "Position 3D Constraint")
class WFCNA_API UWFCPosition3DConstraint : public UWFCPositionConstraint
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Position Constraint")
	FWFCPositionAxisRule RuleX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Position Constraint")
	FWFCPositionAxisRule RuleY;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Position Constraint")
	FWFCPositionAxisRule RuleZ;

protected:
	virtual void CalculateTilesToBan() override;
};