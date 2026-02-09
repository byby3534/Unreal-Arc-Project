// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimInstance/Player/CMPlayerLinkedAnimLayer.h"

#include "AnimInstance/Player/CMPlayerAnimInstance.h"

UCMPlayerAnimInstance* UCMPlayerLinkedAnimLayer::GetPlayerAnimInstance() const
{
	return Cast<UCMPlayerAnimInstance>(GetOwningComponent()->GetAnimInstance());
}