// Fill out your copyright notice in the Description page of Project Settings.


#include "Josep/ChainedLightningProjectile.h"

AChainedLightningProjectile::AChainedLightningProjectile()
{
    TotalJumps = 5;
    MaxJumpDistance = 1000.0f;
}

void AChainedLightningProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AChainedLightningProjectile::InitializeProjectile(const TArray<AActor*>& InitialTargets)
{
    ActorsToIgnore = InitialTargets;
    JumpToNextTarget();
}

void AChainedLightningProjectile::FindClosestTarget(const TArray<AActor*>& OverlappingActors)
{
    float MinDistance = MAX_FLT;
    AActor* ClosestActor = nullptr;

    for (AActor* OverlappingActor : OverlappingActors)
    {
        if (!ActorsToIgnore.Contains(OverlappingActor))
        {
            float Distance = (OverlappingActor->GetActorLocation() - ActorsToIgnore.Last()->GetActorLocation()).Size();
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestActor = OverlappingActor;
            }
        }
    }

    if (ClosestActor)
    {
        ActorsToIgnore.Add(ClosestActor);
        HandleTargetHit(ClosestActor);
    }
    else
    {
        JumpToNextTarget();
    }
}

void AChainedLightningProjectile::JumpToNextTarget()
{
    if (ActorsToIgnore.Num() < TotalJumps)
    {
        TArray<AActor*> OverlappingActors;
        // Implement Sphere Overlap Actors and get overlapping actors
        // Use FindClosestTarget(OverlappingActors) to continue the chain
    }
    else
    {
        // Chain completed, perform final actions or cleanup
    }
}

void AChainedLightningProjectile::HandleTargetHit(AActor* HitActor)
{
    // Logic to handle what happens when the projectile hits a target
    // Spawn a new actor if needed, passing the relevant information
    // Implement checks and the chaining logic as described
    // For demonstration purposes, let's print the actor's name
    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *HitActor->GetName());

    // Continue the chain
    JumpToNextTarget();
}

