// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "SymbolRec.h"
#include "SpellDataAsset.generated.h"


USTRUCT()
struct FSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FTemplate> SymbolTemplate;
};

/**
 * 
 */
UCLASS(Blueprintable)
class MAGICTECHART_API USpellDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly)
	UMaterialInterface* SymbolMaterial;
	
	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* SymbolVFX;

	UPROPERTY(EditDefaultsOnly)
	UNiagaraSystem* TransitionVFX;

	UPROPERTY(EditDefaultsOnly)
	float SymbolRadius;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* MeshCylinderRadiusCurve;

	UPROPERTY(EditDefaultsOnly)
	float TransitionDuration;
	
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* TransitionRadiusCurve;
	
	TArray<FTemplate> symbolTemplate;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMagicSpellEffect> SpellEffect;

	UPROPERTY(EditAnywhere)
	USoundBase* DrawSound;
	
	UFUNCTION()
	bool Save();

	UFUNCTION()
	bool Load();
};
