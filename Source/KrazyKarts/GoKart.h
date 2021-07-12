// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForward(float i_Val);
	void MoveRight(float i_Val);

	UFUNCTION(Server, WithValidation, Reliable)
	void Server_MoveForward(float i_Val);

	UFUNCTION(Server, WithValidation, Reliable)
	void Server_MoveRight(float i_Val);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(EditAnywhere)
	float Mass = 1000;	//in Kg

	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000; //in Newton(N)

	//UPROPERTY(EditAnywhere)
	//float MaxDegreesPerSecond = 90; //the no of deg rotated per sond at full control throw(deg/sec)

	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10; //Minimum Radius of car turning at full lock(m)

	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16; //assuming MaxDrivingForce of 10000 and max speed of 25 m/s

	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015; //Higher the more Rolling Resistance

private:
	void ApplyRotation(float DeltaTime);
	void UpdateLocationFromVelocity(float DeltaTime);
	FVector GetAirResistance();
	FVector GetRollingResistance();
	UFUNCTION()
	void OnRep_ReplicatedTransform();

	UPROPERTY(Replicated)
	FVector Velocity;

	UPROPERTY(Replicated)
	float Throttle;
	UPROPERTY(Replicated)
	float SteeringThrow;

	/*UPROPERTY(replicated)
	FVector ReplicatedLocation;

	UPROPERTY(replicated)
	FRotator ReplicatedRotation;*/

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;
};
