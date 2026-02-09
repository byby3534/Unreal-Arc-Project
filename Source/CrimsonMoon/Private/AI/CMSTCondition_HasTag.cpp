// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/CMSTCondition_HasTag.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

bool UCMSTCondition_HasTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	// 1. 대상이나 태그가 유효하지 않으면 무조건 False
	if (!ContextActor || !TagToCheck.IsValid())
	{
		return false;
	}

	// 2. GAS 시스템(ASC) 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ContextActor);
	if (!ASC)
	{
		return false;
	}

	// 3. 태그 검사 (정확히 일치하는 태그가 있는지)
	const bool bHasTag = ASC->HasMatchingGameplayTag(TagToCheck);

	// 4. 결과 반환 (bInvert가 켜져 있으면 반대로 뒤집음)
	return bInvert ? !bHasTag : bHasTag;
}