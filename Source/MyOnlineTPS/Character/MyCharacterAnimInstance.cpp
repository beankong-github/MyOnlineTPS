// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMyCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 캐릭터 가져오기
	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
}

void UMyCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 캐릭터 확인
	if (nullptr == MyCharacter)
	{
		// 캐릭터 없으면 다시 가져오기
		MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
	}
	if (nullptr == MyCharacter) return;

	// 스피드 얻어오기
	FVector Velocity = MyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// 공중에 있는지 확인하기
	bIsInAir = MyCharacter->GetCharacterMovement()->IsFalling();

	// 가속중인지 확인하기
	bIsAccelerating = MyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

	// 무기 장착중인지 확인
	bWeaponEquipped = MyCharacter->IsWeaponEquipped();

	// 쭈구리고 있는지 확인
	bIsCrouched = MyCharacter->bIsCrouched;

}
