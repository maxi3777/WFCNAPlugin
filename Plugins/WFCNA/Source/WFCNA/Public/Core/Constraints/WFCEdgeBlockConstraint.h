// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/Constraints/WFCEdgeConstraint.h"
#include "WFCEdgeBlockConstraint.generated.h"

/** 
 * 邻接判断的生效方向类型 
 */
UENUM(BlueprintType)
enum class EAdjacentType : uint8
{
	All				UMETA(DisplayName = "All Directions"),
	Surroundings	UMETA(DisplayName = "Horizontal Only"),
	TopDown			UMETA(DisplayName = "Vertical Only")
};

/**
 * 定义一条具体的排斥规则
 */
USTRUCT(BlueprintType)
struct FExclusionRule
{
	GENERATED_BODY()

	/** 要排斥的 Tag 集合 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ExcludedTags;

	/** 该排斥规则在哪些方向上生效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAdjacentType AdjacentType = EAdjacentType::All;
};

/**
 * 用于包装 TArray，以绕过 UHT 对 TMap 嵌套容器的限制
 */
USTRUCT(BlueprintType)
struct FExclusionRuleList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (TitleProperty = "AdjacentType"))
	TArray<FExclusionRule> Rules;
};


/**
 * 带有排斥标签功能和方向限制的邻接约束。
 */
UCLASS(Abstract, DisplayName = "Edge Block Constraint")
class WFCNA_API UWFCEdgeBlockConstraint : public UWFCEdgeConstraint
{
	GENERATED_BODY()

public:
	/** 
	 * 排斥规则字典。
	 * Key: 自身拥有的 Tag
	 * Value: 包装好的规则列表 (包含多条不同方向的规则)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC")
	TMap<FGameplayTag, FExclusionRuleList> ExcludedTagMap;

	virtual bool AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction) const override;
};