// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Josep/Interfaces/HitInterface.h"
#include "Josep/Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;
class UNiagaraSystem;

UCLASS()
class PROYECTO4_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class ABaseProyectil> ProjectileClass;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SpawnProjectileFromAnimation(int32 Index);

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TArray<TSubclassOf<class ABaseProyectil>> ProjectileTypes; // TArray que almacena diferentes tipos de proyectiles

	TArray<AActor*> OwnedObjects;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<class ALightningStrike> LightningStrikeClass; // Declaración de la clase LightningStrike

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SpawnLightningStrikeFromAnimation();

	void BeamNiagara(const FVector& StartLocation, const FVector& EndPoint);

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* LightningEffectSystem; // Niagara system for the lightning effect

protected:
	virtual void BeginPlay() override;
	/* Combat */
	UFUNCTION(BlueprintCallable)
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void Attack();
	virtual void Die();
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	void DisableMeshCollision();

	/* Montage */
	void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	void SelectAttackMontageSection(UAnimMontage* Montage);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	virtual void PlayDodgeMontage(const FName& SectionName);
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	void ComboAdd();

	UFUNCTION(BlueprintCallable)
	void ComboEnd();

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* AimTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(EditAnywhere, Category = Combat)
	int32 AttackCount;

	UPROPERTY(EditAnywhere, Category = Combat)
	int32 MaxCombo;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* LightningMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* GravityMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* VitalMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	float TimeInCombo;

private:
	
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PreviousMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> AttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;

	UPROPERTY()
	FTimerHandle TimerCombo;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
