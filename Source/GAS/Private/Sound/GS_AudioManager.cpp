// Fill out your copyright notice in the Description page of Project Settings.


#include "Sound/GS_AudioManager.h"
#include "Sound/GS_CharacterAudioSystem.h"
#include "Sound/GS_UIAudioSystem.h"
#include "Sound/GS_EnvironmentAudioSystem.h"

void UGS_AudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ����ý��� �ν��Ͻ� ����
	CharacterAudio = NewObject<UGS_CharacterAudioSystem>(this);
	UIAudio = NewObject<UGS_UIAudioSystem>(this);
	EnvironmentAudio = NewObject<UGS_EnvironmentAudioSystem>(this);
}

void UGS_AudioManager::Deinitialize()
{
	// ���ҽ� ���� ����
	CharacterAudio = nullptr;
	UIAudio = nullptr;
	EnvironmentAudio = nullptr;

	Super::Deinitialize();
}

// Wwise �̺�Ʈ�� ȣ���ϴ� �κ�
void UGS_AudioManager::PlayEvent(UAkAudioEvent* Event, AActor* Context)
{
	if (!Event || !Context) // Wwise �̺�Ʈ�� ���ؽ�Ʈ�� ��ȿ���� ���� ���
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayEvent ȣ�� ����: Invalid Event or Context."));
		return;
	}
	FOnAkPostEventCallback DummyCallback;
	// Wwise�� �̺�Ʈ ����Ʈ
	UAkGameplayStatics::PostEvent(Event, Context, /*CallbackMask=*/0, /*PostEventCallback=*/DummyCallback);
}
