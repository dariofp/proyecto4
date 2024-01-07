// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Josep/Interfaces/PickupInterface.h"
#include "PlayerCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;
class AItem;
class ASoul;
//class UGroomComponent;
class UAnimMontage;
class UPlayerOverlay;

UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	None,
	Left,
	Right,
	Forward,
	Backward
};

UENUM(BlueprintType)
enum class ETargetSwitchDirection
{
	Left,
	Right
};

UCLASS()
class PROYECTO4_API APlayerCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;

	void ToggleAimState();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	/* <IPickupInterface> */
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(int32 GoldAdded) override;
	/* </IPickupInterface> */

	UPROPERTY(EditAnywhere)
	float CameraRotationSpeed = 20.f;
	UPROPERTY(EditAnywhere)
	float CameraDistanceFromTarget = 30.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isAiming = false;
protected:
	virtual void BeginPlay() override;

	void SwitchTargetAxis(const FInputActionValue& AxisValue);

	void EnableTrigger();

	void ChangeTargetByDirection(ETargetSwitchDirection SwitchDirection);

	void LockOn(bool bSwitchLeft);

	void LockOn();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AttackFireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AttackLightningAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AttackGravityAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* ChangeTargetAction;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input);
	UInputAction* DodgeComboAction;
	*/
	/*
	Callbacks for inputs
	*/
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EKeyPressed();
	virtual void Attack() override;
	virtual void AttackFire();
	virtual void AttackGravity();
	virtual void AttackLightning();
	void Dodge();
	void Lock();
	//void DodgeCombo(const FInputActionValue& Value);

	/* Combat */
	void EquipWeapon(AWeapon* Weapon);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();
	void PlayEquipMontage(const FName SectionName);
	virtual void Die() override;
	bool HasEnoughMana();
	bool IsOccupied();
	void PlayDodgeAnimation(EDodgeDirection DodgeDirection);
	EDodgeDirection CalculateDodgeDirection(const FVector& Direction);

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDodgeDirection DodgeDirection;

	bool bIsTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<AActor*> EnemiesInRange;

private:
	bool IsUnoccupied();
	void InitializePlayerOverlay();
	void SetHUDHealth();
	FVector2D Direccion;
	/* Character components */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USphereComponent* DetectionSphere;

	/*UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;*/

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPlayerOverlay* PlayerOverlay;

	UPROPERTY(EditInstanceOnly, Category = "Target")
	TArray<AActor*> EnemyTargets;



	UPROPERTY(EditAnywhere)
	double TargetRadius = 200.f;

	bool bCanTrigger = true;
	const float TriggerCooldown = 0.5f; // Set your desired cooldown time here
	FTimerHandle TimerHandle_TriggerCooldown;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
