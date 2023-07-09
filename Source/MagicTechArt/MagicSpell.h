// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Components/SplineComponent.h"
#include "SpellDataAsset.h"
#include "Components/TimelineComponent.h"
#include "SymbolRec.h"
#include "MagicSpell.generated.h"

UCLASS()
class MAGICTECHART_API AMagicSpell : public AActor
{	
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProceduralMesh;

	UPROPERTY(EditAnywhere)
	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* TransitionNiagaraComponent;
	
	UPROPERTY(EditAnywhere)
	UNiagaraComponent* SpellNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	TArray<USpellDataAsset*> SpellDataAssets;
	UPROPERTY(EditDefaultsOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	bool ActivateTraining = false;
	UPROPERTY(EditDefaultsOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	int TrainingIndex = 0;
	
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	float PlaneRadius = 500.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	int MeshCylinderResolution = 6;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	float MeshCylinderRadius = 15;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* MeshCylinderRadiusCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	float SegmentSize = 15;

	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* CommonMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* CommonVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Symbole", meta = (AllowPrivateAccess = "true"))
	FVector2D Tiling;

	UPROPERTY(EditDefaultsOnly, Category = "Magic|Transition", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* TransitionVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Transition", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* FailVFX;
	UPROPERTY(EditDefaultsOnly, Category = "Magic|Transition", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* FailRadiusCurve;
	
	TArray<FVector> DrawingPoints;
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	float CurrentUVy;
	TArray<int32> Triangles;
	FPlane DrawingPlane;

	FTimeline TransitionTimeline;

	UMaterialInterface* CurrentMaterial;
	UCurveFloat* CurrentRadiusCurve;
	float CurrentRadius;
	int SpellIndex;

public:
	AMagicSpell();
	
	SymbolRec symboleRec;

	UPROPERTY(EditAnywhere)
	USoundBase* FailedSpellSound;

	UPROPERTY(EditAnywhere)
	USoundBase* LambdaDrawSound;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ClearDrawing();
	void AddPointToDrawing(FVector LocalPoint);

	void LaunchSpell(int index);

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void GenerateMesh(float Scale);
	
	void StartDrawing();
	bool IsRayIntersectingPlane(FVector RayStart, FVector RayEnd);
	FVector GetPlaneIntersection(FVector RayStart, FVector RayEnd);
	void UpdateDrawing(FVector Point);
	void StopDrawing();

	UFUNCTION()
	void TransitionDrawing(float Value);
	
	UFUNCTION(BlueprintCallable)
	void SwitchToSpell();

	void SpellSuccess(int Index);
	void SpellFailure();

	UFUNCTION(BlueprintCallable)
	void DestroySpell();
};
