// Fill out your copyright notice in the Description page of Project Settings.


#include "Josep/Characters/PlayerCharacter.h"

#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Josep/Components/AttributeComponent.h"
#include "Kismet/KismetMathLibrary.h"
//#include "GroomVisualizationData.h"
#include "Josep/Items/Item.h"
#include "Josep/Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Josep/HUD/PlayerHUD.h"
#include "Josep/HUD/PlayerOverlay.h"
#include "Josep/Items/Soul.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
	DetectionSphere->SetupAttachment(GetRootComponent()); 
	DetectionSphere->InitSphereRadius(300.0f);
	DetectionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	/*Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");*/
	//Weapon = CreateDefaultSubobject<AWeapon>(TEXT("WeaponCharacter"));
	//DetectionSphere->SetupAttachment(GetRootComponent());
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes && PlayerOverlay)
	{
		Attributes->RegenMana(DeltaTime);
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	}

	if (CombatTarget) 
	{
		FVector TargetLocation = CombatTarget->GetActorLocation();

		FVector CameraLocation = CameraBoom->GetComponentLocation();
		FRotator AimingTargetRotation = (TargetLocation - CameraLocation).Rotation();
		FRotator NewControlRotation = FRotator(-10, AimingTargetRotation.Yaw, 0);
		GetController()->SetControlRotation(NewControlRotation);
	}

	if (bIsCharging)
	{
		ChargeDuration += DeltaTime;
		if (ChargeDuration >= MaxChargeDuration)
		{
			ChargeDuration = MaxChargeDuration;
		}
	}

	if (CharacterState == ECharacterState::ECS_EquippedMagicFire && ChargeDuration >= 0.5)
	{
		UE_LOG(LogTemp, Warning, TEXT("Termino..."));
		CharacterState = ECharacterState::ECS_Charging;
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &APlayerCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartCharging);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dodge);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Completed, this, &APlayerCharacter::Lock);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::ToggleAimState);
		EnhancedInputComponent->BindAction(ChangeTargetAction, ETriggerEvent::Triggered, this, &APlayerCharacter::SwitchTargetAxis);
		EnhancedInputComponent->BindAction(AttackFireAction, ETriggerEvent::Started, this, &APlayerCharacter::StartCharging);
		EnhancedInputComponent->BindAction(AttackLightningAction, ETriggerEvent::Started, this, &APlayerCharacter::StartCharging);
		EnhancedInputComponent->BindAction(AttackGravityAction, ETriggerEvent::Started, this, &APlayerCharacter::StartCharging);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &APlayerCharacter::Attack);
		EnhancedInputComponent->BindAction(AttackFireAction, ETriggerEvent::Completed, this, &APlayerCharacter::AttackFire);
		EnhancedInputComponent->BindAction(AttackLightningAction, ETriggerEvent::Completed, this, &APlayerCharacter::AttackLightning);
		EnhancedInputComponent->BindAction(AttackGravityAction, ETriggerEvent::Completed, this, &APlayerCharacter::AttackGravity);
	}
}
void APlayerCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

void APlayerCharacter::ToggleAimState()
{
	// Toggle between aiming and idle state
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		ActionState = EActionState::EAS_Aiming;
		// Additional logic for entering aim state (e.g., adjusting camera, UI, etc.)
	}
	else if (CharacterState == ECharacterState::ECS_EquippedMagicFire)
	{
		ActionState = EActionState::EAS_Unoccupied;
		// Additional logic for exiting aim state (e.g., resetting camera, UI, etc.)
	}
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void APlayerCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void APlayerCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void APlayerCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && PlayerOverlay) 
	{
		Attributes->AddSouls(Soul->GetSouls());
		PlayerOverlay->SetSouls(Attributes->GetSouls());
	}
}

void APlayerCharacter::AddGold(int32 GoldAdded)
{
	if (Attributes && PlayerOverlay)
	{
		Attributes->AddGold(GoldAdded);
		PlayerOverlay->SetGold(Attributes->GetGold());
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("EngageableTarget"));
	InitializePlayerOverlay();
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnSphereOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnSphereEndOverlap);
	EquipWeapon();
}

void APlayerCharacter::SwitchTargetAxis(const FInputActionValue& AxisValue)
{
	if (!bCanTrigger) {
		return;
	}

	const FVector2D MovementVector = AxisValue.Get<FVector2D>();
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("X: %f, Y: %f"), MovementVector.X, MovementVector.Y));

	ETargetSwitchDirection SwitchDirection = (MovementVector.X > 0) ? ETargetSwitchDirection::Right : ETargetSwitchDirection::Left;
	ChangeTargetByDirection(SwitchDirection);

	bCanTrigger = false;
	GetWorldTimerManager().SetTimer(TimerHandle_TriggerCooldown, this, &APlayerCharacter::EnableTrigger, TriggerCooldown, false);

}

void APlayerCharacter::EnableTrigger()
{
	bCanTrigger = true;
}

void APlayerCharacter::ChangeTargetByDirection(ETargetSwitchDirection SwitchDirection)
{
	// Determine the direction of target switching
	bool bSwitchLeft = (SwitchDirection == ETargetSwitchDirection::Left);

	LockOn(bSwitchLeft);
}

void APlayerCharacter::LockOn(bool bSwitchLeft)
{
	if (EnemiesInRange.Num() > 0 && CombatTarget)
	{
		const FVector CurrentTargetLocation = CombatTarget->GetActorLocation();
		AActor* BestTarget = nullptr;
		float BestAngle = 180.0f;

		for (AActor* Enemy : EnemiesInRange)
		{
			if (Enemy != CombatTarget)
			{
				FVector DirectionToEnemy = Enemy->GetActorLocation() - CurrentTargetLocation;
				DirectionToEnemy.Normalize();

				FVector ForwardVector = GetActorForwardVector();
				ForwardVector.Normalize();

				float AngleToEnemy = FMath::RadiansToDegrees(FMath::Atan2(DirectionToEnemy.Y, DirectionToEnemy.X) - FMath::Atan2(ForwardVector.Y, ForwardVector.X));
				AngleToEnemy = FMath::FindDeltaAngleDegrees(0.0f, AngleToEnemy);
				
				if (bSwitchLeft)
				{
					float AngleDifference = FMath::FindDeltaAngleDegrees(AngleToEnemy, 0.0f);
					if (AngleDifference > 0 && FMath::Abs(AngleDifference) < BestAngle)
					{
						BestAngle = FMath::Abs(AngleDifference);
						BestTarget = Enemy;
					}
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("EntraDerecha"));
					float AngleDifference = FMath::FindDeltaAngleDegrees(AngleToEnemy, 0.0f);
					if (AngleDifference < 0 && FMath::Abs(AngleDifference) < BestAngle)
					{
						BestAngle = FMath::Abs(AngleDifference);
						BestTarget = Enemy;
					}
				}
			}
		}

		if (BestTarget)
		{
			CombatTarget = BestTarget;
		}
	}
}

void APlayerCharacter::LockOn()
{
	if (EnemiesInRange.Num() > 0)
	{
		if (!CombatTarget || !EnemiesInRange.Contains(CombatTarget))
		{
			float ClosestEnemyDistance = FLT_MAX;
			AActor* ClosestEnemy = nullptr;
			const FVector MyLocation = GetActorLocation();

			for (auto Enemy : EnemiesInRange)
			{
				float DistanceToEnemy = FVector::Distance(MyLocation, Enemy->GetActorLocation());
				if (DistanceToEnemy < ClosestEnemyDistance)
				{
					ClosestEnemy = Enemy;
					ClosestEnemyDistance = DistanceToEnemy;
				}
			}

			CombatTarget = ClosestEnemy;
		}
		else
		{
			CombatTarget = nullptr;
		}

		if (CombatTarget)
		{
			bUseControllerRotationYaw = true;
			CameraBoom->SetRelativeLocation(FVector(0, 35, 70));
		}
	}
}

void APlayerCharacter::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("Enemy")))
	{
		EnemiesInRange.AddUnique(OtherActor);
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Enemigo Detectado"));
	}
}

void APlayerCharacter::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (EnemiesInRange.Contains(OtherActor))
	{
		EnemiesInRange.Remove(OtherActor);
	}
	if (CombatTarget == OtherActor)
	{
		CombatTarget = nullptr;
		bUseControllerRotationYaw = false;
		CameraBoom->SetRelativeLocation(FVector(-20, 35, 70));
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator CameraRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, CameraRotation.Yaw, 0.f);

	if (bCanAttack)
	{
		ActionState = EActionState::EAS_Unoccupied;
		bCanAttack = false;
		StopAnimMontage();
	}

	if (ActionState == EActionState::EAS_Unoccupied)
	{
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);

		Direccion = MovementVector;
	}

	if (bcanRotate && !MovementVector.IsNearlyZero() || ActionState == EActionState::EAS_Unoccupied)
	{
		const FVector WorldMovementDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) * MovementVector.Y +
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) * MovementVector.X;

		FRotator DesiredRotation = FRotationMatrix::MakeFromX(WorldMovementDirection).Rotator();

		float DeltaTime = GetWorld()->GetDeltaSeconds();
		float RotationSpeed = 10.0f;
		FRotator SmoothRotation = FMath::RInterpTo(GetActorRotation(), DesiredRotation, DeltaTime, RotationSpeed);

		SetActorRotation(SmoothRotation);
	}
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	if (!CombatTarget) 
	{
		const FVector2D LookAxisVector = Value.Get<FVector2D>();

		AddControllerPitchInput(LookAxisVector.Y);
		AddControllerYawInput(LookAxisVector.X);
	}
}

void APlayerCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		EquipWeapon();
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void APlayerCharacter::StartCharging()
{
	bIsCharging = true;
	ChargeDuration = 0.0f;
	// Start charging animation or effects if needed
}

void APlayerCharacter::StopCharging()
{
	bIsCharging = false;
}

void APlayerCharacter::ExecuteChargedAttack()
{
	Super::Attack();
	if (ChargeDuration >= MaxChargeDuration)
	{

	}
	else
	{
		if (CanAttack())
		{
			SelectAttackMontageSection(FireMontage);
			Attributes->UseMana(Attributes->GetDodgeCost());
			ActionState = EActionState::EAS_Melee;
		}
	}

	StopCharging();
}

void APlayerCharacter::Attack()
{
	Super::Attack();
	if (!CombatTarget)
	{
		FindAndSetClosestEnemyInSight();
	}

	if (CanAttack() || bCanAttack)
	{
		bCanAttack = false;
		SelectAttackMontageSection(MeleeMontage);
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::FindAndSetClosestEnemyInSight()
{
	if (EnemiesInRange.Num() == 0) return; // Early exit if no enemies in range

	AActor* ClosestEnemy = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	FVector MyLocation = GetActorLocation();
	FVector ForwardVector = GetActorForwardVector();

	for (AActor* Enemy : EnemiesInRange)
	{
		FVector DirectionToEnemy = Enemy->GetActorLocation() - MyLocation;
		float DistanceSq = DirectionToEnemy.SizeSquared();
		DirectionToEnemy.Normalize();

		// Calculate the angle between the player's forward vector and the direction to the enemy
		float DotProduct = FVector::DotProduct(ForwardVector, DirectionToEnemy);
		float Angle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		// Only consider enemies within a 45-degree angle in front of the player
		if (Angle <= 120.0f)
		{
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestEnemy = Enemy;
			}
		}
	}

	CombatTarget = ClosestEnemy; // May be nullptr if no suitable target found
	if (CombatTarget)
	{
		OrientTowards(CombatTarget);
	}
}

void APlayerCharacter::OrientTowards(AActor* Target)
{
	if (Target)
	{
		FRotator TargetRotation = (Target->GetActorLocation() - GetActorLocation()).Rotation();
		SetActorRotation(TargetRotation);
	}
}

void APlayerCharacter::AttackFire()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(FireMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::AttackGravity()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(GravityMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::AttackLightning()
{
	if (!HasEnoughMana()) return;
	Super::Attack();
	if (CanAttack())
	{
		SelectAttackMontageSection(LightningMontage);
		Attributes->UseMana(Attributes->GetDodgeCost());
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughMana() || !GetCharacterMovement()->IsMovingOnGround()) return;

	FVector CharacterVelocity = GetCharacterMovement()->Velocity;
	if (CharacterVelocity == FVector::ZeroVector) 
	{
		PlayDodgeMontage("DodgeBack");
	}
	if (CombatTarget) 
	{
		FVector DodgeDirectionVector = FVector(Direccion.Y, Direccion.X, 0).GetSafeNormal();

		FRotator TargetRotation = (CombatTarget->GetActorLocation() - GetActorLocation()).Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 100);
		SetActorRotation(NewRotation);
		
		if (!DodgeDirectionVector.IsNearlyZero() && CharacterVelocity != FVector::ZeroVector)
		{
			DodgeDirection = CalculateDodgeDirection(DodgeDirectionVector);
			PlayDodgeAnimation(DodgeDirection);
		}
	}
	else 
	{
			PlayDodgeMontage("DodgeNormal");
	}

	ActionState = EActionState::EAS_Dodge;
	if (Attributes && PlayerOverlay)
	{
		Attributes->UseMana(Attributes->GetDodgeCost());
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	}
}

void APlayerCharacter::Lock()
{
	LockOn();
}

void APlayerCharacter::PlayDodgeAnimation(EDodgeDirection DodgeDirection2)
{
	switch (DodgeDirection2)
	{
		case EDodgeDirection::Forward:
			PlayDodgeMontage("DodgeForward");
			break;
		case EDodgeDirection::Backward:
			PlayDodgeMontage("DodgeBack");
			break;
		case EDodgeDirection::Left:
			PlayDodgeMontage("DodgeLeft");
			break;
		case EDodgeDirection::Right:
			PlayDodgeMontage("DodgeRight");
			break;
	}
}

EDodgeDirection APlayerCharacter::CalculateDodgeDirection(const FVector& Direction)
{
	const float Angle = FMath::Atan2(Direction.Y, Direction.X) * 180.f / PI;

	if (Angle < 45.f && Angle >= -45.f)
	{
		return EDodgeDirection::Forward;
	}
	else if (Angle >= 45.f && Angle < 135.f)
	{
		return EDodgeDirection::Right;
	}
	else if (Angle >= -135.f && Angle < -45.f)
	{
		return EDodgeDirection::Left;
	}
	else
	{
		return EDodgeDirection::Backward;
	}
}

void APlayerCharacter::EquipWeapon(/*AWeapon* WeaponEquipped*/)
{
	/*
	WeaponEquipped->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedMagicFire;
	OverlappingItem = nullptr;
	EquippedWeapon = WeaponEquipped;
	*/

	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedMagicFire;
		EquippedWeapon = DefaultWeapon;
	}
}

void APlayerCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void APlayerCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool APlayerCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

void APlayerCharacter::AnimAttackEnd()
{
	bCanAttack = true;
}

bool APlayerCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool APlayerCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		EquippedWeapon;
}

void APlayerCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void APlayerCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedMagicFire;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void APlayerCharacter::PlayEquipMontage(const FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void APlayerCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

bool APlayerCharacter::HasEnoughMana()
{
	return Attributes && Attributes->GetMana() > Attributes->GetDodgeCost();
}

bool APlayerCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void APlayerCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void APlayerCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void APlayerCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void APlayerCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool APlayerCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void APlayerCharacter::InitializePlayerOverlay()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PlayerController->GetHUD());
		if (PlayerHUD)
		{
			PlayerOverlay = PlayerHUD->GetPlayerOverlay();
			if (PlayerOverlay && Attributes)
			{
				PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				PlayerOverlay->SetManaBarPercent(1.f);
				PlayerOverlay->SetGold(0);
				PlayerOverlay->SetSouls(0);
			}
		}

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
}

void APlayerCharacter::SetHUDHealth()
{
	if (PlayerOverlay && Attributes)
	{
		PlayerOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}