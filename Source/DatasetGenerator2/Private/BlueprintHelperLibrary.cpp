// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintHelperLibrary.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"
#include "RenderUtils.h"


void UBlueprintHelperLibrary::WriteTextFile(FString filename, FString content)
{
    // Define directory and file path
    FString Directory = FPaths::ProjectDir() + TEXT("Saved/");
    FString FilePath = Directory + filename;

    // Get platform file manager
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    // Ensure the directory exists
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectory(*Directory);
        if (!PlatformFile.DirectoryExists(*Directory))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create directory: %s"), *Directory);
            return;
        }
    }

    // Write content to file
    if (FFileHelper::SaveStringToFile(content, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("File saved successfully at: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save file at: %s"), *FilePath);
    }
}

void UBlueprintHelperLibrary::Get2DBoundingBoxFromOrientedBox(
    const FVector& Origin,
    const FVector& Extent,
    const FVector& AxisX,
    const FVector& AxisY,
    const FVector& AxisZ,
    APlayerController* Controller,
    FVector2D& OutOrigin,
    FVector2D& OutSize
)
{
    // Create the oriented box
    FOrientedBox oBox;
    oBox.Center = Origin;
    oBox.ExtentX = Extent.X;
    oBox.ExtentY = Extent.Y;
    oBox.ExtentZ = Extent.Z;
    oBox.AxisX = AxisX;
    oBox.AxisY = AxisY;
    oBox.AxisZ = AxisZ;

    FVector OutVertices[8];
    oBox.CalcVertices(OutVertices);

    // Initialize an empty 2D bounding box using FBox2D default constructor.
    FBox2D ActorBox2D(FVector2D(FLT_MAX, FLT_MAX), FVector2D(-FLT_MAX, -FLT_MAX));

    // Project the 3D vertices to screen space
    for (uint8 BoundsPointItr = 0; BoundsPointItr < 8; BoundsPointItr++)
    {
        FVector2D ScreenLocation;

        // Project vertex into screen space.
        if (Controller)
        {
            Controller->ProjectWorldLocationToScreen(OutVertices[BoundsPointItr], ScreenLocation);
            ActorBox2D += ScreenLocation; // Expand the box to include the projected point
        }
    }

    // Get the final 2D bounding box
    OutOrigin = ActorBox2D.Min;
    //OutOrigin.X += 5;
    //OutOrigin.Y += 127;
    OutSize = ActorBox2D.GetSize();
    OutSize.X += OutOrigin.X;
    OutSize.Y += OutOrigin.Y;
}

void UBlueprintHelperLibrary::AdjustCubeToMatchMesh(AActor* TargetActor, FName CubeTag)
{
    if (!TargetActor || CubeTag.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid actor or tag!"));
        return;
    }

    // Find the Static Mesh Component
    UStaticMeshComponent* StaticMeshComponent = TargetActor->FindComponentByClass<UStaticMeshComponent>();
    if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid static mesh found on actor!"));
        return;
    }

    // Find the Cube Component by tag
    UPrimitiveComponent* CubeComponent = nullptr;
    for (UActorComponent* Component : TargetActor->GetComponents())
    {
        if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
        {
            if (PrimitiveComp->ComponentHasTag(CubeTag))
            {
                CubeComponent = PrimitiveComp;
                break;
            }
        }
    }

    if (!CubeComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("No component with the specified tag found!"));
        return;
    }

    // Get the bounds of the static mesh
    const FBoxSphereBounds MeshBounds = StaticMeshComponent->GetStaticMesh()->GetBounds();
    const FVector MeshExtent = MeshBounds.BoxExtent * 2.0f; // Convert half-size to full-size
    const FVector MeshCenter = MeshBounds.Origin; // Center of the bounding box relative to the mesh

    // Get the default size of the cube
    FVector CubeDefaultExtent = FVector(100.0f, 100.0f, 100.0f); // Default Unreal cube size is 100 units in each dimension
    if (UStaticMeshComponent* CubeMesh = Cast<UStaticMeshComponent>(CubeComponent))
    {
        CubeDefaultExtent = CubeMesh->GetStaticMesh()->GetBounds().BoxExtent * 2.0f;
    }

    // Calculate scale factor
    FVector ScaleFactor = MeshExtent / CubeDefaultExtent;

    // Adjust the cube scale
    CubeComponent->SetWorldScale3D(ScaleFactor);

    // Calculate the world position of the cube to align it with the static mesh bounds
    FVector StaticMeshWorldLocation = StaticMeshComponent->GetComponentLocation(); // Static mesh world position
    FVector CubeOffset = MeshCenter * StaticMeshComponent->GetComponentScale(); // Offset considering mesh's scale
    FVector NewCubePosition = StaticMeshWorldLocation + CubeOffset;

    // Set the cube position
    CubeComponent->SetWorldLocation(NewCubePosition);
}

bool UBlueprintHelperLibrary::PerformAdjustedRaycast(
    AActor* CameraOwner,
    float PitchOffset,
    float YawOffset,
    FHitResult& OutHitResult,
    float MaxRayDistance
)
{
    if (!CameraOwner)
    {
        UE_LOG(LogTemp, Warning, TEXT("No component with the specified tag found!"));
        return false;
    }

    // Find the camera component
    UCameraComponent* Camera = CameraOwner->FindComponentByClass<UCameraComponent>();
    if (!Camera)
    {
        return false;
    }

    // Get camera properties
    FVector CameraLocation = Camera->GetComponentLocation();
    FRotator CameraRotation = Camera->GetComponentRotation();

    // Apply rotation offsets
    FRotator AdjustedRotation = CameraRotation + FRotator(PitchOffset, YawOffset, 0.0f);
    FVector RotatedForwardVector = AdjustedRotation.Vector();

    // Calculate the raycast endpoint
    FVector RayEnd = CameraLocation + (RotatedForwardVector * MaxRayDistance);

    // Perform the raycast
    FCollisionQueryParams TraceParams(FName(TEXT("RaycastTrace")), true, CameraOwner);
    bool bHit = CameraOwner->GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        CameraLocation,
        RayEnd,
        ECC_Visibility,
        TraceParams
    );

    FColor LineColor = bHit ? FColor::Green : FColor::Red;
    DrawDebugLine(CameraOwner->GetWorld(), CameraLocation, RayEnd, LineColor, true, 200.0f);

    // Optional: Draw debug line
#if WITH_EDITOR

#endif

    return bHit;
}

bool UBlueprintHelperLibrary::GetStaticMeshScreenBoundsAccurate(const UStaticMeshComponent* MeshComponent, FVector2D& TopLeft, FVector2D& BottomRight)
{
    if (!MeshComponent || !MeshComponent->GetWorld() || !GEngine)
    {
        return false;
    }

    const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(MeshComponent, 0);
    if (!PlayerController)
    {
        return false;
    }

    const UStaticMesh* StaticMesh = MeshComponent->GetStaticMesh();
    if (!StaticMesh || !StaticMesh->GetRenderData())
    {
        return false;
    }

    const FTransform ComponentTransform = MeshComponent->GetComponentTransform();
    const FStaticMeshLODResources& LODResources = StaticMesh->GetRenderData()->LODResources[0];

    FVector2D MinScreen(FLT_MAX, FLT_MAX);
    FVector2D MaxScreen(-FLT_MAX, -FLT_MAX);

    // Access vertex positions
    const int32 VertexCount = LODResources.VertexBuffers.PositionVertexBuffer.GetNumVertices();
    for (int32 VertexIndex = 0; VertexIndex < VertexCount; ++VertexIndex)
    {
        // Convert FVector3f to FVector
        const FVector3f VertexPositionF = LODResources.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
        const FVector VertexPosition(VertexPositionF.X, VertexPositionF.Y, VertexPositionF.Z);

        const FVector WorldPosition = ComponentTransform.TransformPosition(VertexPosition);

        FVector2D ScreenPosition;
        if (PlayerController->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition))
        {
            MinScreen.X = FMath::Min(MinScreen.X, ScreenPosition.X);
            MinScreen.Y = FMath::Min(MinScreen.Y, ScreenPosition.Y);

            MaxScreen.X = FMath::Max(MaxScreen.X, ScreenPosition.X);
            MaxScreen.Y = FMath::Max(MaxScreen.Y, ScreenPosition.Y);
        }
    }

    if (MinScreen.X == FLT_MAX || MinScreen.Y == FLT_MAX || MaxScreen.X == -FLT_MAX || MaxScreen.Y == -FLT_MAX)
    {
        return false; // Bounding box not visible on screen
    }

    TopLeft = MinScreen;
    BottomRight = MaxScreen;
    return true;
}
