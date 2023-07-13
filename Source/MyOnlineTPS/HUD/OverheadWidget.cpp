// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay, FColor TextColor)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
		FSlateColor Color = FSlateColor(TextColor);
		DisplayText->SetColorAndOpacity(Color);
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	/* Show local role as string */
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ROLE_None:
		Role = FString("NONE");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy");
		break;
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	}
	FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(LocalRoleString);


	/* Show remote role as string */
	//ENetRole RemoteRole = InPawn->GetRemoteRole();
	//FString Role;
	//switch (RemoteRole)
	//{
	//case ROLE_None:
	//	Role = FString("NONE");
	//	break;
	//case ROLE_SimulatedProxy:
	//	Role = FString("Simulated Proxy");
	//	break;
	//case ROLE_AutonomousProxy:
	//	Role = FString("Autonomous Proxy");
	//	break;
	//case ROLE_Authority:
	//	Role = FString("Authority");
	//	break;
	//}
	//FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	//SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	if (nullptr == InPawn)
		return;

	/* Get Player Name */
	const APlayerState* LocalPlayerState = InPawn->GetPlayerState();
	FString Name = "Unknown";
	if (LocalPlayerState)
	{
		Name = LocalPlayerState->GetPlayerName();
	}

	/* Show role as color */
	ENetRole LocalRole = InPawn->GetLocalRole();
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FColor RoleColor;
	switch (LocalRole)
	{
	case ROLE_None:
		RoleColor = FColor::Black;
		break;
	case ROLE_SimulatedProxy:
		RoleColor = FColor::White;
		break;
	case ROLE_AutonomousProxy:
		RoleColor = FColor::Yellow;
		break;
	case ROLE_Authority:
		if (RemoteRole == ROLE_AutonomousProxy)
		{
			RoleColor = FColor::Yellow;
		}
		else if (RemoteRole == ROLE_SimulatedProxy)
		{
			RoleColor = FColor::White;
		}
		break;
	}

	SetDisplayText(Name, RoleColor);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();

	// 마지막에 호출해주기
	Super::NativeDestruct();
}
