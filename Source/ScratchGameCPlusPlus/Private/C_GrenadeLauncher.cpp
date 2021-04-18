// Fill out your copyright notice in the Description page of Project Settings.


#include "C_GrenadeLauncher.h"

#include "Kismet/GameplayStatics.h"

void AC_GrenadeLauncher::Fire()
{

  if (GetLocalRole() < ROLE_Authority) {
    ServerFire();
  }

  //Trace the world from pawn eyes to crosshair location
  AActor* MYOwner = GetOwner();
  if (MYOwner) {

    FVector EyeLocation;
    FRotator EyeRotation;
    MYOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

    FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
    //FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);

    if (HasAuthority()) {

      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      if (ProjectileClass) {
        AActor* BulletActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
        //We have access to this actor in the blueprint so we can set this information there
        //BulletActor->SetReplicates(true);
        //BulletActor->SetReplicateMovement(true);
      }
    }

    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner) {
      APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
      if (PC) {
        UE_LOG(LogTemp, Warning, TEXT("CameraShake called!"));
        PC->ClientPlayCameraShake(CameraShake);
      }
    }
  }


}



void AC_GrenadeLauncher::StartFire()
{

  float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

  GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AC_GrenadeLauncher::Fire, TimeBetweenShots, true, FirstDelay);

}

void AC_GrenadeLauncher::StopFire()
{
  GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}



//void AC_Weapon::OnRep_Bullet()
//{
//  //Replicate bullet
//
//  PlayFireEffects(HitScanTrace.TraceTo);
//  PlayImpaceEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
//
//}