// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReversiBoardComponent.generated.h"

UENUM(BlueprintType)
enum class ECell : uint8 { Empty, Black, White};

UENUM(BlueprintType)
enum class EPlayer : uint8 { Black, White };

USTRUCT(BlueprintType)
struct FIntPoint8 {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 X = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 Y = 0;
	FIntPoint8() {}
	FIntPoint8(int32 InX, int32 InY) : X(InX), Y(InY) {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoardChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bBothNoMoves);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HORRORPROJECT_API UReversiBoardComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReversiBoardComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reversi")
	int32 Size = 8;

	UPROPERTY(BlueprintReadOnly, Category = "Reversi")
	TArray<ECell> Cells;

	UPROPERTY(BlueprintReadOnly, Category = "Reversi")
	EPlayer CurrentPlayer = EPlayer::Black;

	UPROPERTY(BlueprintAssignable, Category = "Reversi")
	FOnBoardChanged OnBoardChanged;

	UPROPERTY(BlueprintAssignable, Category = "Reversi")
	FOnGameOver OnGameOver;

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	void ResetBoard();

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	bool TryPlaceAt(int32 X, int32 Y, bool bSimulate, int32& OutFlippedCount);

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	void GetValidMoves(TArray<FIntPoint8>& OutMoves) const;

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	bool HasAnyValidMove(EPlayer Player) const;

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	void GetScore(int32& OutBlack, int32& OutWhite) const;

	UFUNCTION(BlueprintCallable, Category = "Reversi")
	bool IsGameOver(bool& bBothNoMoves) const;

	UFUNCTION(BlueprintPure, Category = "Reversi")
	int32 GetSize() const { return Size; }

	UFUNCTION(BlueprintPure, Category = "Reversi")
	ECell GetCell(int32 X, int32 Y) const { return Cells[Index(X, Y)]; }

	UFUNCTION(BlueprintCallable, Category = "Reversi|AI")
	bool PlayAIMove(EPlayer AIPlayer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE bool InBounds(int32 X, int32 Y) const { return X >= 0 && X < Size && Y >= 0 && Y < Size; }
	FORCEINLINE int32 Index(int32 X, int32 Y) const { return Y * Size + X; }

	ECell PlayerToCell(EPlayer P) const { return P == EPlayer::Black ? ECell::Black : ECell::White; }
	EPlayer Opp(EPlayer P) const { return P == EPlayer::Black ? EPlayer::White : EPlayer::Black; }


	int32 CountFlipsIfPlace(int32 X, int32 Y, EPlayer P, TArray<int32>* OutFlip) const;

	void NextTurnWithSkipIfNeeded();
};
