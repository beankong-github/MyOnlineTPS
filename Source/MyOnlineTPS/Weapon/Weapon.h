#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName  = "Initial State"),
	EWS_Equipped UMETA(DisplayName  = "Equipped"),
	EWS_Dropped UMETA(DisplayName  = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class MYONLINETPS_API AWeapon : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget; 
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 구체가 무엇인가와 Overlap될 때 호출된다.
	UFUNCTION()
		virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent
			, AActor* OtherActor
			, UPrimitiveComponent* OtherPrimitiveComponent
			, int32					OtherBodyIndex
			, bool					bFromSweep
			, const FHitResult& SweepResult);

private:

};
