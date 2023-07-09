// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SymbolRec.generated.h"

USTRUCT()
struct FTemplate
{
	GENERATED_BODY()

//private:
	UPROPERTY()
	TArray<FVector2D> templatePoints;
	UPROPERTY()
	int index;
};


/**
 * 
 */
class MAGICTECHART_API SymbolRec
{
public:
	SymbolRec(){};
	~SymbolRec(){};

	// Set the template
	TArray<FVector2D> Train(TArray<FVector> DrawingPoints);
	// return the index of the nearest template
	int Recognition(TArray<FVector> DrawingPoints, TArray<FTemplate> templates, float threshold);

	float Recognize(TArray<FVector2D> points, TArray<FVector2D> myTemplate);
	
	TArray<FVector2D> To2DPoints(TArray<FVector> points);
	
	// Useful functions
	TArray<FVector2D> Resample(TArray<FVector2D> points, int nbPoints);	// Need rework
	TArray<FVector2D> Scale(TArray<FVector2D> points, float size);
	TArray<FVector2D> Translate(TArray<FVector2D> points, FVector2D pt);

	// Swift math adaptations
	float PathLength(TArray<FVector2D> points);
	FVector2D Centroid(TArray<FVector2D> v);
	float DistanceAtAngle(TArray<FVector2D> points, TArray<FVector2D> strokeTemplate, float angle);
	double DistanceAtBestAngle(TArray<FVector2D> points, TArray<FVector2D> strokeTemplate, float threshold);
};