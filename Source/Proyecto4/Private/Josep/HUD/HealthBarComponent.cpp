// Fill out your copyright notice in the Description page of Project Settings.


#include "Josep/HUD/HealthBarComponent.h"
#include "Josep/HUD/HealthBar.h"
#include "Components/ProgressBar.h"
void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (HealthBarWidget == nullptr)
	{
		HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
	}
	
	if (HealthBarWidget && HealthBarWidget->HealthBar)
	{
		HealthBarWidget->HealthBar->SetPercent(Percent);
	}
}
