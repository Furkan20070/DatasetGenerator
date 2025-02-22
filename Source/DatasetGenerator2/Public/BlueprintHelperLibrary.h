// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintHelperLibrary.generated.h"

/**
 * 
 */
UCLASS()
class DATASETGENERATOR2_API UBlueprintHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "File Operations")
    static void WriteTextFile(FString filename, FString content);

    UFUNCTION(BlueprintCallable, Category = "Helper|Bounding Box")
    static void Get2DBoundingBoxFromOrientedBox(
        const FVector& Origin,
        const FVector& Extent,
        const FVector& AxisX,
        const FVector& AxisY,
        const FVector& AxisZ,
        APlayerController* Controller,
        FVector2D& OutOrigin,
        FVector2D& OutSize
    );

    UFUNCTION(BlueprintCallable, Category = "Helper|Bounding Box")
    static void AdjustCubeToMatchMesh(AActor* TargetActor, FName CubeTag);
	
    UFUNCTION(BlueprintCallable, Category = "Helper|Generation")
    static bool PerformAdjustedRaycast(
        AActor* CameraOwner,
        float PitchOffset,
        float YawOffset,
        FHitResult& OutHitResult,
        float MaxRayDistance = 10000.0f // Default max distance
    );

    UFUNCTION(BlueprintCallable, Category = "Helper|Bounding Box")
    static bool GetStaticMeshScreenBoundsAccurate(const UStaticMeshComponent* MeshComponent, FVector2D& TopLeft, FVector2D& BottomRight);

    UFUNCTION(BlueprintCallable, Category = "Helper|Annotation Formats")
    static void ConvertToYOLOFormat(const FVector2D& upperLeft, const FVector2D& lowerRight, const float& resolutionX, const float& resolutionY, FVector2D& outBoundingBoxMid, FVector2D& boundingBoxExtends);
};
