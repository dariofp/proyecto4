// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChainedLightningProjectile.generated.h"

UCLASS()
class PROYECTO4_API AChainedLightningProjectile : public AActor
{
	GENERATED_BODY()
	
public:
    AChainedLightningProjectile();

    virtual void Tick(float DeltaTime) override;

    void InitializeProjectile(const TArray<AActor*>& InitialTargets);

protected:
    UPROPERTY(EditAnywhere, Category = "Chained Lightning")
    int32 TotalJumps;

    UPROPERTY(EditAnywhere, Category = "Chained Lightning")
    float MaxJumpDistance;

private:
    TArray<AActor*> ActorsToIgnore;

    void FindClosestTarget(const TArray<AActor*>& OverlappingActors);
    void JumpToNextTarget();
    void HandleTargetHit(AActor* HitActor);
};
