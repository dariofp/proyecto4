// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Josep/Interfaces/PickupInterface.h"
#include "Components/TimelineComponent.h"
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

USTRUCT(BlueprintType)
struct FAbilityUnlockInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SoulCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked;

	FAbilityUnlockInfo() : AbilityName(TEXT("")), SoulCost(0), bIsUnlocked(false) {}

	FAbilityUnlockInfo(const FString& InName, const int32 InCost, const bool InIsUnlocked = false)
		: AbilityName(InName), SoulCost(InCost), bIsUnlocked(InIsUnlocked) {}
};

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

UENUM(BlueprintType)
enum class ECurrentAttackType
{
	None UMETA(DisplayName = "None"),
	Melee UMETA(DisplayName = "Melee"),
	Fire UMETA(DisplayName = "Fire"),
	Lightning UMETA(DisplayName = "Lightning"),
	Gravity UMETA(DisplayName = "Gravity"),
	Vital UMETA(DisplayName = "Vital")
};

UENUM(BlueprintType)
enum class ERMBAction
{
	LightningAttack,
	GravityAttack,
	VitalAttack,
	// Add more actions as needed
};


UCLASS()
class PROYECTO4_API APlayerCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void ChangeAttackMaterials();
	UFUNCTION(BlueprintCallable, Category = "Materials")
	void RevertMaterials();
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCharging = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanAttack = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanDodge = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanDash = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bcanRotate = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldOrientTowardsTarget = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldMoveForward = false;
	float ChargeStartTime = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashDistance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InputBufferTime = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dash")
	UTimelineComponent* DashTimeline;

	UFUNCTION()
	void DashTick(float Value);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void SetAttackToNone();

	UFUNCTION(BlueprintNativeEvent, Category = "Attack")
	void FinAim();
	virtual void FinAim_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Attack")
	void CreateChaos();
	virtual void CreateChaos_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void MoveForward(float ScaleValue);

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial1;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial2;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* AttackMaterial3;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* InvisibleMaterial;

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
	UInputAction* ChangeAttackAction;

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
	void StartCharging();
	void StopCharging();
	virtual void Attack() override;
	void FindAndSetClosestEnemyInSight();
	void OrientTowards(AActor* Target);
	virtual void AttackFire();
	virtual void AttackGravity();
	virtual void AttackLightning();
	void Dodge();
	void Lock();
	//void DodgeCombo(const FInputActionValue& Value);

	/* Combat */
	void ChangeRMBAction();
	void Dash();
	void CancelDash(EActionState State);
	void OnAttackReleased();
	void PerformRegularAttack();
	void ReleasedChargedAttack();
	void ExecuteChargedAttack();
	void AttackMeleeCombo();
	void AttackMagicCombo();
	void AttackCombo(ECurrentAttackType Type);
	void EquipWeapon(/*AWeapon* WeaponEquipped*/);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	UFUNCTION(BlueprintCallable)
	void AnimAttackEnd();
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECurrentAttackType AttackType;

	bool bIsTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<AActor*> EnemiesInRange;

	UPROPERTY(EditAnywhere, Category = "Dash")
	UCurveFloat* DashCurve;

	FVector DashStartLocation;

	FVector DashTargetLocation;

	void InitializeDashTimeline();

	ERMBAction CurrentRMBAction = ERMBAction::LightningAttack;


private:
	bool IsUnoccupied();
	void InitializePlayerOverlay();
	void SetHUDHealth();
	FVector2D Direccion;
	FVector DireccionMov;

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
	const float ChargeAttackThreshold = 0.5f; // Seconds

	FTimerHandle TimerHandle_TriggerCooldown;
	FTimerHandle TimerHandle_ChargeAttack;

	
	//float ChargeDuration = 0.0f;
	const float MaxChargeDuration = 2.0f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	/*-------------------------<Input Buffering>---------------------------------------*/
	bool bDashInputBuffered = false;
	bool bAttackInputBuffered = false;
	float LastInputTimeDash = 0.0f;
	float LastInputTimeAttack = 0.0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities")
	TArray<FAbilityUnlockInfo> AbilitiesToUnlock;
	UFUNCTION(BlueprintCallable)
	void UnlockAbility(FString AbilityName);
	bool IsAbilityUnlocked(FString AbilityName) const;
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};
