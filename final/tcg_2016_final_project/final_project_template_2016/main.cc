/*****************************************************************************\
 * Theory of Computer Games: Fall 2012
 * Chinese Dark Chess Search Engine Template by You-cheng Syu
 *
 * This file may not be used out of the class unless asking
 * for permission first.
 *
 * Modify by Hung-Jui Chang, December 2013
 \*****************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <utility>
#include <vector>
#include "anqi.hh"
#include "Protocol.h"
#include "ClientSocket.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <ctime>
#endif

const int DEFAULTTIME = 30;
typedef  int SCORE;
static const SCORE INF=10000001;
static const SCORE WIN=10000000;
SCORE SearchMax(const BOARD&, SCORE, SCORE, int, int, HashEntry*);


#ifdef _WINDOWS
DWORD Tick;     // 開始時刻
int   TimeOut;  // 時限
#else
clock_t Tick;     // 開始時刻
clock_t TimeOut;  // 時限
#endif
MOV   BestMove; // 搜出來的最佳著法

bool TimesUp() {
#ifdef _WINDOWS
  return GetTickCount() - Tick >= TimeOut;
#else
  return clock() - Tick > TimeOut;
#endif
}

// 一個重量不重質的審局函數
SCORE Eval(const BOARD &B) {
  int cnt[2] = {0, 0};
  for (POS p = 0; p < 32; p++) {
    const CLR c = GetColor(B.fin[p]);
    if (c != -1) cnt[c] += GetScore(B.fin[p]);
  }
  for (int i = 0; i < 14; i++) cnt[GetColor(FIN(i))] += B.cnt[i] * GetScore(FIN(i));
  for (POS p1 = 0; p1 < 32; p1++) {
    for (POS p2 = p1 + 1; p2 < 32; p2++) {
      if (GetColor(B.fin[p1]) == 0 && ChkGeq(B.fin[p1], B.fin[p2]))
        cnt[0] += isDiagonal(p1, p2) ? 10 : 10 - Distance(p1, p2);
      if (GetColor(B.fin[p1]) == 1 && ChkGeq(B.fin[p1], B.fin[p2]))
        cnt[1] += isDiagonal(p1, p2) ? 10 : 10 - Distance(p1, p2);
    }
  }
  return cnt[B.who] - cnt[B.who ^ 1];
}

// alpha-beta search
SCORE SearchMax(const BOARD &B, SCORE alpha, SCORE beta, int dep, int cut, HashEntry* hashTable) {

  if (B.ChkLose()) return -WIN;

  MOVLST lst;
  if (cut == 0 || TimesUp()) return Eval(B);

  SCORE m = -INF;
  SCORE n = beta;
  SCORE tmp;
  int sum;
  // move
  B.MoveGen(lst);
  if (lst.num == 0 && dep != 0) {
    tmp = Eval(B);
    // printf("depth = %d, who = %d, eval = %d\n", dep, B.who, tmp);
    return tmp;
  }
  std::vector<std::pair<SCORE, MOV> > srtlst(lst.num);
  for (int i = 0; i < lst.num; i++) {
    BOARD N(B);
    N.Move(lst.mov[i]);
    srtlst[i].first = Eval(N);
    srtlst[i].second = lst.mov[i];
  }
  std::sort(srtlst.begin(), srtlst.end());
  for (int i = 0; i < lst.num; i++) {
    BOARD N(B);
    N.Move(srtlst[i].second);
    tmp = -SearchMax(N, -n, -std::max(m, alpha), dep+1, cut-1, hashTable);
    if (tmp > m){
      if (n == beta || tmp >= beta) {
        m = tmp;
      }
      else {
        m = -SearchMax(N, -beta, -tmp, dep+1, cut-1, hashTable);
      }
      if (dep == 0) BestMove = srtlst[i].second;
    }
    if (m >= beta) break;
    n = std::max(m, alpha) + 1;
  }
  // flip
  if (dep == 0) {
    printf("Flip:\n");
    sum = 0;
    for (int i = 0; i < 14; i++) sum += B.cnt[i];
    for (POS p = 0; p < 32; p++) {
      if (B.fin[p] != FIN_X) continue;
      tmp = 0;
      for (int i = 0; i < 14; i++) {
        if (B.cnt[i] != 0) {
          BOARD N(B);
          N.Flip(p, FIN(i));
          tmp += -SearchMax(N, -beta, -alpha, dep+1, 3, hashTable) * B.cnt[i];
        }
      }
      tmp /= sum;
      if (tmp > m) {
        m = tmp;
        BestMove = MOV(p, p);
      }
    }
  }
  // printf("depth = %d, who = %d, ret = %d\n", dep, B.who, m);
  return m;
}

MOV Play(const BOARD &B) {
#ifdef _WINDOWS
  Tick = GetTickCount();
  TimeOut = (DEFAULTTIME - 3) * 1000;
#else
  Tick = clock();
  TimeOut = (DEFAULTTIME - 3) * CLOCKS_PER_SEC;
#endif
  POS p;
  int c = 0;

  // 新遊戲？隨機翻子
  if (B.who == -1){
    p = rand() % 32;
    printf("%d\n", p);
    return MOV(p, p);
  }

  HashEntry *hashTable = new HashEntry[1<<20];
  // 若搜出來的結果會比現在好就用搜出來的走法
  BestMove = MOV(-1, -1);
  SCORE result = SearchMax(B, -INF, INF, 0, 8, hashTable);
  printf("result = %d\n", result);
  printf("(%d %d)\n", BestMove.st >> 2, BestMove.st & 3);
  assert(BestMove.st != -1);
  delete hashTable;
  return BestMove;
}

FIN type2fin(int type) {
  switch(type) {
    case  1: return FIN_K;
    case  2: return FIN_G;
    case  3: return FIN_M;
    case  4: return FIN_R;
    case  5: return FIN_N;
    case  6: return FIN_C;
    case  7: return FIN_P;
    case  9: return FIN_k;
    case 10: return FIN_g;
    case 11: return FIN_m;
    case 12: return FIN_r;
    case 13: return FIN_n;
    case 14: return FIN_c;
    case 15: return FIN_p;
    default: return FIN_E;
  }
}
FIN chess2fin(char chess) {
  switch (chess) {
    case 'K': return FIN_K;
    case 'G': return FIN_G;
    case 'M': return FIN_M;
    case 'R': return FIN_R;
    case 'N': return FIN_N;
    case 'C': return FIN_C;
    case 'P': return FIN_P;
    case 'k': return FIN_k;
    case 'g': return FIN_g;
    case 'm': return FIN_m;
    case 'r': return FIN_r;
    case 'n': return FIN_n;
    case 'c': return FIN_c;
    case 'p': return FIN_p;
    default: return FIN_E;
  }
}

int main(int argc, char* argv[]) {

#ifdef _WINDOWS
  srand(Tick = GetTickCount());
#else
  srand(Tick = time(NULL));
#endif

  BOARD B;
  if (argc!=2) {
    TimeOut = (B.LoadGame("board.txt") - 3) * 1000;
    if (!B.ChkLose()) Output(Play(B));
    return 0;
  }
  Protocol *protocol;
  protocol = new Protocol();
  protocol->init_protocol(argv[0], atoi(argv[1]));
  int iPieceCount[14];
  char iCurrentPosition[32];
  int type, remain_time;
  bool turn;
  PROTO_CLR color;

  char src[3], dst[3], mov[6];
  History moveRecord;
  protocol->init_board(iPieceCount, iCurrentPosition, moveRecord, remain_time);
  protocol->get_turn(turn,color);

  TimeOut = (DEFAULTTIME - 3) * 1000;

  B.Init(iCurrentPosition, iPieceCount, (color == 2) ? (-1) : (int)color);

  MOV m;
  if(turn) // 我先
  {
    m = Play(B);
    sprintf(src, "%c%c", (m.st % 4) + 'a', m.st / 4 + '1');
    sprintf(dst, "%c%c", (m.ed % 4) + 'a', m.ed / 4 + '1');
    protocol->send(src, dst);
    protocol->recv(mov, remain_time);
    if (color == 2)
      color = protocol->get_color(mov);
    B.who = color;
    B.DoMove(m, chess2fin(mov[3]));
    protocol->recv(mov, remain_time);
    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
    B.DoMove(m, chess2fin(mov[3]));
  }
  else // 對方先
  {
    protocol->recv(mov, remain_time);
    if (color == 2)
    {
      color = protocol->get_color(mov);
      B.who = color;
    }
    else {
      B.who = color;
      B.who^=1;
    }
    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
    m.ed = (mov[2]=='(') ? m.st : (mov[3] - 'a' + (mov[4] - '1')*4);
    B.DoMove(m, chess2fin(mov[3]));
  }
  B.Display();
  while(1)
  {
    m = Play(B);
    sprintf(src, "%c%c",(m.st%4)+'a', m.st/4+'1');
    sprintf(dst, "%c%c",(m.ed%4)+'a', m.ed/4+'1');
    protocol->send(src, dst);
    protocol->recv(mov, remain_time);
    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
    B.DoMove(m, chess2fin(mov[3]));
    B.Display();

    protocol->recv(mov, remain_time);
    m.st = mov[0] - 'a' + (mov[1] - '1')*4;
    m.ed = (mov[2]=='(')?m.st:(mov[3] - 'a' + (mov[4] - '1')*4);
    B.DoMove(m, chess2fin(mov[3]));
    B.Display();
  }

  return 0;
}
