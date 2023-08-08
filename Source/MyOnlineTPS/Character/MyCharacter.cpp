#include "MyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

#include "MyOnlineTPS/Weapon/Weapon.h"
#include "MyOnlineTPS/Components/CombatComponent.h"
#include "MyCharacterAnimInstance.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());		// 메쉬에 카메라 붐 부착
	CameraBoom->TargetArmLength = 600.f;		// 카메라 붐 길이
	CameraBoom->bUsePawnControlRotation = true; // 마우스에 따라 카메라 붐 회전

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Set Overhead Widget
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Set Combat
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	Combat->SetIsReplicated(true); // 컴포넌트는 따로 등록할 필요없이 이렇게 함수만 호출해도 리플리케이트 등록이 된다.

	// Set Crouch
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;  //수구릴 수 있음
	GetCharacterMovement()->SetCrouchedHalfHeight(60.f);      //수구렸을때 충돌체 높이

	// 카메라와 플레이어 사이에 다른 플레이어가 있을 때 카메라 위치 이동 끄기
	// 캡슐 콜라이더와 카메라 충돌 반응 무시
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	// 메쉬와 카메라 충돌 반응 무시
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 회전 상태 초기화
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// [ Register variables to be replicated ]
	// - 모든 클라에게 복제 : DOREPLIFETIME(class, variable);
	// - Owner 클라에만 복제 : DOREPLIFETIME_CONDITION(class, variable, COND_OwnerOnly)

	DOREPLIFETIME_CONDITION(AMyCharacter, OverlappingWeapon, COND_OwnerOnly);
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);

		// Equipping
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &AMyCharacter::Equip);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMyCharacter::TryCrouch);

		// Animing
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &AMyCharacter::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AMyCharacter::AimReleased);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered,this, &AMyCharacter::Fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMyCharacter::FireReleased);
	}
}

// Called after the components are initialized
void AMyCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMyCharacter::Jump()
{
	if (bIsCrouched)
		UnCrouch();
	else
		Super::Jump();
}

// Input callbacks
void AMyCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMyCharacter::Equip()
{	
	if (Combat)
	{
		// 서버에서 실행되었을때 -> 바로 무기 장착
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}

		// 클라에서 실행되었을때 -> 서버에 무기 장착 요청(RPC)
		else
		{
			ServerEquip();
		}
	}
}

void AMyCharacter::TryCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMyCharacter::Aim()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AMyCharacter::AimReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AMyCharacter::Fire()
{
	if (Combat)
	{
		Combat->SetFire(true);
	}
}

void AMyCharacter::FireReleased()
{
	if (Combat)
	{
		Combat->SetFire(false);
	}
}
void AMyCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		if (!AnimInstance->Montage_IsPlaying(FireWeaponMontage))
		{
			AnimInstance->Montage_Play(FireWeaponMontage);
			FName SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}
// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void AMyCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	// 속도 체크
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f; // z축은 고려하지 않는다.
	float Speed = Velocity.Size();

	// 공중 체크
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	// AO_Yaw
	// 제자리에 서있을 때 z축 Aim Offset을 사용한다.
	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;

		}
		bUseControllerRotationYaw = true;	
		TurnInPlace(DeltaTime);
	}

	// 뛰거나 점프할 때 현재 프레임의 Yaw 기록
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	// AO_Pitch
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f  && !IsLocallyControlled())
	{
		// [270, 360) -> [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMyCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}


// 서버 실행
void AMyCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// Host 클라이언트 무기 위젯 끄기
	if (IsLocallyControlled())
	{
		if(OverlappingWeapon)
			OverlappingWeapon->ShowPickupWidget(false);
	}

	// OverlappingWeapon 설정
	OverlappingWeapon = Weapon;

	// Host 클라이언트 무기 위젯 표시
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
			OverlappingWeapon->ShowPickupWidget(true);
	}
}

AWeapon* AMyCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AMyCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

// RPC
void AMyCharacter::ServerEquip_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

// Owner 클라 실행
void AMyCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// 무기 위젯 끄기
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}

	// 무기 위젯 표시
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
}

bool AMyCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMyCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}
