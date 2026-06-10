// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/Constraints/WFCEdgeBlockConstraint.h"
#include "WFCTileAsset.h"
#include "Core/WFCGrid.h"
#include "Core/Grids/WFCGrid3D.h"
#include "Core/Grids/WFCGrid3DHex.h"

bool UWFCEdgeBlockConstraint::AreTilesCompatible(const FWFCModelAssetTile& TileA, const FWFCModelAssetTile& TileB, FWFCGridDirection Direction) const
{
	// 1. 基础匹配检查
	if (!Super::AreTilesCompatible(TileA, TileB, Direction))
	{
		return false;
	}

	if (!TileA.TileAsset.IsValid() || !TileB.TileAsset.IsValid() || ExcludedTagMap.IsEmpty())
	{
		return true;
	}

	// 2. 解析当前方向属于 水平(Surroundings) 还是 垂直(TopDown)
	bool bIsTopDown = false;
	
	// 只有 3D 类型的网格才有 TopDown 的概念
	if (Grid->IsA<UWFCGrid3D>() || Grid->IsA<UWFCGrid3DHex>())
	{
		const int32 NumDirections = Grid->GetNumDirections();
		// 倒数两个方向是 Z 轴上下
		if (Direction >= NumDirections - 2)
		{
			bIsTopDown = true;
		}
	}

	// 定义一个 Lambda 表达式，判断该规则在当前方向是否适用
	auto DoesRuleApply = [bIsTopDown](EAdjacentType RuleType) -> bool
	{
		if (RuleType == EAdjacentType::All) return true;
		if (RuleType == EAdjacentType::Surroundings && !bIsTopDown) return true;
		if (RuleType == EAdjacentType::TopDown && bIsTopDown) return true;
		return false;
	};

	const FGameplayTagContainer& TagsA = TileA.TileAsset->OwnedTags;
	const FGameplayTagContainer& TagsB = TileB.TileAsset->OwnedTags;

	// 3. 检查 A 是否根据方向排斥了 B
	for (const FGameplayTag& TagA : TagsA)
	{
		if (const FExclusionRuleList* RuleListForA = ExcludedTagMap.Find(TagA))
		{
			for (const FExclusionRule& Rule : RuleListForA->Rules) // 注意这里通过 .Rules 获取内部数组
			{
				if (DoesRuleApply(Rule.AdjacentType) && TagsB.HasAny(Rule.ExcludedTags))
				{
					return false; // 在适用方向上触发了排斥
				}
			}
		}
	}

	// 4. 检查 B 是否根据方向排斥了 A
	for (const FGameplayTag& TagB : TagsB)
	{
		if (const FExclusionRuleList* RuleListForB = ExcludedTagMap.Find(TagB))
		{
			for (const FExclusionRule& Rule : RuleListForB->Rules) // 注意这里通过 .Rules 获取内部数组
			{
				if (DoesRuleApply(Rule.AdjacentType) && TagsA.HasAny(Rule.ExcludedTags))
				{
					return false; // 在适用方向上触发了排斥
				}
			}
		}
	}

	return true;
}