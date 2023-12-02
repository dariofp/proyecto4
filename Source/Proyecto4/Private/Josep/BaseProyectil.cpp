// Fill out your copyright notice in the Description page of Project Settings.


#include "Josep/BaseProyectil.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Josep/Interfaces/HitInterface.h"

ABaseProyectil::ABaseProyectil()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(10.0f); 
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionSphere->SetupAttachment(RootComponent); 
    CollisionSphere->ComponentTags.Add(TEXT("Proyectil"));

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseProyectil::OnProjectileBeginOverlap);
}

void ABaseProyectil::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseProyectil::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (ActorIsSameType(OtherActor)) return;

        if (OtherActor == Owner) 
        {
            return;
        }

        FString OtherActorName = OtherActor->GetName();
        FString DebugString = FString::Printf(TEXT("Collision with: %s"), *OtherActorName);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
        }

        AController* OwnerController = Owner ? Owner->GetInstigatorController() : nullptr;
        if (OwnerController && !ActorIsSameType(OtherActor))
        {
            UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, OwnerController, this, UDamageType::StaticClass());
            FHitResult NonConstSweepResult = SweepResult;
            ExecuteGetHit(NonConstSweepResult);
        }

        Destroy();
    }
}
/*
void ABaseProyectil::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this && OtherComp)
    {
        if (ActorIsSameType(OtherActor)) return;

        if (OtherActor == Owner) // Skip detection if colliding with the owner
        {
            return;
        }

        FString OtherActorName = OtherActor->GetName();
        FString DebugString = FString::Printf(TEXT("Collision with: %s"), *OtherActorName);

        // Display the debug string on the screen
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
        }

        FHitResult SphereHit;
        SphereTrace(SphereHit, Hit.ImpactPoint); // Llama a SphereTrace en el punto de colisión

        if (SphereHit.GetActor())
        {
            if (ActorIsSameType(SphereHit.GetActor())) return;

            UGameplayStatics::ApplyDamage(SphereHit.GetActor(), ProjectileDamage, GetInstigatorController(), this, UDamageType::StaticClass());
            ExecuteGetHit(SphereHit);
            // CreateFields(SphereHit.ImpactPoint);
        }
        Destroy();
    }

}


void ABaseProyectil::SphereTrace(FHitResult& OutHit, const FVector& StartPoint)
{
    const FVector EndPoint = StartPoint + GetActorForwardVector() * ProjectileSpeed;

    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepSingleByChannel(
        OutHit,
        StartPoint,
        EndPoint,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel1,
        FCollisionShape::MakeSphere(10.0f),
        CollisionParams
    );

    if (bHit)
    {
        FString HitActorName = OutHit.GetActor()->GetName();
        UE_LOG(LogTemp, Warning, TEXT("Colisión con: %s"), *HitActorName);

        if (!ActorIsSameType(OutHit.GetActor()))
        {
            UGameplayStatics::ApplyDamage(OutHit.GetActor(), ProjectileDamage, GetInstigatorController(), this, UDamageType::StaticClass());
            ExecuteGetHit(OutHit);
            // CreateFields(OutHit.ImpactPoint);
        }
    }

    if (GetWorld()->DebugDrawTraceTag.IsValid() && bHit)
    {
        DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.0f, 12, FColor::Green, false, 2.0f);
    }
}
*/

bool ABaseProyectil::ActorIsSameType(AActor* OtherActor)
{
    return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void ABaseProyectil::ExecuteGetHit(FHitResult& SphereHit)
{
    IHitInterface* HitInterface = Cast<IHitInterface>(SphereHit.GetActor());
    if (HitInterface)
    {
        HitInterface->Execute_GetHit(SphereHit.GetActor(), SphereHit.ImpactPoint, GetOwner());
    }
}

void ABaseProyectil::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

