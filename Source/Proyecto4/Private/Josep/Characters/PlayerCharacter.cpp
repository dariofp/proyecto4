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
}

void APlayerCharacter::Tick(float DeltaTime)
{
	if (Attributes && PlayerOverlay)
	{
		Attributes->RegenMana(DeltaTime);
		PlayerOverlay->SetManaBarPercent(Attributes->GetManaPercent());
	}

	if (CombatTarget) 
	{
		FVector TargetLocation = CombatTarget->GetActorLocation();
		TargetLocation.Z -= 60.f;
		FRotator AimingTargetRotation = (TargetLocation - CameraBoom->GetComponentLocation()).Rotation();
		GetController()->SetControlRotation(AimingTargetRotation);
	}
	/*
	if (bIsTargeting)
	{
		if (CombatTarget != nullptr) 
		{
			GetRotationWarpTarget();
		}
		else 
		{
			bIsTargeting = false;
		}
		
	}*/
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
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Dodge);
		EnhancedInputComponent->BindAction(LockAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Lock);
		//EnhancedInputComponent->BindAction(DodgeComboAction, ETriggerEvent::Triggered, this, &APlayerCharacter::DodgeCombo);
	}
}
void APlayerCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
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
}
void APlayerCharacter::LockOn()
{
	const FVector MyLocation = GetActorLocation();
	FVector ClosestEnemyDistance(10000.f);
	AActor* ClosestEnemy{};
	for (auto Enemy : EnemiesInRange)
	{
		FVector DistanceToEnemy = MyLocation - Enemy->GetActorLocation();
		if (DistanceToEnemy.Length() < ClosestEnemyDistance.Length())
		{
			ClosestEnemy = Enemy;
			ClosestEnemyDistance = DistanceToEnemy;
		}
	}
	CombatTarget = ClosestEnemy;

}

void APlayerCharacter::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(FName("Enemy")))
	{
		EnemiesInRange.AddUnique(OtherActor);
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Enemigo Detectado"));
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
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();

	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);

	Direccion = MovementVector;
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
		EquipWeapon(OverlappingWeapon);
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

void APlayerCharacter::Attack()
{
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Melee;
	}
}

void APlayerCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughMana()) return;

	FVector CharacterVelocity = GetCharacterMovement()->Velocity;
	if (CharacterVelocity == FVector::ZeroVector) 
	{
		PlayDodgeMontage("DodgeBack");
	}
	if (CombatTarget) 
	{
		FVector DodgeDirectionVector = FVector(Direccion.Y, Direccion.X, 0).GetSafeNormal();

		if (!DodgeDirectionVector.IsNearlyZero() && CharacterVelocity != FVector::ZeroVector)
		{
			//PlayAttackMontage();
			DodgeDirection = CalculateDodgeDirection(DodgeDirectionVector);
			//bIsDodging = true;
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

void APlayerCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedMagicFire;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
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