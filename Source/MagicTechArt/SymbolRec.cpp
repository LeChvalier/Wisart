// Fill out your copyright notice in the Description page of Project Settings.

#include "SymbolRec.h"

// Set how many points we want
TArray<FVector2D> SymbolRec::Resample(TArray<FVector2D> pts, int n)
{
	TArray<FVector2D> points = pts;
	float increment = PathLength(points) / float(n - 1);
	float D = 0.f;
	TArray<FVector2D> resampledPoints;
	resampledPoints.Add(pts[0]);

	int i = 1;
	while (i < points.Num())
	{
		float d = (points[i] - points[i - 1]).Size();
		if (D + d >= increment)
		{
			float qx = points[i - 1].X + ((increment - D) / d) * (points[i].X - points[i - 1].X);
			float qy = points[i - 1].Y + ((increment - D) / d) * (points[i].Y - points[i - 1].Y);
			FVector2D q = {qx, qy};
			resampledPoints.Add(q);
			points.Insert(q, i);
			D = 0.f;
		}
		else
			D += d;
		i++;
	}

	if (resampledPoints.Num() == n - 1)
		resampledPoints.Add(pts.Last());
	
	return resampledPoints;
}

// Place the points in a 2size x 2size box (useful to draw template)
TArray<FVector2D> SymbolRec::Scale(TArray<FVector2D> points, float size)
{
	// Get min and max of the current AABB
	float minW = points[0].X;
	float maxW = points[0].X;
	float minH = points[0].Y;
	float maxH = points[0].Y;

	for (int i = 1; i < points.Num(); i++)
	{
		if (points[i].X < minW)
			minW = points[i].X;
		if (points[i].X > maxW)
			maxW = points[i].X;
		if (points[i].Y < minH)
			minH = points[i].Y;
		if (points[i].Y > maxH)
			maxH = points[i].Y;
	}

	// Get the box
	float width = maxW - minW;
	float height = maxH - minH;

	// Place the points inside the box
	TArray<FVector2D> toReturn;
	for (int i = 0; i < points.Num(); i++)
	{
		float qx = points[i].X * (size / width);
		float qy = points[i].Y * (size / height);
		FVector2D p = {qx, qy};
		toReturn.Add(p);
	}
	return toReturn;
}

// Translate the points around pt
TArray<FVector2D> SymbolRec::Translate(TArray<FVector2D> points, FVector2D pt)
{
	FVector2D centroidPoint = Centroid(points);
	TArray<FVector2D> toReturn;
	for (int i = 0; i < points.Num(); i++)
	{
		toReturn.Add(points[i] + pt - centroidPoint);
	}
	return toReturn;
}

// Distance between points between draw and templates
double SymbolRec::DistanceAtBestAngle(TArray<FVector2D> points, TArray<FVector2D> strokeTemplate, float threshold)
{
	double Phi = 1.61803398875f;

	float x1 = 1.0 - Phi;
	float f1 = DistanceAtAngle(points, strokeTemplate, x1);

	float x2 = 1.0 - Phi;
	float f2 = DistanceAtAngle(points, strokeTemplate, x2);

	int step = 0;
	int maxstep = 1000;
	while (step > maxstep)
	{
		if (f1 < f2)
		{
			//toAngle = x2;
			x2 = x1;
			f2 = f1;
			x1 = Phi;
			f1 = DistanceAtAngle(points, strokeTemplate, x1);
		}
		else
		{
			//fromAngle = x1;
			x1 = x2;
			f1 = f2;
			x2 = 0.f;
			f2 = DistanceAtAngle(points, strokeTemplate, x2);
		}
		maxstep++;
	}

	if (f1 < f2)
		return f1;
	return f2;
}

TArray<FVector2D> SymbolRec::Train(TArray<FVector> DrawingPoints)
{
	TArray<FVector2D> points;
	if (DrawingPoints.Num() == 0)
		return points;

	points = To2DPoints(DrawingPoints);

	points = Resample(points, 64);
	points = Scale(points, 64);
	points = Translate(points, {0.f, 0.f});

	if(points.Num() != 64)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
			FString::Printf(TEXT("Failed resample : resampledPoints.Num() : %i"), points.Num()));
		TArray<FVector2D> empty;
		return empty;
	}
	
	// Save them
	return points;
}

int SymbolRec::Recognition(TArray<FVector> DrawingPoints, TArray<FTemplate> templates, float threshold = 0.75f)
{
	if (DrawingPoints.Num() == 0)
		return -1;

	TArray<FVector2D> points = To2DPoints(DrawingPoints);

	points = Resample(points, 64);
	points = Scale(points, 64);
	points = Translate(points, {0.f, 0.f});

	if(points.Num() != 64)
		return -1;
	
	// check the best
	float best = threshold;
	int index = -1;
	for (int i = 0; i < templates.Num(); i++)
	{
		if(templates[i].templatePoints.Num() != 0)
		{
			float current = Recognize(points, templates[i].templatePoints);
			//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
			// 	FString::Printf(TEXT("template %d : %f"), i, current));
			
			if (current > best)
			{
				best = current;
				index = templates[i].index;
			}
		}
	}

	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
	//	FString::Printf(TEXT("value : %f"), best));
	return index;
}

TArray<FVector2D> SymbolRec::To2DPoints(TArray<FVector> points)
{
	TArray<FVector2D> toReturn;

	for (int i = 0; i < points.Num(); i++)
		toReturn.Add({points[i].Y, points[i].Z});

	return toReturn;
}

float SymbolRec::PathLength(TArray<FVector2D> points)
{
	FVector2D previous = points[0];

	float pathLength = 0;
	for (int i = 1; i < points.Num(); i++)
	{
		pathLength += (previous - points[i]).Size();
		previous = points[i];
	}
	return pathLength;
}

FVector2D SymbolRec::Centroid(TArray<FVector2D> points)
{
	FVector2D centroid = {0.f, 0.f};
	for (int i = 0; i < points.Num(); i++)
	{
		centroid += points[i];
	}
	centroid /= points.Num();

	return centroid;
}

float SymbolRec::DistanceAtAngle(TArray<FVector2D> points, TArray<FVector2D> strokeTemplate, float angle)
{
	float distance = 0;

	TArray<FVector2D> rotatedPoints;
	for (int i = 0; i < points.Num(); i++)
	{
		rotatedPoints.Add(points[i].GetRotated(angle));
		distance += (strokeTemplate[i] - rotatedPoints[i]).Size();
	}
	return distance;
}

float SymbolRec::Recognize(TArray<FVector2D> points, TArray<FVector2D> myTemplate)
{
	float b = DistanceAtBestAngle(points, myTemplate, 0.01f);
	
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red,
	//	FString::Printf(TEXT("computed is : %f"), b ));

	b = FMath::Clamp(b, 0.f, 3500.f);
	return (1.f - b / 3500.f);
}