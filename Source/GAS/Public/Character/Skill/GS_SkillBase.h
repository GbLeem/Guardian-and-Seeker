// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_SkillBase.generated.h"

class AGS_Character;
/**
 * 
 */
UCLASS()
class GAS_API UGS_SkillBase : public UObject
{
	GENERATED_BODY()

public:
	// ��Ÿ�� ����
	float GetCoolTime();

	// ��ų �ʱ�ȭ
	void InitSkill(AGS_Character* InOwner);

	// ��ų �۵�
	virtual void ActiveSkill(); // ���� ���ѿ����� ȣ��
	virtual void ExecuteSkillEffect();
	virtual bool CanActive() const;

protected:
	// ��Ÿ�� ����
	FTimerHandle CooldownHandle;
	float Cooltime;
	bool bIsCoolingDown;
	void StartCoolDown();

	// ��ų ���� �ִϸ��̼�
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* SkillAnimMontage;

	// ��ų ������
	AGS_Character* OwnerCharacter;
};
