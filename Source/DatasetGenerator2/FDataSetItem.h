#pragma once
#include "FDataSetItem.generated.h"


USTRUCT(BlueprintType)
struct FDataSetItem
{
	GENERATED_BODY()  

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Name")
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Description")  
	FString Description;

	// Reference to the mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh") 
	UStaticMesh* Mesh;

	// Default constructor
	/*FDataSet()
	: Name(TEXT("Unknown")), Description(TEXT("No Description")), Mesh(nullptr)
	{}
	
	// Constructor with parameters
	FDataSet(const FString& InName, const FString& InDescription, UStaticMesh* InMesh)
		: Name(InName), Description(InDescription), Mesh(InMesh)
	{}*/
};

