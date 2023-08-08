// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyOnlineTPSHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
	
public:
	class UTexture2D*	CrosshairCenter;
	UTexture2D*			CrosshairLeft;
	UTexture2D*			CrosshairRight;
	UTexture2D*			CrosshairTop;
	UTexture2D*			CrosshairBottom;
	
	FLinearColor		CrosshiarColor;
	float				CrosshairSpread;
};

/**
 * 
 */

UCLASS()
class MYONLINETPS_API AMyOnlineTPSHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AMyOnlineTPSHUD();

private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax;

public:
	virtual void DrawHUD() override;
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	
};
