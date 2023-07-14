#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "MyCharacter.generated.h"

UCLASS()
class MYONLINETPS_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AMyCharacter();

private:
	/** Camera **/
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** MappingContext **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Input Action **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Overhead Widget **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	/** Weapon **/
	// 충돌처리같은 중요한 이벤트는 서버에서 처리하기 위해 Overlap된 무기를 리플리케이트 변수로 지정. 
	// 리플리케이트는 변수에 변경점이 생길때만 발생한다. 리플리케이트가 발생하면 서버의 변경점이 클라이언트에 복사되어 반영된다.
	// notify 사용 x : UPROPERTY(Replicated)
	// notify 사용 O : UPROPERTY(ReplicatedUsing = 함수명)
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon; 



public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

	void SetOverlappingWeapon(AWeapon* Weapon);

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
};
