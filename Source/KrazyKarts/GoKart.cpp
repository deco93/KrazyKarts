// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}


// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
		NetUpdateFrequency = 1;
}


void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ReplicatedTransform);
	/*DOREPLIFETIME(AGoKart, ReplicatedLocation);
	DOREPLIFETIME(AGoKart, ReplicatedRotation);*/
}

void AGoKart::OnRep_ReplicatedTransform()
{
	SetActorTransform(ReplicatedTransform);
}

FString GetEnumText(ENetRole i_Role)
{
	switch (i_Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	//a=dv/dt,  v=dx/dt, a=F/M;
	Super::Tick(DeltaTime);
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();
	GetRollingResistance();
	FVector Acceleration = Force / Mass;
	Velocity = Velocity + (Acceleration * DeltaTime);

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);
	if (HasAuthority())
	{
		ReplicatedTransform = GetActorTransform();
		/*ReplicatedLocation = GetActorLocation();
		ReplicatedRotation = GetActorRotation();*/
	}
	/*else
	{
		SetActorLocation(ReplicatedLocation);
		SetActorRotation(ReplicatedRotation);
	}*/
	DrawDebugString(GetWorld(),FVector(0,0,100),GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;//dx = v * dt
	//float RotationAngle = MaxDegreesPerSecond * DeltaTime * SteeringThrow;
	float RotationAngle = 0;
	if(SteeringThrow !=0)
		RotationAngle = DeltaLocation /(MinTurningRadius * SteeringThrow);// Because dx = d0 * R
	//FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);//no longer need to convert degrees to radians as the equation dx = d0 * R already in radian units
	Velocity = RotationDelta.RotateVector(Velocity);
	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

FVector AGoKart::GetAirResistance()
{
	return - Velocity.GetSafeNormal() * DragCoefficient * Velocity.SizeSquared();
}

FVector AGoKart::GetRollingResistance()
{
	//RollingResistance = RollingResistanceCoefficient * NormalForce
	//NormalForce is opposite of GravitationalForce = Mass * AccelerationDueToGravity
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	//UE_LOG(LogTemp, Warning, TEXT("Acceleration %f"), GetWorld()->GetGravityZ());
	float NormalForce =  Mass * AccelerationDueToGravity;
	
	return -Velocity.GetSafeNormal()* RollingResistanceCoefficient * NormalForce;
}


void AGoKart::MoveForward(float i_Val)
{
	Throttle = i_Val;
	Server_MoveForward(i_Val);
}

void AGoKart::MoveRight(float i_Val)
{
	SteeringThrow = i_Val;
	Server_MoveRight(i_Val);
}

void AGoKart::Server_MoveForward_Implementation(float i_Val)
{
	//Velocity = GetActorForwardVector() * 20 * Val;
	Throttle = i_Val;
}

bool AGoKart::Server_MoveForward_Validate(float i_Val)
{
	//Velocity = GetActorForwardVector() * 20 * Val;
	return FMath::Abs(i_Val) <= 1;
}

void AGoKart::Server_MoveRight_Implementation(float i_Val)
{
	SteeringThrow = i_Val;
}

bool AGoKart::Server_MoveRight_Validate(float i_Val)
{
	return FMath::Abs(i_Val) <= 1;
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}


