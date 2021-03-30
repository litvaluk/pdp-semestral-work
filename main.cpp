// NI-PDP 20/21 - Lukáš Litvan
// ========================================================================================
// Task 3 - OpenMP data parallelism for the rook and knight problem
// ========================================================================================
// 

#include <iostream>
#include <algorithm>
#include <omp.h>
#include <cstdio>

#define MAX_K 19

using namespace std;

enum Square {
  EMPTY,
  ROOK,
  KNIGHT,
  PIECE
};

struct Move {
  int index;
  int value;
};

struct Moves {
  Move moves[2*MAX_K-2];
  int length;
};

struct Board {
  Square board[MAX_K*MAX_K];
  int k;
  int depth;
  int rookIndex;
  int knightIndex;
  int remaining;
  Moves performedMoves;
};

// finds and returns the position of the rook
int getRookIndex(Board board) {
  for (int i = 0; i < board.k*board.k; i++) {
    if (board.board[i] == ROOK) {
      return i;
    }
  }
  exit(1);
}

// finds and returns the position of the knight
int getKnightIndex(Board board) {
  for (int i = 0; i < board.k*board.k; i++) {
    if (board.board[i] == KNIGHT) {
      return i;
    }
  }
  exit(2);
}

Board initBoard(int k) {
  Board board;
  board.k = k;
  board.depth = 0;
  board.remaining = 0;

  Moves performedMoves;
  performedMoves.length = 0;
  board.performedMoves = performedMoves;
  
  for (int i = 0; i < board.k*board.k; i++) {
    char piece;
    cin >> piece;
    switch (piece) {
    case '-':
      board.board[i] = EMPTY;
      break;
    case 'P':
      board.board[i] = PIECE;
      board.remaining++;
      break;
    case 'V':
      board.board[i] = ROOK;
      break;
    case 'J':
      board.board[i] = KNIGHT;
      break;
    }
  }

  board.rookIndex = getRookIndex(board);
  board.knightIndex = getKnightIndex(board);
  
  return board;
}

// returns true if the knight can take any piece when positioned at the specified index, otherwise false
bool knightCanTake(int index, Board board) {
  int i = index / board.k;
  int j = index % board.k;

  // - # - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j > 0 && board.board[index - 2*board.k - 1] == PIECE) {
    return true;
  }

  // - - - # -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j < board.k-1 && board.board[index - 2*board.k + 1] == PIECE) {
    return true;
  }

  // - - - - -
  // # - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j > 1 && board.board[index - board.k - 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - #
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j < board.k-2 && board.board[index - board.k + 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // # - - - -
  // - - - - -
  if (i < board.k-1 && j > 1 && board.board[index + board.k - 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - #
  // - - - - -
  if (i < board.k-1 && j < board.k-2 && board.board[index + board.k + 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - # - - -
  if (i < board.k-2 && j > 0 && board.board[index + 2*board.k - 1] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - # -
  if (i < board.k-2 && j < board.k-1 && board.board[index + 2*board.k + 1] == PIECE) {
    return true;
  }

  return false;
}

// returns moves available to the knight positioned at the specified index
Moves nextKnight(int index, Board board) {
  Moves moves;
  moves.length = 0;

  int i = index / board.k;
  int j = index % board.k;

  // - # - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j > 0 && board.board[index - 2*board.k - 1] != ROOK) {
    Move move;
    move.index = index - 2*board.k - 1;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - # -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j < board.k-1 && board.board[index - 2*board.k + 1] != ROOK) {
    Move move;
    move.index = index - 2*board.k + 1;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // # - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j > 1 && board.board[index - board.k - 2] != ROOK) {
    Move move;
    move.index = index - board.k - 2;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // - - - - #
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j < board.k-2 && board.board[index - board.k + 2] != ROOK) {
    Move move;
    move.index = index - board.k + 2;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // # - - - -
  // - - - - -
  if (i < board.k-1 && j > 1 && board.board[index + board.k - 2] != ROOK) {
    Move move;
    move.index = index + board.k - 2;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - #
  // - - - - -
  if (i < board.k-1 && j < board.k-2 && board.board[index + board.k + 2] != ROOK) {
    Move move;
    move.index = index + board.k + 2;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - # - - -
  if (i < board.k-2 && j > 0 && board.board[index + 2*board.k - 1] != ROOK) {
    Move move;
    move.index = index + 2*board.k - 1;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - # -
  if (i < board.k-2 && j < board.k-1 && board.board[index + 2*board.k + 1] != ROOK) {
    Move move;
    move.index = index + 2*board.k + 1;
    move.value = board.board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board) ? 1 : 0);
    moves.moves[moves.length] = move;
    moves.length++;
  }
  return moves;
}

// returns true if the rook can take any piece when positioned at the specified index, otherwise false
bool rookCanTake(int index, Board board) {
  // up
  for (int i = index-board.k; i >= 0; i-=board.k) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      return true;
    }
  }

  // right
  for (int i = index+1; i%board.k != 0; i++) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      return true;
    }
  }

  // down
  for (int i = index+board.k; i < board.k*board.k; i+=board.k) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      return true;
    }
  }

  // left
  for (int i = index-1; i%board.k != board.k-1 && i >= 0; i--) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      return true;
    }
  }

  return false;  
}

// returns moves available to the rook positioned at the specified index
Moves nextRook(int index, Board board) {
  Moves moves;
  moves.length = 0;

  // up
  for (int i = index-board.k; i >= 0; i-=board.k) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.moves[moves.length] = move;
      moves.length++;
      break;
    }
    if (board.board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board) ? 1 : 0;
      moves.moves[moves.length] = move;
      moves.length++;
    }
  }
  
  // right
  for (int i = index+1; i%board.k != 0; i++) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.moves[moves.length] = move;
      moves.length++;
      break;
    }
    if (board.board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board) ? 1 : 0;
      moves.moves[moves.length] = move;
      moves.length++;
    }
  }
  
  // down
  for (int i = index+board.k; i < board.k*board.k; i+=board.k) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.moves[moves.length] = move;
      moves.length++;
      break;
    }
    if (board.board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board) ? 1 : 0;
      moves.moves[moves.length] = move;
      moves.length++;
    }
  }

  // left
  for (int i = index-1; i%board.k != board.k-1 && i >= 0; i--) {
    if (board.board[i] == KNIGHT) {
      break;
    }
    if (board.board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.moves[moves.length] = move;
      moves.length++;
      break;
    }
    if (board.board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board) ? 1 : 0;
      moves.moves[moves.length] = move;
      moves.length++;
    }
  }

  return moves;
}

void printBoard(Board board) {
  for (int i = 0; i < board.k*board.k; i++) {
    if (i != 0 && i%board.k == 0) {
      cout << endl;
    }
    switch (board.board[i]) {
    case EMPTY:
      cout << "- ";
      break;
    case ROOK:
      cout << "R ";
      break;
    case KNIGHT:
      cout << "K ";
      break;
    case PIECE:
      cout << "P ";
      break;
    }
  }
  cout << endl;
}

void printMoves(Moves moves) {
  for (int i = 0; i < moves.length; i++) {
    cout << moves.moves[i].index << "(" << moves.moves[i].value << ") ";
  }
  cout << endl;
}

// compare function for move sorting
bool compareMoves(const Move &a, const Move &b) {
  return a.value > b.value;
}

// returns board with performed move (chesspiece move from square A to square B)
Board executeMove(int from, Move move, Board board) {
  if(board.board[move.index] == PIECE) {
    board.remaining--;
  }
  
  board.board[move.index] = board.board[from];
  board.board[from] = EMPTY;
  board.depth = board.depth + 1;

  if (from == board.rookIndex) {
    board.rookIndex = move.index;
  } else if (from == board.knightIndex) {
    board.knightIndex = move.index;
  } else {
    exit(3);
  }

  board.performedMoves.moves[board.performedMoves.length] = move;
  board.performedMoves.length++;

  return board;
}

// returns number of pieces left for given board
int numberOfRemainingPieces(Board board) {
  int n = 0;
  for (int i = 0; i < board.k*board.k; i++) {
    if (board.board[i] == PIECE) {
      n++;
    }
  }
  return n;
}

long long int calls = 0;
int threadCount;
int maxPieces;
Moves bestSolution;

// DFS-BB algorithm for the rook and knight problem using OpenMP tasks
void dfs(Board board) {
  #pragma omp atomic
    calls++;
  
  // if there are no pieces left, set current moves as a solution, if better
  if (board.remaining <= 0 && board.depth < bestSolution.length) {
    #pragma omp critical
      bestSolution = board.performedMoves;
    return;
  }

  // branch and bound condition
  if (board.depth + board.remaining >= bestSolution.length) {
    return;
  }

  if (board.depth%2 == 0) {
    // rook on the move
    Moves moves = nextRook(board.rookIndex, board);
    sort(moves.moves, &moves.moves[moves.length], compareMoves);
    for (int i = 0; i < moves.length; i++) {
      Board executed = executeMove(board.rookIndex, moves.moves[i], board);
      dfs(executed);
    }
  } else {
    // knight on the move
    Moves moves = nextKnight(board.knightIndex, board);
    sort(moves.moves, &moves.moves[moves.length], compareMoves);
    for (int i = 0; i < moves.length; i++) {
      Board executed = executeMove(board.knightIndex, moves.moves[i], board);
      dfs(executed);
    }
  }
}

void printSolution(Moves solutionMoves, int k) {
  cout << "Total moves: " << solutionMoves.length << endl << "Number of dfs function calls: " << calls << endl << "Moves:" << endl;
  for (int i = 0; i < solutionMoves.length; i++) {
    int rowIndex = solutionMoves.moves[i].index / k;
    int colIndex = solutionMoves.moves[i].index % k;

    if (i%2 == 0) {
      cout << "  ROOK   -> (";
    } else {
      cout << "  KNIGHT -> (";
    }

    cout << rowIndex+1 << ", " << colIndex+1 << ")";
    if (solutionMoves.moves[i].value == 2) {
      cout << " *";
    }
    cout << endl;
  }  
}

int main(int argc, char const *argv[]) {
  if (argc > 1) {
    threadCount = atoi(argv[1]);
  } else {
    threadCount = omp_get_max_threads();
  }
  
  int k, maxDepth;
  cin >> k >> maxDepth;
  
  Board board = initBoard(k);
  maxPieces = board.remaining;
  bestSolution.length = maxDepth;

  dfs(board);
  printSolution(bestSolution, k);

  return 0;
}
