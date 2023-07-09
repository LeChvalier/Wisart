// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DrawMagicComponent.generated.h"

class AMagicTechArtCharacter;
class UCameraComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAGICTECHART_API UDrawMagicComponent : public USceneComponent
{
	GENERATED_BODY()

	UPROPERTY()
	AMagicTechArtCharacter* Character;
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMagicSpell> SpellClass;
	
	AMagicSpell* CurrentDrawMagicSpell;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	float PlaneDistance = 100.0f;
	UPROPERTY(EditDefaultsOnly, Category = Magic, meta = (AllowPrivateAccess = "true"))
	float RayDistance = 500.0f;
	
public:	
	// Sets default values for this component's properties
	UDrawMagicComponent();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartDrawing();
	void UpdateDrawing();
	void StopDrawing();
};
