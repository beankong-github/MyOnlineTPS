// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyOnlineTPS/Weapon/Weapon.h"

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

	// 스피드
	FVector Velocity = MyCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// 플레이어 상태 확인
	bIsInAir = MyCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = MyCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;

	bWeaponEquipped = MyCharacter->IsWeaponEquipped();
	EquippedWeapon = MyCharacter->GetEquippedWeapon();

	bIsCrouched = MyCharacter->bIsCrouched;
	bIsAiming = MyCharacter->IsAiming();
	TurningInPlace = MyCharacter->GetTurningInPlace();

	// 좌우앞뒤로 기울이기
	// yaw => z축 기준 회전축
	FRotator AimRotation = MyCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MyCharacter->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeletaRotation = FMath::RInterpTo(DeltaRot, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeletaRotation.Yaw;

	// 기대기
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MyCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	// Aim Offset
	AO_Yaw = MyCharacter->GetAO_Yaw();
	AO_Pitch = MyCharacter->GetAO_Pitch();

	
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MyCharacter->GetMesh())
	{
		// 무기에서 소켓으로 왼손 위치 구하기
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		// LeftHand 소켓 월드 위치의 캐릭터의 오른손 소켓에 대한 상대 위치를 구한다. (무기는 항상 오른손에 고정으로 달려있기 때문에) 
		MyCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		// 왼손 위치를 LeftHand 소켓 위치로 바꾼다.
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// 총구 위치가 에임 방향으로 향할 수 있도록 오른손 회전 계산
		if (MyCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = MyCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MyCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}
}