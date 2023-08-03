#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "MyOnlineTPS/Weapon/Weapon.h"
#include "MyOnlineTPS/Character/MyCharacter.h"
#include "MyOnlineTPS/PlayerController/MyCharacterController.h"
#include "MyOnlineTPS/HUD/MyOnlineTPSHUD.h"

UCombatComponent::UCombatComponent()
	: Character(nullptr)
	, Controller(nullptr)
	, HUD(nullptr)
	, EquippedWeapon(nullptr)
	, bAiming(false)
	, bFire(false)
	, BaseWalkSpeed(600.f)
	, AimWalkSpeed(450.f)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [ Register variables to be replicated ]
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshair(DeltaTime);
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	if (Controller == nullptr)
		Controller = Cast<AMyCharacterController>(Character->Controller);

	if (Controller)
	{
		if (HUD == nullptr)
			HUD = Cast<AMyOnlineTPSHUD>(Controller->GetHUD());

		if (HUD)
		{
			FHUDPackage HUDPackage;
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
			}
			HUD->SetHUDPackage(HUDPackage);
		}
	}

}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	
	// 클라면 서버에서도 Aiming 설정
	ServerSetAimg(bIsAiming);

	// 걸음 속도 조정
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// RPC
void UCombatComponent::ServerSetAimg_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	// 걸음 속도 조정
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::SetFire(bool bPressed)
{
	bFire = bPressed;

	if (bFire)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

// RPC
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

// Muticast RRC (서버-> 모든 클라이언트)
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector2D ViewportSoze;

	// 화면 크기 구하기
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSoze);
	}

	// 화면 중앙에 Crosshair 두기
	FVector2D CrosshairLocation(ViewportSoze.X / 2.f, ViewportSoze.Y/2.f);
	
	// Crosshair 위치를 World 기준 위치로 변환
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	// Crosshair부터 직선으로 TRACE_LENGTH만큼 충돌 검사
	if (bScreenToWorld)
	{
		// 충돌 검사 시작 지점
		FVector Start = CrosshairWorldPosition;
		// 충돌 검사 종료 지점
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		// 충돌한 오브젝트(TraceHitResult)를 구한다.
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility			
		);

		// 충돌한 것이 없다면
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}



// 클라 실행
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

// 서버 실행
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr
		|| WeaponToEquip == nullptr)
		return;

	EquippedWeapon = WeaponToEquip;

	// 무기 상태 장착으로 변경 (replicate)
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	// 캐릭터 손 소켓에 무기 붙이기 (replicate)
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("hand_rSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// 무기의 Owner를 캐릭터로 설정 (replicate)
	EquippedWeapon->SetOwner(Character); 

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;

}

