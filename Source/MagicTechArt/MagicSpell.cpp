// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSpell.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MagicSpellEffect.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
AMagicSpell::AMagicSpell()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("SymbolMesh"));
	ProceduralMesh->bUseAsyncCooking = true;
	RootComponent = ProceduralMesh;

	// Set up Spline Component
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);


	// Set up Niagara Component
	TransitionNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TransitionNiagaraComponent"));
	TransitionNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	SpellNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SpellNiagaraComponent"));
	SpellNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AMagicSpell::BeginPlay()
{
	Super::BeginPlay();

	SpellNiagaraComponent->SetNiagaraVariableObject("SplineObject", SplineComponent);
	CurrentMaterial = CommonMaterial;
	CurrentRadiusCurve = MeshCylinderRadiusCurve;
	CurrentRadius = MeshCylinderRadius;
	SpellIndex = -1;

	for (USpellDataAsset* SpellData : SpellDataAssets) {
		SpellData->Load();
	}
}

// Called every frame
void AMagicSpell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TransitionTimeline.IsPlaying())
		TransitionTimeline.TickTimeline(DeltaTime);
}

void AMagicSpell::ClearDrawing()
{
	Vertices.Empty();
	Normals.Empty();
	UVs.Empty();
	CurrentUVy = 0;
	Triangles.Empty();
}

void AMagicSpell::AddPointToDrawing(FVector LocalPoint)
{
	LocalPoint.X = 0;

	SplineComponent->AddSplineLocalPoint(LocalPoint);
	const int Num = DrawingPoints.Num();
	if (Num > 1)
	{
		FVector Dir = (LocalPoint - DrawingPoints.Last()).GetSafeNormal();
		Dir = Dir.RotateAngleAxis(90, FVector::ForwardVector);
		SplineComponent->SetTangentAtSplinePoint(Num - 1, Dir, ESplineCoordinateSpace::Local);
	}

	DrawingPoints.Add(LocalPoint);
	
	//SplineComponent->SetTangentAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, FVector::ForwardVector,  ESplineCoordinateSpace::Local);
}


void AMagicSpell::GenerateMesh(float Scale)
{
	ClearDrawing();

	if (DrawingPoints.Num() < 2)
		return;

	const float UVxStep = 1.0f / MeshCylinderResolution;
	float UVy = 0;
	
	
	for (int i = 0; i < DrawingPoints.Num(); i++)
	{
		FVector Current = DrawingPoints[i];

		FVector Dir = FVector::RightVector;
		if (i > 0)
		{
			FVector Last = DrawingPoints[i - 1];
			Dir = (Current - Last).GetSafeNormal();
			UVy += (Current - Last).Size();
		}
		FVector Normal = FVector::ForwardVector;

		const float Radius = Scale * CurrentRadius * CurrentRadiusCurve->GetFloatValue(static_cast<float>(i) / static_cast<float>(DrawingPoints.Num() - 1));
		const float AngleRes = (360.0f / MeshCylinderResolution);
		for (int k = 0; k < MeshCylinderResolution + 1; k++)
		{
			FVector CircleDir = Normal.RotateAngleAxis(AngleRes * k, Dir).GetSafeNormal();

			Vertices.Add(Current + CircleDir * Radius);
			Normals.Add(CircleDir);
			UVs.Add(FVector2D((UVxStep * k * Tiling.X) * (2 * PI) * CurrentRadius, UVy * Tiling.Y));
		}
	}

	for (int i = 0; i < DrawingPoints.Num() - 1; i++)
	{
		const int First = i * (MeshCylinderResolution + 1);
		const int Second = (i + 1) * (MeshCylinderResolution + 1);

		for (int k = 0; k < MeshCylinderResolution + 1; k++)
		{
			Triangles.Add(First + k);
			Triangles.Add(Second + k);
			Triangles.Add(First + k + 1);

			Triangles.Add(First + k + 1);
			Triangles.Add(Second + k);
			Triangles.Add(Second + k + 1);
		}
	}

	ProceduralMesh->ClearAllMeshSections();
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(),
	                                  TArray<FProcMeshTangent>(), false);
	ProceduralMesh->SetMaterial(0, CurrentMaterial);
}

void AMagicSpell::StartDrawing()
{
	SplineComponent->ClearSplinePoints();
	DrawingPoints.Empty();
	DrawingPlane = FPlane(GetActorLocation(), - GetActorForwardVector());

	if(LambdaDrawSound)
		UGameplayStatics::PlaySound2D(GetWorld(), LambdaDrawSound);
}

bool AMagicSpell::IsRayIntersectingPlane(FVector RayStart, FVector RayEnd)
{
	return FMath::Abs(FVector::DotProduct(RayEnd, DrawingPlane.GetSafeNormal())) > 0.7f;
}

FVector AMagicSpell::GetPlaneIntersection(FVector RayStart, FVector RayEnd)
{
	return FMath::LinePlaneIntersection(RayStart, RayEnd, DrawingPlane);
}

void AMagicSpell::UpdateDrawing(FVector Point)
{
	if (DrawingPoints.Num() > 2)
	{
		SpellNiagaraComponent->SetAsset(CommonVFX);
	}
	
	FVector LocalPoint = UKismetMathLibrary::InverseTransformLocation(GetTransform(), Point);
	LocalPoint.X = 0;
	if (LocalPoint.Size() > PlaneRadius)
		return;

	if (DrawingPoints.Num() > 0)
	{
		FVector DirFromLast = LocalPoint - DrawingPoints.Last();
		if (DirFromLast.Size() < SegmentSize)
			return;

		const FVector DirNorm = DirFromLast.GetSafeNormal();

		while (DirFromLast.Size() > SegmentSize)
		{
			FVector StepPoint = DrawingPoints.Last() + DirNorm * SegmentSize;		
			AddPointToDrawing(StepPoint);
			DirFromLast = LocalPoint - DrawingPoints.Last();
		}
	}
	else
	{
		AddPointToDrawing(LocalPoint);
	}

	GenerateMesh(1);

	// Draw debug lines for Spline
	/*
	FVector Start, End, OutTangent;
	for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints() - 1; ++i)
	{
		Start = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		End = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);
		OutTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::World);

		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, -1.f, 0, 5.f);
		//DrawDebugLine(GetWorld(), Start, Start + OutTangent, FColor::Blue, false, -1.f, 0, 5.f);
	}*/
}

void AMagicSpell::StopDrawing()
{
	if (ActivateTraining)
	{
		SpellDataAssets[TrainingIndex]->symbolTemplate.Add({symboleRec.Train(DrawingPoints), TrainingIndex});
		SpellDataAssets[TrainingIndex]->Save();
		return;
	}

	if (DrawingPoints.Num() <= 1)
	{
		DestroySpell();
		return;
	}

	FVector Center = FVector::ZeroVector;
	for (int i = 0; i < DrawingPoints.Num(); i++)
	{
		Center += DrawingPoints[i];
	}
	Center /= DrawingPoints.Num();
	Center = UKismetMathLibrary::TransformLocation(GetActorTransform(), Center);
	FVector Offset = GetRootComponent()->GetComponentLocation() - Center;
	Offset = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), Offset);
	GetRootComponent()->SetWorldLocation(Center);
	
	TArray<FVector> PointsCopy(DrawingPoints);
	SplineComponent->ClearSplinePoints();
	DrawingPoints.Empty();
	for (int i = 0; i < PointsCopy.Num(); i++)
	{
		AddPointToDrawing(PointsCopy[i] + Offset);
	}
	GenerateMesh(1.0f);
	
	
	TArray<FTemplate> templates;
	for(int j = 0; j < SpellDataAssets.Num(); j++)
		for(int i = 0; i < SpellDataAssets[j]->symbolTemplate.Num(); i++)
			templates.Add({SpellDataAssets[j]->symbolTemplate[i].templatePoints, j});

	// return the best match between the drawingPoints and the templates list (85/100 precision)
	int index = symboleRec.Recognition(DrawingPoints, templates, 0.85f);

	if (index < 0)
		SpellFailure();
	else
		SpellSuccess(index);
}

void AMagicSpell::TransitionDrawing(float Value)
{
	GenerateMesh(Value);
}

void AMagicSpell::SwitchToSpell()
{
	SplineComponent->UpdateSpline();
	CurrentMaterial = SpellDataAssets[SpellIndex]->SymbolMaterial;
	CurrentRadiusCurve = SpellDataAssets[SpellIndex]->MeshCylinderRadiusCurve;
	CurrentRadius = SpellDataAssets[SpellIndex]->SymbolRadius;
	SpellNiagaraComponent->SetAsset(SpellDataAssets[SpellIndex]->SymbolVFX);
	SpellNiagaraComponent->SetNiagaraVariableObject("SplineObject", SplineComponent);

	LaunchSpell(SpellIndex);
}

void AMagicSpell::SpellSuccess(int Index)
{
	SpellIndex = Index;
	if (SpellDataAssets[Index]->TransitionVFX)
		TransitionNiagaraComponent->SetAsset(SpellDataAssets[SpellIndex]->TransitionVFX);
	else
		TransitionNiagaraComponent->SetAsset(TransitionVFX);
	
	TransitionNiagaraComponent->SetNiagaraVariableObject("SplineObject", SplineComponent);
	
	TransitionTimeline.SetTimelineLength(SpellDataAssets[SpellIndex]->TransitionDuration);
	FOnTimelineFloat TransitionFloat;
	TransitionFloat.BindUFunction(this, FName("TransitionDrawing"));
	TransitionTimeline.AddInterpFloat(SpellDataAssets[SpellIndex]->TransitionRadiusCurve, TransitionFloat);
	
	FOnTimelineEvent SwitchToSpellEvent;
	SwitchToSpellEvent.BindUFunction(this, FName("SwitchToSpell"));
	TransitionTimeline.AddEvent(1.0f, SwitchToSpellEvent);
	
	TransitionTimeline.PlayFromStart();

	if(SpellDataAssets[SpellIndex]->DrawSound)
		UGameplayStatics::PlaySound2D(GetWorld(), SpellDataAssets[SpellIndex]->DrawSound);
}

void AMagicSpell::LaunchSpell(int index)
{
	// Create actor
	FVector Forward = GetActorForwardVector();
	Forward.Z = 0;
	Forward.Normalize();
	FRotator Rotation = FRotationMatrix::MakeFromX(Forward).Rotator();
	AMagicSpellEffect* MagicSpellEffect = GetWorld()->SpawnActor<AMagicSpellEffect>(SpellDataAssets[index]->SpellEffect, GetActorLocation(), Rotation);
	MagicSpellEffect->MagicSpell = this;
}

void AMagicSpell::SpellFailure()
{
	TransitionNiagaraComponent->SetAsset(FailVFX);
	TransitionNiagaraComponent->SetNiagaraVariableObject("SplineObject", SplineComponent);
	
	TransitionTimeline.SetTimelineLength(1.1f);

	FOnTimelineFloat TransitionFloat;
	TransitionFloat.BindUFunction(this, FName("TransitionDrawing"));
	TransitionTimeline.AddInterpFloat(FailRadiusCurve, TransitionFloat);

	FOnTimelineEvent SwitchToDestroyEvent;
	SwitchToDestroyEvent.BindUFunction(this, FName("DestroySpell"));
	TransitionTimeline.AddEvent(1.0f, SwitchToDestroyEvent);

	TransitionTimeline.PlayFromStart();
	
	if(FailedSpellSound)
		UGameplayStatics::PlaySound2D(GetWorld(), FailedSpellSound);
}

void AMagicSpell::DestroySpell()
{
	TransitionNiagaraComponent->Deactivate();
	SpellNiagaraComponent->Deactivate();

	FTimerHandle TimerHandle;
	float DelaySeconds = 5.0f;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]() {
		Destroy();
	}, DelaySeconds, false);
}
