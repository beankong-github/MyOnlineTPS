// Fill out your copyright notice in the Description page of Project Settings.
#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

AProjectileWeapon::AProjectileWeapon()
{
}

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	if (!HasAuthority()) return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle flash socket to hit loaction from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator  TargetRatation = ToTarget.Rotation();

		if (ProjectilClass)
		{
			FActorSpawnParameters SpwnParams;
			SpwnParams.Owner = GetOwner();			// 투사체 주인 : 무기 소유주(플래이어)
			SpwnParams.Instigator = InstigatorPawn; // 데미지를 주는 주체 : 무기 소유주(플래이어)

			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectilClass,
					SocketTransform.GetLocation(),
					TargetRatation,
					SpwnParams
				);
			}
		}
	}
}
