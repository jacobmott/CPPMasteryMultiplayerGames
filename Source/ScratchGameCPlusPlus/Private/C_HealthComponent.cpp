// Fill out your copyright notice in the Description page of Project Settings.


#include "C_HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "C_GameModeBase.h"


// Sets default values for this component's properties
UC_HealthComponent::UC_HealthComponent()
{ 

  DefaultHealth = 100.0f;

  bIsDead = false;

  TeamNum = 255;


  SetIsReplicatedByDefault(true);


	// ...
}


// Called when the game starts
void UC_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	//Only the server should be doing any damage
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
	if (GetOwnerRole() == ROLE_Authority) {
		AActor* MyOwner = GetOwner();
		if (MyOwner) {
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UC_HealthComponent::HandleTakeAnyDamage);
		}

		Health = DefaultHealth;
	}

}

void UC_HealthComponent::OnRep_Health(float OldHealth)
{

	float Damage = Health - OldHealth;
	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void UC_HealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	if (Damage <= 0.0f || bIsDead) {
		return;
	}

	if (DamagedActor != DamagedActor && IsFriendly(DamagedActor, DamageCauser)) {
	  return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));
	
	bIsDead = Health <= 0.0f;

	//This allows us to bind/subscribe to this event in blueprints so we can do things when this event triggers
	//if we listen to the broadcast
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);


	if (bIsDead) {
		AC_GameModeBase* GM = Cast<AC_GameModeBase>(GetWorld()->GetAuthGameMode());

		if (GM) {

			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}

}


void UC_HealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f ) {
		return;
	}


	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

  //This allows us to bind/subscribe to this event in blueprints so we can do things when this event triggers
  //if we listen to the broadcast
  OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);
}

float UC_HealthComponent::GetHealth() const
{
	return Health;
}

bool UC_HealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{

	if (ActorA == nullptr || ActorB == nullptr) {
    //Assume friendly 
    return true;
	}

	UC_HealthComponent* HealthCompA = Cast<UC_HealthComponent>(ActorA->GetComponentByClass(UC_HealthComponent::StaticClass()));
	UC_HealthComponent* HealthCompB = Cast<UC_HealthComponent>(ActorB->GetComponentByClass(UC_HealthComponent::StaticClass()));


	if (HealthCompA == nullptr || HealthCompB == nullptr) {
		//Assume friendly 
		return true;
	}


	return HealthCompA->TeamNum == HealthCompB->TeamNum;

}

void UC_HealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UC_HealthComponent, Health);

}

