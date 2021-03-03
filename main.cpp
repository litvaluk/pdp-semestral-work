#include <iostream>
#include <algorithm>
#include <vector>

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

vector<Square> initBoard(int k) {
  vector<Square> board;
  for (int i = 0; i < k*k; i++) {
    char piece;
    cin >> piece;
    switch (piece) {
    case '-':
      board.push_back(EMPTY);
      break;
    case 'P':
      board.push_back(PIECE);
      break;
    case 'V':
      board.push_back(ROOK);
      break;
    case 'J':
      board.push_back(KNIGHT);
      break;
    }
  }
  return board;
}

int getRookIndex(vector<Square> board, int k) {
  for (int i = 0; i < k*k; i++) {
    if (board[i] == ROOK) {
      return i;
    }
  }
  exit(1);
}

int getKnightIndex(vector<Square> board, int k) {
  for (int i = 0; i < k*k; i++) {
    if (board[i] == KNIGHT) {
      return i;
    }
  }
  exit(1);
}

bool knightCanTake(int index, vector<Square> board, int k) {
  int i = index / k;
  int j = index % k;

  // - # - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j > 0 && board[index - 2*k - 1] == PIECE) {
    return true;
  }

  // - - - # -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j < k-1 && board[index - 2*k + 1] == PIECE) {
    return true;
  }

  // - - - - -
  // # - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j > 1 && board[index - k - 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - #
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j < k-2 && board[index - k + 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // # - - - -
  // - - - - -
  if (i < k-1 && j > 1 && board[index + k - 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - #
  // - - - - -
  if (i < k-1 && j < k-2 && board[index + k + 2] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - # - - -
  if (i < k-2 && j > 0 && board[index + 2*k - 1] == PIECE) {
    return true;
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - # -
  if (i < k-2 && j < k-1 && board[index + 2*k + 1] == PIECE) {
    return true;
  }

  return false;
}

vector<Move> nextKnight(int index, vector<Square> board, int k) {
  vector<Move> moves;

  int i = index / k;
  int j = index % k;

  // # - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j > 0 && board[index - 2*k - 1] != ROOK) {
    Move move;
    move.index = index - 2*k - 1;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - #
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 1 && j < k-1 && board[index - 2*k + 1] != ROOK) {
    Move move;
    move.index = index - 2*k + 1;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // # - - - -
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j > 1 && board[index - k - 2] != ROOK) {
    Move move;
    move.index = index - k - 2;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // - - - - #
  // - - K - -
  // - - - - -
  // - - - - -
  if (i > 0 && j < k-2 && board[index - k + 2] != ROOK) {
    Move move;
    move.index = index - k + 2;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // # - - - -
  // - - - - -
  if (i < k-1 && j > 1 && board[index + k - 2] != ROOK) {
    Move move;
    move.index = index + k - 2;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - #
  // - - - - -
  if (i < k-1 && j < k-2 && board[index + k + 2] != ROOK) {
    Move move;
    move.index = index + k + 2;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - # - - -
  if (i < k-2 && j > 0 && board[index + 2*k - 1] != ROOK) {
    Move move;
    move.index = index + 2*k - 1;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }

  // - - - - -
  // - - - - -
  // - - K - -
  // - - - - -
  // - - - # -
  if (i < k-2 && j < k-1 && board[index + 2*k + 1] != ROOK) {
    Move move;
    move.index = index + 2*k + 1;
    move.value = board[move.index] == PIECE ? 2 : (knightCanTake(move.index, board, k) ? 1 : 0);
    moves.push_back(move);
  }
  return moves;
}

bool rookCanTake(int index, vector<Square> board, int k) {
  // up
  for (int i = index-k; i >= 0; i-=k) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      return true;
    }
  }

  // right
  for (int i = index+1; i%k != 0; i++) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      return true;
    }
  }

  // down
  for (int i = index+k; i < k*k; i+=k) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      return true;
    }
  }

  // left
  for (int i = index-1; i%k != k-1 && i >= 0; i--) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      return true;
    }
  }

  return false;  
}

vector<Move> nextRook(int index, vector<Square> board, int k) {
  vector<Move> moves;

  // up
  for (int i = index-k; i >= 0; i-=k) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.push_back(move);
      break;
    }
    if (board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board, k) ? 1 : 0;
      moves.push_back(move);
    }
  }
  
  // right
  for (int i = index+1; i%k != 0; i++) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.push_back(move);
      break;
    }
    if (board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board, k) ? 1 : 0;
      moves.push_back(move);
    }
  }
  
  // down
  for (int i = index+k; i < k*k; i+=k) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.push_back(move);
      break;
    }
    if (board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board, k) ? 1 : 0;
      moves.push_back(move);
    }
  }

  // left
  for (int i = index-1; i%k != k-1 && i >= 0; i--) {
    if (board[i] == KNIGHT) {
      break;
    }
    if (board[i] == PIECE) {
      Move move;
      move.index = i;
      move.value = 2;
      moves.push_back(move);
      break;
    }
    if (board[i] == EMPTY) {
      Move move;
      move.index = i;
      move.value = rookCanTake(i, board, k) ? 1 : 0;
      moves.push_back(move);
    }
  }

  return moves;
}

void printBoard(vector<Square> board, int k) {
  for (int i = 0; i < k*k; i++) {
    if (i != 0 && i%k == 0) {
      cout << endl;
    }
    switch (board[i]) {
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

void printMoves(vector<Move> moves) {
  for (int i = 0; i < moves.size(); i++) {
    cout << moves[i].index << "(" << moves[i].value << ") ";
  }
  cout << endl;
}

bool compareMoves(const Move &a, const Move &b) {
  return a.value > b.value;
}

vector<Square> executeMove(int from, int to, vector<Square> board, int k) {
  board[to] = board[from];
  board[from] = EMPTY;
  return board;
}

int numberOfRemainingPieces(vector<Square> board, int k) {
  int n = 0;
  for (int i = 0; i < k*k; i++) {
    if (board[i] == PIECE) {
      n++;
    }
  }
  return n;
}

long long int calls = 0;

vector<Move> dfs(int currentDepth, vector<Square> board, int k, int maxPieces, int remaining, int rookIndex, int knightIndex, vector<Move> currentMoves, vector<Move> bestSolution) {
  calls++;
  if (remaining <= 0) {
    return currentMoves;
  }
  if (currentDepth + remaining >= bestSolution.size()) {
    return bestSolution;
  }
  if (currentDepth%2 == 0) {
    // rook on the move
    vector<Move> moves = nextRook(rookIndex, board, k);
    sort(moves.begin(), moves.end(), compareMoves);
    for (int i = 0; i < moves.size(); i++) {
      vector<Square> executed = executeMove(rookIndex, moves[i].index, board, k);
      
      vector<Move> executedMoves = currentMoves;
      executedMoves.push_back(moves[i]);

      int newRemaining = board[moves[i].index] == PIECE ? remaining-1 : remaining;
      vector<Move> res = dfs(currentDepth+1, executed, k, maxPieces, newRemaining, moves[i].index, knightIndex, executedMoves, bestSolution);
      if (res.size() == maxPieces) {
        return res;
      }
      if (res.size() < bestSolution.size()) {
        bestSolution = res;
      }
    }
  } else {
    // knight on the move
    vector<Move> moves = nextKnight(knightIndex, board, k);
    sort(moves.begin(), moves.end(), compareMoves);
    for (int i = 0; i < moves.size(); i++) {
      vector<Square> executed = executeMove(knightIndex, moves[i].index, board, k);

      vector<Move> executedMoves = currentMoves;
      executedMoves.push_back(moves[i]);

      int newRemaining = board[moves[i].index] == PIECE ? remaining-1 : remaining;
      vector<Move> res = dfs(currentDepth+1, executed, k, maxPieces, newRemaining, rookIndex, moves[i].index, executedMoves, bestSolution);
      if (res.size() == maxPieces) {
        return res;
      }
      if (res.size() < bestSolution.size()) {
        bestSolution = res;
      }
    }
  }
  return bestSolution;
}

void printSolution(vector<Move> solutionMoves, int k) {
  cout << "Total moves: " << solutionMoves.size() << endl << "Number of dfs function calls: " << calls << endl << "Moves:" << endl;
  for (int i = 0; i < solutionMoves.size(); i++) {
    int rowIndex = solutionMoves[i].index / k;
    int colIndex = solutionMoves[i].index % k;

    if (i%2 == 0) {
      cout << "  ROOK   -> (";
    } else {
      cout << "  KNIGHT -> (";
    }

    cout << rowIndex+1 << ", " << colIndex+1 << ")";
    if (solutionMoves[i].value == 2) {
      cout << " *";
    }
    cout << endl;
  }  
}

int main(int argc, char const *argv[]) {
  int k, maxDepth;
  cin >> k >> maxDepth;
  
  vector<Square> board = initBoard(k);
  int remaining = numberOfRemainingPieces(board, k);
  
  vector<Move> currentMoves;
  vector<Move> bestSolution(maxDepth);

  vector<Move> res = dfs(0, board, k, remaining, remaining, getRookIndex(board, k), getKnightIndex(board, k), currentMoves, bestSolution);
  printSolution(res, k);
  return 0;
}
