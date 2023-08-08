// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyOnlineTPS/HUD/MyOnlineTPSHUD.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80'000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYONLINETPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	friend class AMyCharacter;

private:
	class AMyCharacter* Character;
	class AMyCharacterController* Controller;
	class AMyOnlineTPSHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon*	EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bAiming;

	bool bFire;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	FVector HitTarget;

	/** HUD and Crosshair **/
	FHUDPackage HUDPackage;

	float DefualtCrosshiarCenterGap;
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	/** Aiming and FOV **/
	float CurrentFOV;

	// 조준하지 않을 때 FOV BeginPlay시에 카메라의 base FOV로 설정
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	/** Overridings **/
	virtual void BeginPlay() override;

	/** Getters and Setters **/
	void SetAiming(bool bIsAiming);
	void SetFire(bool bPressed);

	/** RPC **/
	UFUNCTION(Server, Reliable)
	void ServerSetAimg(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/** Crosshair **/
	void TraceUnderCrosshair(FHitResult& TraceHitResult);
	void SetHUDCrosshair(float DeltaTime);

	/** Aiming and FOV **/
	void InterpFOV(float DeltaTime);

private:
	/** OnRep functions **/
	UFUNCTION()
	void OnRep_EquippedWeapon();
		
};
