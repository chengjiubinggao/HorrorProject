// Fill out your copyright notice in the Description page of Project Settings.


#include "ReversiBoardComponent.h"

// Sets default values for this component's properties
UReversiBoardComponent::UReversiBoardComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UReversiBoardComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Cells.Num() != Size * Size) {
		ResetBoard();
	}

	// ...
}


// Called every frame
void UReversiBoardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UReversiBoardComponent::ResetBoard() {
	Cells.SetNum(Size * Size);
	for (auto& C : Cells) C = ECell::Empty;

	const int m = Size / 2 - 1;
	Cells[Index(m, m)] = ECell::White;
	Cells[Index(m + 1, m)] = ECell::Black;
	Cells[Index(m, m + 1)] = ECell::Black;
	Cells[Index(m + 1, m + 1)] = ECell::White;

	CurrentPlayer = EPlayer::Black;
	OnBoardChanged.Broadcast();
}

int32 UReversiBoardComponent::CountFlipsIfPlace(int32 X, int32 Y, EPlayer P, TArray<int32>* OutFlip) const {
	if (!InBounds(X, Y) || Cells[Index(X, Y)] != ECell::Empty) return 0;

	static const int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
	static const int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

	const ECell PlayerCell = PlayerToCell(P);
	const ECell OppPlayerCell = PlayerToCell(Opp(P));

	int32 Total = 0;
	TArray<int32> Tmp;

	for (int dir = 0; dir < 8; ++dir) {
		int cx = X + dx[dir];
		int cy = Y + dy[dir];
		TArray<int32> Line;
		while (InBounds(cx, cy) && Cells[Index(cx, cy)] == OppPlayerCell) {
			Line.Add(Index(cx, cy));
			cx += dx[dir];
			cy += dy[dir];
		}
		if (Line.Num() > 0 && InBounds(cx, cy) && Cells[Index(cx, cy)] == PlayerCell) {
			Total += Line.Num();
			Tmp.Append(Line);
		}

	}

	if (OutFlip && Total > 0) *OutFlip = MoveTemp(Tmp);
	return Total;
}

bool UReversiBoardComponent::TryPlaceAt(int32 X, int32 Y, bool bSimulate, int32& OutFlippedCount) {
	TArray<int32> Flips;
	const int32 N = CountFlipsIfPlace(X, Y, CurrentPlayer, &Flips);
	OutFlippedCount = N;
	if (N <= 0) return false;

	if (!bSimulate) {
		Cells[Index(X, Y)] = PlayerToCell(CurrentPlayer);
		for (auto Flip : Flips) Cells[Flip] = PlayerToCell(CurrentPlayer);

		NextTurnWithSkipIfNeeded();
		OnBoardChanged.Broadcast();
	}

	bool bBothNoMoves = false;
	if (IsGameOver(bBothNoMoves)) {
		OnGameOver.Broadcast(bBothNoMoves);
	}

	return true;
}

void UReversiBoardComponent::NextTurnWithSkipIfNeeded() {
	CurrentPlayer = Opp(CurrentPlayer);

	if (!HasAnyValidMove(CurrentPlayer)) {
		CurrentPlayer = Opp(CurrentPlayer);
	}

}

void UReversiBoardComponent::GetValidMoves(TArray<FIntPoint8>& OutMoves) const {
	OutMoves.Reset();
	for (int y = 0; y < Size; ++y) {
		for (int x = 0; x < Size; ++x) {
			if (Cells[Index(x, y)] == ECell::Empty && CountFlipsIfPlace(x, y, CurrentPlayer, nullptr)) {
				OutMoves.Add(FIntPoint8(x, y));
			}
		}
	}
}

bool UReversiBoardComponent::HasAnyValidMove(EPlayer P) const {
	for (int y = 0; y < Size; ++y)
		for (int x = 0; x < Size; ++x)
			if (Cells[Index(x, y)] == ECell::Empty && CountFlipsIfPlace(x, y, P, nullptr) > 0)
				return true;
	return false;
}

void UReversiBoardComponent::GetScore(int32& OutBlack, int32& OutWhite) const {
	OutBlack = OutWhite = 0;
	for (ECell C : Cells) {
		if (C == ECell::Black) ++OutBlack;
		else if (C == ECell::White) ++OutWhite;
	}
}

bool UReversiBoardComponent::IsGameOver(bool& bBothNoMoves) const {
	const bool BlackHas = HasAnyValidMove(EPlayer::Black);
	const bool WhiteHas = HasAnyValidMove(EPlayer::White);
	bBothNoMoves = (!BlackHas && !WhiteHas);
	if (bBothNoMoves) return true;

	for (ECell C : Cells) if (C == ECell::Empty) return false;
	return true;
}

bool UReversiBoardComponent::PlayAIMove(EPlayer AIPlayer) {
	if (CurrentPlayer != AIPlayer) return false;

	TArray<FIntPoint8> ValidMoves;
	GetValidMoves(ValidMoves);

	if (ValidMoves.Num() == 0) return false;

	int32 BestFlips = -1;
	TArray<FIntPoint8> BestMoves;

	for (const auto& Move : ValidMoves) {
		int32 Flips = CountFlipsIfPlace(Move.X, Move.Y, AIPlayer, nullptr);
		if (Flips > BestFlips) {
			BestFlips = 1;
			BestMoves = { Move };
		}
		else if (Flips == BestFlips) {
			BestMoves.Add(Move);
		}
	}

	if (BestMoves.Num() == 0)
		return false;

	const int32 Index = FMath::RandRange(0, BestMoves.Num() - 1);
	const FIntPoint8& Choice = BestMoves[Index];

	int32 OutFlipped = 0;
	TryPlaceAt(Choice.X, Choice.Y, false, OutFlipped);

	return true;
}