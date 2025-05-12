// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GS_EnvironmentAudioSystem.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class GAS_API UGS_EnvironmentAudioSystem : public UObject
{
	GENERATED_BODY()

public:
	// ȯ�� ���� ����
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetEnvironmentZone(FName ZoneName);

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void SetWeatherState(FName WeatherName);

	// ���� ���
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayAmbientSound();

	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void PlayWeatherSound();

	// �����ũ �ε�
	UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
	void LoadEnvironmentSoundBank();
	
};
