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
#include "Engine/World.h"
#include "EngineUtils.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"), DebugTrackerBotDrawing, TEXT("Draw debug lines for TrackerBot"), ECVF_Cheat);

// Sets default values
AC_AITrackerBot::AC_AITrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;


	MovementForce = 10.0f;
	bUseVelocityChange = false;
	RequiredDistanceToTarget = 100.0f;

	HealthComp = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &AC_AITrackerBot::HandleTakeDamage);


	ExplosionDamage = 60.0f; 
	ExplosionRadius = 350.0f;
	SelfDamageInterval = 0.5f;


	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);


  // In your some onTick or some such that is called each frame:
  float MyMaximumSpeedValue = 600.0f;
  FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
  FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
  MeshComp->SetPhysicsLinearVelocity(clampedVelocity);

	firstTimeGettingClose = true;

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

	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (TActorIterator<APawn> It(GetWorld()); It; ++It) {


		APawn* TestPawn = *It;

		if (TestPawn == nullptr || UC_HealthComponent::IsFriendly(TestPawn, this)) {
			continue;
		}

		UC_HealthComponent* TestPawnHealthComp = Cast<UC_HealthComponent>(TestPawn->GetComponentByClass(UC_HealthComponent::StaticClass()));

		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f) {
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();
			if (Distance < NearestTargetDistance) {
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}

	}


	if (BestTarget != nullptr){

		UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);
		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &AC_AITrackerBot::RefreshPath, 2.0f, false);

	  if (NavigationPath && NavigationPath->PathPoints.Num() > 0) {
	  	return NavigationPath->PathPoints[1];
	  }

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

	//UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

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

		if (DebugTrackerBotDrawing) {
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}
		//NEed to give the client some time to handle explosion effect
		//Destroy();
		SetLifeSpan(2.0f);
	}

}

void AC_AITrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

void AC_AITrackerBot::RefreshPath()
{

	NextPathPoint = GetNextPathPoint();
}

void AC_AITrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{

	Super::NotifyActorBeginOverlap(OtherActor);

	if (bStartedSelfDestruction || bExploded) {
		return;
	}

	AC_MyCharacter* PlayerPawn = Cast<AC_MyCharacter>(OtherActor);
	if (PlayerPawn && !UC_HealthComponent::IsFriendly(OtherActor, this)) {
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


		NextPathPoint = GetNextPathPoint();
		float DistanceFromBotToNextPathPoint = (GetActorLocation() - NextPathPoint).Size();


    if (bStartedSelfDestruction) {

      float MyMaximumSpeedValue = 1000.0f;
      FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
      FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
      MeshComp->SetPhysicsLinearVelocity(clampedVelocity);

    }

		if (DistanceFromBotToNextPathPoint <= RequiredDistanceToTarget) { 

			if (firstTimeGettingClose && !bStartedSelfDestruction) {
				// In your some onTick or some such that is called each frame:
				float MyMaximumSpeedValue = 100.0f;
				FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
				FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
				MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
				firstTimeGettingClose = false;
			}




      FVector ForceDirection = NextPathPoint - GetActorLocation();
      ForceDirection.Normalize();
      ForceDirection *= MovementForce;


      MeshComp->AddImpulse(ForceDirection, NAME_None, bUseVelocityChange);
			//MeshComp->SetPhysicsLinearVelocity(ForceDirection);
			if (DebugTrackerBotDrawing) {
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}

			if (!bStartedSelfDestruction) {
				// In your some onTick or some such that is called each frame:
				float MyMaximumSpeedValue = 600.0f;
				FVector currentVelocity = MeshComp->GetPhysicsLinearVelocity();
				FVector clampedVelocity = currentVelocity.GetClampedToMaxSize(MyMaximumSpeedValue);
				MeshComp->SetPhysicsLinearVelocity(clampedVelocity);
			}

		}
		else {

			//Keep moving towards next target
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;


			MeshComp->AddImpulse(ForceDirection, NAME_None, bUseVelocityChange);
			//MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			if (DebugTrackerBotDrawing) {
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32.0f, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}

		if (DebugTrackerBotDrawing) {
			DrawDebugSphere(GetWorld(), NextPathPoint, 20.0f, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}


}





