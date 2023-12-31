// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYONLINETPS_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon();

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectilClass;

public:
	/** Functions **/
	virtual void Fire(const FVector& HitTarget) override;
};
