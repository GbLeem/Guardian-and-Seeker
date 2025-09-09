#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_DrakharAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Component/GS_CameraShakeComponent.h"
#include "Character/Component/GS_DebuffVFXComponent.h"
#include "Character/Component/GS_DrakharSFXComponent.h"
#include "Character/Component/GS_DrakharVFXComponent.h"
#include "Props/Interactables/GS_BridgePiece.h"
#include "Components/WidgetComponent.h"

AGS_Guardian::AGS_Guardian()
{
	PrimaryActorTick.bCanEverTick = false;
	
	CameraShakeComponent = CreateDefaultSubobject<UGS_CameraShakeComponent>(TEXT("CameraShakeComponent"));

	NormalMoveSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SpeedUpMoveSpeed = 1200.f;

	//boss monster tag for user widget
	Tags.Add("Guardian");
	
	//디버프 VFX 컴포넌트 생성
	DebuffVFXComponent = CreateDefaultSubobject<UGS_DebuffVFXComponent>("DebuffVFXComponent");

	// 컴포넌트 생성 및 초기화
	TargetedUIComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetedUI"));
	TargetedUIComponent->SetupAttachment(RootComponent);
	TargetedUIComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	TargetedUIComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TargetedUIComponent->SetDrawSize(FVector2D(100.f, 100.f));
	TargetedUIComponent->SetVisibility(false);
}

void AGS_Guardian::BeginPlay()
{
	Super::BeginPlay();
}

void AGS_Guardian::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	GuardianAnim = Cast<UGS_DrakharAnimInstance>(GetMesh()->GetAnimInstance());
}

void AGS_Guardian::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GuardianState);
	DOREPLIFETIME(ThisClass, GuardianDoSkillState);
	DOREPLIFETIME(ThisClass, MoveSpeed);
}

void AGS_Guardian::LeftMouse()
{
}

void AGS_Guardian::Ctrl()
{
}

void AGS_Guardian::CtrlStop()
{
}

void AGS_Guardian::RightMouse()
{
}

void AGS_Guardian::StartCtrl()
{
}

void AGS_Guardian::StopCtrl()
{
}

void AGS_Guardian::OnRep_MoveSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AGS_Guardian::MeleeAttackCheck()
{
	if (IsLocallyControlled())
	{
		GuardianState = EGuardianCtrlState::CtrlEnd;

		const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float MeleeAttackRange = 200.f;
		const float MeleeAttackRadius = 200.f;
		
		TSet<AGS_Character*> DamagedCharacters = DetectPlayerInRange(Start, MeleeAttackRange, MeleeAttackRadius);
		ApplyDamageToDetectedPlayer(DamagedCharacters, 0.f);
	}
}

TSet<AGS_Character*> AGS_Guardian::DetectPlayerInRange(const FVector& Start, float SkillRange, float Radius)
{
	TArray<FHitResult> OutHitResults;
	TSet<AGS_Character*> DamagedPlayers;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);

	FVector End = Start + GetActorForwardVector() * SkillRange;
	
	bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, End, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(Radius), Params);
	
	if (bIsHitDetected)
	{
		for (auto const& OutHitResult : OutHitResults)
		{
			if (OutHitResult.GetComponent() && OutHitResult.GetComponent()->GetCollisionProfileName() == FName("SoundTrigger"))
			{
				continue;
			}
			AGS_Character* DamagedCharacter = Cast<AGS_Character>(OutHitResult.GetActor());
			if (IsValid(DamagedCharacter))
			{
				DamagedPlayers.Add(DamagedCharacter);
			}
			//break bridge
			if (OutHitResult.GetActor()->IsA<AGS_BridgePiece>())
			{
				AGS_BridgePiece* BridgePiece = Cast<AGS_BridgePiece>(OutHitResult.GetActor());
				if (BridgePiece)
				{
					BridgePiece->BrokeBridge(100.f);
				}
			}
		}
	}
	return DamagedPlayers;
}

void AGS_Guardian::ApplyDamageToDetectedPlayer(const TSet<AGS_Character*>& DamagedCharacters, float PlusDamge)
{
	if (IsLocallyControlled())
	{
		for (auto const& DamagedCharacter : DamagedCharacters)
		{
			ServerRPCMeleeAttack(DamagedCharacter, PlusDamge);
			ServerRPCPlayHitEffect(DamagedCharacter);
			
			//server
			AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
			
			if (!Drakhar->GetIsFeverMode())
			{
				Drakhar->ServerRPCSetFeverGauge(10.f);
			}
			else if (Drakhar->GetIsFeverMode())
			{
				Drakhar->bIsAttckingDuringFever = true;
				Drakhar->ResetIsAttackingDuringFeverMode();
			}
		}
	}
}

void AGS_Guardian::QuitGuardianSkill()
{
	//reset skill state
	GuardianState = EGuardianCtrlState::CtrlEnd;
	GuardianDoSkillState = EGuardianDoSkill::None;
	
	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(this);
	if (Drakhar)
	{
		Drakhar->ServerRPCResetValue();
	}
	//fly end
	GetSkillComp()->Server_TryDeactiveSkill(ESkillSlot::Ready);
}

void AGS_Guardian::FinishCtrlSkill()
{
	StopCtrl();
}

void AGS_Guardian::ShowTargetUI(bool bIsActive)
{
	if (TargetedUIComponent)
	{
		TargetedUIComponent->SetVisibility(bIsActive);
	}
}

void AGS_Guardian::MulticastRPCApplyHitStop_Implementation(AGS_Character* InDamagedCharacter)
{
	if (HasAuthority())
	{
		if (CameraShakeComponent)
		{
			CameraShakeComponent->PlayCameraShake(HitStopShakeInfo);
		}
	}
	if (!HasAuthority())
	{
		if (!IsValid(InDamagedCharacter))
		{
			return;
		}
		CustomTimeDilation = 0.1f;
		InDamagedCharacter->CustomTimeDilation = 0.1f;

		FTimerDelegate HitStopTimerDelegate;
		FTimerHandle HitStopTimerHandle;
		HitStopTimerDelegate.BindUFunction(this, FName("MulticastRPCEndHitStop"), InDamagedCharacter);
		GetWorld()->GetTimerManager().SetTimer(HitStopTimerHandle, HitStopTimerDelegate, HitStopDurtaion, false);
	}
}

void AGS_Guardian::MulticastRPCEndHitStop_Implementation(AGS_Character* InDamagedCharacter)
{
	if (!HasAuthority())
	{
		CustomTimeDilation = 1.f;
		InDamagedCharacter->CustomTimeDilation = 1.f;
	}
}

void AGS_Guardian::ServerRPCPlayHitEffect_Implementation(AGS_Character* DamagedCharacter)
{
	//MulticastRPC_PlayAttackHitVFX(DamagedCharacter->GetActorLocation());
	MulticastRPCApplyHitStop(DamagedCharacter);
}
