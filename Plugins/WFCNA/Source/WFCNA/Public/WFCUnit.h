// Copyright maxi3777. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISMManagerTypes.h"
#include "StackTreeNodeBase.h"

#include "WFCUnit.generated.h"

struct FWFCModelAssetTile;
enum class EWFCGeneratorState : uint8;
class UWFCGenerator;
class UWFCUnitManager;
class UWFCAsset;
class UISMManagerConfig;
class UIxObjectPoolConfig;
/**
 * 
 */
UCLASS()
class WFCNA_API UWFCUnit : public UStackTreeNodeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	TSubclassOf<UWFCUnitManager> ManagerClass;
	
	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	TObjectPtr<UIxObjectPoolConfig> ObjectPoolConfig;

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	TObjectPtr<UISMManagerConfig> ISMManagerConfig;

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	int32 ISMMaxInstancesCount = 50;

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	TArray<TObjectPtr<UWFCAsset>> Assets;

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	int32 ActiveAssetIndex = 0;

	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	bool bUseStartupSnapshot = true;

	/** 如果是这是一个小Hex网格，要嵌套在一个大Hex网格中，设置为true */
	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	bool bIsUseForHexNesting = false;

	/** WFCUnit的在Grid中的旋转中心 */
	UPROPERTY(EditAnywhere, Category ="WFCUnit Settings")
	FIntVector UnitGridCenter;

	virtual void Initialize() override;
	
	virtual void Run() override;
	
	virtual void Construct() override;
	
	virtual void Destruct() override;

	virtual void OnWFCStateChanged(EWFCGeneratorState State);

	void SetGenRootTransform(FTransform InTransform);

private:
	UPROPERTY(Transient)
	UWFCGenerator* Generator;

	FTransform GenRootTransform;

	TMap<FISMBucketKey, TArray<FTransform>> ISMTransforms;
	TMap<TSubclassOf<AActor>, TArray<FTransform>> ObjectPoolTransforms;

	TMap<TSubclassOf<AActor>, TArray<AActor*>> ObjectPoolMap;
	TMap<FISMBucketKey, TArray<int32>> ISMMangerMap;
	
	/** Return the tile that was selected for a collapsed cell. */
	const FWFCModelAssetTile* GetAssetTileForCell(int32 CellIndex) const;

	/**
	* Return the transform to use for spawning a tile in a cell,
	* incorporating the rotation of a tile as well.
	*/
	FTransform GetCellTransform(int32 CellIndex, int32 Rotation = 0) const;
};
