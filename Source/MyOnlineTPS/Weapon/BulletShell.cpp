#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABulletShell::ABulletShell()
	: ShellEjectionImpulse(8.f)
	, bOnGround(false)
{
	PrimaryActorTick.bCanEverTick = false;

	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>("BulletShellMesh");
	SetRootComponent(BulletShellMesh);
	BulletShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletShellMesh->SetSimulatePhysics(true);
	BulletShellMesh->SetEnableGravity(true);
	BulletShellMesh->SetNotifyRigidBodyCollision(true);
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	
	BulletShellMesh->OnComponentHit.AddDynamic(this, &ABulletShell::OnHit);
	BulletShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound && !bOnGround)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	bOnGround = true;
	SetLifeSpan(0.2f);
}


