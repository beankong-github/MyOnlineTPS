#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "MyOnlineTPS/Weapon/Weapon.h"
#include "MyOnlineTPS/Character/MyCharacter.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [ Register variables to be replicated ]
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
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
}

