#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Projectile.generated.h"

UCLASS()
class MYONLINETPS_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	
private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;


};
