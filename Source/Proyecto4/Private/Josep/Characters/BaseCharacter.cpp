// Fill out your copyright notice in the Description page of Project Settings.


#include "Josep/Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Josep/Items/Weapons/Weapon.h"
#include "Josep/Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Josep/BaseProyectil.h"
#include "Josep/Projectiles/LightningStrike.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();


}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		CombatTarget = nullptr;
	}
}

void ABaseCharacter::Die()
{
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the Enemy's Actor Location Z
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Take the inverse cosine (arc-cosine) of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			ImpactPoint
		);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(MeleeMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void ABaseCharacter::SpawnProjectileFromAnimation(int32 Index)
{
	if (ProjectileTypes.IsValidIndex(Index))
	{
		if (ProjectileTypes[Index])
		{
			FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
			FRotator SpawnRotation = GetActorRotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;


			ABaseProyectil* SpawnedProjectile = GetWorld()->SpawnActor<ABaseProyectil>(ProjectileTypes[Index], SpawnLocation, SpawnRotation, SpawnParams);
			if (SpawnedProjectile)
			{
				// Configuración adicional del proyectil spawnado si es necesario
			}
		}
	}
}
//FVector StartLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
//FRotator SpawnRotation = GetActorRotation();
//FVector ShootDirection = SpawnRotation.Vector();

void ABaseCharacter::SpawnLightningStrikeFromAnimation()
{
	if (GetWorld() && LightningStrikeClass)
	{

		FVector CameraLocation;
		FRotator CameraRotation;
		GetController()->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector StartLocation = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
		FVector ShootDirection = CameraRotation.Vector();

		float TraceDistance = 10000.0f;

		FVector EndLocation = StartLocation + GetActorForwardVector() * TraceDistance;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_GameTraceChannel1, CollisionParams);

		FVector EndPoint;

		if (bHit)
		{
			FVector CollisionPoint = HitResult.Location;
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
		}

		if (AimTarget) {
			if (AimTarget->ActorHasTag("Bateria")) {
				UGameplayStatics::ApplyDamage(AimTarget, 10, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			}
		}

		if (CombatTarget) {
			EndPoint = CombatTarget->GetActorLocation();
			UGameplayStatics::ApplyDamage(CombatTarget, 10, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			IHitInterface* HitInterface = Cast<IHitInterface>(CombatTarget);
			if (HitInterface)
			{
				HitInterface->Execute_GetHit(CombatTarget, CombatTarget->GetActorLocation(), GetOwner());
			}
			//UE_LOG(LogTemp, Warning, TEXT("damage to %s en location: %s"), *CombatTarget->GetName(), *CombatTarget->GetActorLocation().ToString());

		}

		if (CombatTarget == nullptr){
			EndPoint = GetActorLocation() + GetActorForwardVector() * 1000.0f;
			//UE_LOG(LogTemp, Warning, TEXT("casting rayo en location: %s"), EndPoint);

		}

		UNiagaraComponent* ProjectileBeam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LightningEffectSystem, StartLocation);

		if (ProjectileBeam)

		{
			ProjectileBeam->SetNiagaraVariableVec3(FString("PosicionFinal"), EndPoint);

		}
	}
}

void ABaseCharacter::SelectAttackMontageSection(UAnimMontage* Montage)
{

	if (PreviousMontage && PreviousMontage != Montage) {
		AttackCount = 0;
	}

	PreviousMontage = Montage;
	AttackCount++;
	FName SectionName = FName("Attack");
	if (AttackCount <= MaxCombo) 
	{
		SectionName.SetNumber(AttackCount + 1);
		PlayMontageSection(Montage, SectionName);
	}
	else 
	{
		ComboEnd();
		SectionName.SetNumber(AttackCount + 1);
		PlayMontageSection(Montage, SectionName);
	}
}

void ABaseCharacter::ComboAdd() 
{
	AttackCount++;
}

void ABaseCharacter::ComboEnd()
{
	AttackCount = 0;
}

void ABaseCharacter::PlayDodgeMontage(const FName& SectionName)
{
	PlayMontageSection(DodgeMontage, SectionName);
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.25f, MeleeMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if(CombatTarget==nullptr) return FVector();
	
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;

	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

bool ABaseCharacter::CanAttack()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}