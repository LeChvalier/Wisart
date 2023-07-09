// Copyright Epic Games, Inc. All Rights Reserved.

#include "MagicTechArtHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "MagicTechArtCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMagicTechArtHUD::AMagicTechArtHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/Blueprints/circle_hair"));
	CrosshairTex = CrosshairTexObj.Object;
}


void AMagicTechArtHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
										   (Center.Y));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	
	
	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		AMagicTechArtCharacter* Char = Cast<AMagicTechArtCharacter>(PC->GetCharacter());
		if (Char && !Char->IsLocked)
		{
			Canvas->DrawItem( TileItem );
			return;
		}
		
		float LocationX, LocationY;
		if (PC != nullptr)
		{
			PC->GetMousePosition(LocationX, LocationY);
			FVector2D MousePosition(LocationX, LocationY);
			TileItem.Position = MousePosition;
		}
	}
	
	Canvas->DrawItem( TileItem );
}
