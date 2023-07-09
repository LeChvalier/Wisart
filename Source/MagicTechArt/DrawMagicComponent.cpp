
#include "DrawMagicComponent.h"

#include "MagicTechArtCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MagicSpell.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UDrawMagicComponent::UDrawMagicComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//ProceduralMesh->SetAbsolute(true, true, true);
}


// Called when the game starts
void UDrawMagicComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AMagicTechArtCharacter>(UGameplayStatics::GetPlayerCharacter(GetOwner(),0));
	Camera = Character->GetFirstPersonCameraComponent();
}



// Called every frame
void UDrawMagicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UDrawMagicComponent::StartDrawing()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	FVector WorldPos;
	FVector WorldDir;
	if (PC->DeprojectMousePositionToWorld(WorldPos, WorldDir))
	{
		FVector PointLocation = WorldPos + WorldDir * PlaneDistance;

		CurrentDrawMagicSpell = GetWorld()->SpawnActor<AMagicSpell>(SpellClass.Get(), PointLocation, FRotationMatrix::MakeFromX(WorldDir).Rotator());
		CurrentDrawMagicSpell->StartDrawing();
	}
}

void UDrawMagicComponent::UpdateDrawing()
{
	if (CurrentDrawMagicSpell == nullptr)
		return;
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	FVector WorldPos;
	FVector WorldDir;
	if (PC->DeprojectMousePositionToWorld(WorldPos, WorldDir))
	{
		FVector RayStart = PC->PlayerCameraManager->GetCameraLocation();
		FVector RayEnd = RayStart + WorldDir * PlaneDistance;

		if (!CurrentDrawMagicSpell->IsRayIntersectingPlane(RayStart, RayEnd))
			return;

		FVector PointLocation = CurrentDrawMagicSpell->GetPlaneIntersection(RayStart, RayEnd);
		Character->DrawingTarget = PointLocation;
		
		FVector LocalPoint = UKismetMathLibrary::InverseTransformLocation(Camera->GetComponentTransform(), PointLocation);
		if (!(LocalPoint.Size() < RayDistance))
			return;

		if (Character->IsDrawing)
			CurrentDrawMagicSpell->UpdateDrawing(PointLocation);
	}
}

void UDrawMagicComponent::StopDrawing()
{
	if (CurrentDrawMagicSpell)
	{
		CurrentDrawMagicSpell->StopDrawing();
		CurrentDrawMagicSpell = nullptr;
	}
}