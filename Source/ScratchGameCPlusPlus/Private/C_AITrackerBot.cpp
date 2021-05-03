// Fill out your copyright notice in the Description page of Project Settings.


#include "C_AITrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "C_MyCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AC_AITrackerBot::AC_AITrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;


	MovementForce = 1000.0f;
	bUseVelocityChange = false;
	RequiredDistanceToTarget = 100.0f;

	HealthComp = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AC_AITrackerBot::HandleTakeDamage);


	ExplosionDamage = 40.0f; 
	ExplosionRadius = 200.0f;
	SelfDamageInterval = 0.5f;


	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AC_AITrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
  //Only the server should be doing any damage
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
	if (GetLocalRole() == ROLE_Authority) {
		//Find inital move to
		NextPathPoint = GetNextPathPoint();
	}

}

FVector AC_AITrackerBot::GetNextPathPoint()
{
	
	//Hack to get player location
	//AActor* PlayerPawn = Cast<AActor>(UGameplayStatics::GetPlayerCharacter(this, 0));
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);


	if (NavigationPath && NavigationPath->PathPoints.Num() > 0){
		return NavigationPath->PathPoints[1];
  }

	//failed to find path
	return GetActorLocation();
}

void AC_AITrackerBot::HandleTakeDamage(UC_HealthComponent* HealthCompent, float Health, float HealthDelta, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst) {
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

	//Explode on hitpoints == 0
  if (Health <= 0) {
		SelfDestruct();
	}

}

void AC_AITrackerBot::SelfDestruct()
{

	if (bExploded) {
		return;
	}
	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, GetActorLocation());
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  //Only the server should be doing any damage
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
	if (GetLocalRole() == ROLE_Authority) {
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), 200.0f, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		//NEed to give the client some time to handle explosion effect
		//Destroy();
		SetLifeSpan(2.0f);
	}

}

void AC_AITrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void AC_AITrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{

	Super::NotifyActorBeginOverlap(OtherActor);

	if (bStartedSelfDestruction || bExploded) {
		return;
	}

	AC_MyCharacter* PlayerPawn = Cast<AC_MyCharacter>(OtherActor);
	if (PlayerPawn) {
		//We overlapped with a player!
		//Start self destruction sequence

  //Only the server should be doing any damage
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
		if (GetLocalRole() == ROLE_Authority) {
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &AC_AITrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		}
		bStartedSelfDestruction = true;
		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
	}
}

// Called every frame
void AC_AITrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


  //Only the server should be doing any damage
  //if (GetLocalRole() == ROLE_Authority) {
  //HasAuthority is the same thing as above, its just inlined to it
	if (GetLocalRole() == ROLE_Authority && !bExploded) {
		float DistanceFromBotToNextPathPoint = (GetActorLocation() - NextPathPoint).Size();


		if (DistanceFromBotToNextPathPoint <= RequiredDistanceToTarget) {
			NextPathPoint = GetNextPathPoint();
			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else {
			//Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32.0f, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}


}





