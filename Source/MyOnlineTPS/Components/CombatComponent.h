// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80'000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYONLINETPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	
	friend class AMyCharacter; // 이게 맞나..?

private:
	class AMyCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon*	EquippedWeapon;
	
	UPROPERTY(Replicated)
	bool bAiming;

	bool bFire;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

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

	/** Functions **/
	void TraceUnderCrosshair(FHitResult& TraceHitResult);

private:
	/** OnRep functions **/
	UFUNCTION()
	void OnRep_EquippedWeapon();
		
};
