// NI-PDP 20/21 - Lukáš Litvan
// ========================================================================================
// Task 4 - MPI approach with data parallelism for the rook and knight problem
// ========================================================================================
// Master slave moodel implementation using the OpenMPI (High Performance Message Passing
// Library). Master performs initial expansion of the state space using BFS to a certain
// depth and then sends the expanded states to the slaves. Each slave solves the part
// problem using data parallelism with OpenMP (BFS expansion, parallel for).
// There are 3 parameters: thread count (for parallel for), first expand depth (depth
// to expand at master), second expand depth (depth to expand at slave).
// When running on higher core count, it is more effective to set higher expand depth
// (both first and second).

#include <iostream>
#include <algorithm>
#include <omp.h>
#include <cstdio>
#include <vector>
#include <list>
#include <mpi.h>

#define MAX_K 19
#define MASTER_RANK 0

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

struct Message {
  bool end;
  int expandDepth;
  int threadCount;
  Board board;
  Moves best;
};

struct Parameters {
  int threadCount;
  int firstExpandDepth;
  int secondExpandDepth;
};

Parameters setParameters(int argc, char** argv) {
  Parameters parameters;

  // set threadCount
  if (argc > 1) {
    parameters.threadCount = atoi(argv[1]);
  } else {
    parameters.threadCount = omp_get_max_threads();
  }

  // set firstExpandDepth
  if (argc > 2) {
    parameters.firstExpandDepth = atoi(argv[2]);
  } else {
    parameters.firstExpandDepth = 2;
  }

  // set secondExpandDepth
  if (argc > 3) {
    parameters.secondExpandDepth = atoi(argv[3]);
  } else {
    parameters.secondExpandDepth = 3;
  }

  return parameters;
}

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

// board score computing function
int computeBoardScore(Board board) {
  Moves moves;
  if (board.depth%2 == 0) {
    moves = nextRook(board.rookIndex, board);
  } else {
    moves = nextKnight(board.knightIndex, board);
  }

  int movesScore = 0;
  for (int i = 0; i < moves.length; i++) {
    if (moves.moves[i].value > movesScore) {
      movesScore = moves.moves[i].value;
    }
  }

  return 2*board.remaining - movesScore;
}

// compare function for move sorting
bool compareMoves(const Move &a, const Move &b) {
  return a.value > b.value;
}

// compare function for board sorting
bool compareBoards(const Board &a, const Board &b) {
  return computeBoardScore(a) < computeBoardScore(b);
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

// sequential expansion by bfs
vector<Board> prepare(Board board, int depthLimit) {
  list<Board> queue;
  queue.push_back(board);

  while((!queue.empty()) && queue.front().depth < depthLimit) {
    board = queue.front();
    queue.pop_front();

    if (board.depth%2 == 0) {
      // rook on the move
      Moves moves = nextRook(board.rookIndex, board);
      for (int i = 0; i < moves.length; i++) {
        queue.push_back(executeMove(board.rookIndex, moves.moves[i], board));
      }
    } else {
      // knight on the move
      Moves moves = nextKnight(board.knightIndex, board);
      for (int i = 0; i < moves.length; i++) {
        queue.push_back(executeMove(board.knightIndex, moves.moves[i], board));
      }
    }
  }

  vector<Board> vec;
  int queueSize = queue.size();
  for (int i = 0; i < queueSize; i++) {
    vec.push_back(queue.front());
    queue.pop_front();
  }

  return vec;
}

Moves best;

// DFS-BB algorithm for the rook and knight problem using OpenMP tasks
void dfs(Board board) {
  // if there are no pieces left, set current moves as a solution, if better
  if (board.remaining <= 0 && board.depth < best.length) {
    #pragma omp critical
      best = board.performedMoves;
    return;
  }

  // branch and bound condition
  if (board.depth + board.remaining >= best.length) {
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
  cout << "Total moves: " << solutionMoves.length << endl << "Moves:" << endl;
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

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int numberOfProcesses;
  int processRank;
  MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
  MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

  if (processRank == MASTER_RANK) {
      // ===== master process ======
      double startTime = MPI_Wtime();

      Parameters parameters = setParameters(argc, argv);

      int k, maxDepth;
      cin >> k >> maxDepth;
      
      best.length = maxDepth;

      Board board = initBoard(k);
      int maxPieces = board.remaining;
      vector<Board> expanded = prepare(board, parameters.firstExpandDepth);
      sort(expanded.begin(), expanded.end(), compareBoards);

      // send message to each slave
      for (int i = 1; i < numberOfProcesses; i++) {
        Message message;
        message.end = false;
        message.expandDepth = parameters.secondExpandDepth;
        message.threadCount = parameters.threadCount;
        message.board = expanded[i-1];
        message.best = best;
        MPI_Send(&message, sizeof(message), MPI_PACKED, i, i, MPI_COMM_WORLD);
      }

      // when message from any slave is received, send another message to the same slave (until expanded is empty)
      for (int i = numberOfProcesses - 1; i < expanded.size(); i++) {
        Moves received;
        MPI_Status status;
        MPI_Recv(&received, sizeof(received), MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (received.length < best.length) {
          best = received;
        }
        Message message;
        message.end = false;
        message.expandDepth = parameters.secondExpandDepth;
        message.threadCount = parameters.threadCount;
        message.board = expanded[i-1];
        message.best = best;
        MPI_Send(&message, sizeof(message), MPI_PACKED, status.MPI_SOURCE, i, MPI_COMM_WORLD);
      }

      // wait for messages from all slaves
      for (int i = 1; i < numberOfProcesses; i++) {
        Moves received;
        MPI_Status status;
        MPI_Recv(&received, sizeof(received), MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (received.length < best.length) {
          best = received;
        }
      }

      // send end flag to all slaves
      for (int i = 1; i < numberOfProcesses; i++) {
        Message message;
        message.end = true;
        MPI_Send(&message, sizeof(message), MPI_PACKED, i, i, MPI_COMM_WORLD);
      }

      printSolution(best, k);
      double endTime = MPI_Wtime();
      cout << "Time: " << endTime - startTime << " seconds" << endl;
  } else {
      // ===== slave process =====
      while (true) {
        Message received;
        MPI_Status status;
        MPI_Recv(&received, sizeof(received), MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(received.end) {
          break;
        } else {
          vector<Board> expanded = prepare(received.board, received.expandDepth);
          best = received.best;

          #pragma omp parallel for num_threads(received.threadCount)
          for (int i = 0; i < expanded.size(); i++){
            dfs(expanded[i]);
          }

          MPI_Send(&best, sizeof(best), MPI_PACKED, 0, processRank, MPI_COMM_WORLD);
        }
      }
  }

  MPI_Finalize();
  return 0;
}
