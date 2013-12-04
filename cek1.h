#ifndef _cek1_
#define _cek1_

//#define DEBUG_DIAGATTACK 1
//#define DEBUG_MOVESPRINTCB 1
//#define DEBUG_EVALPRINT 1
//#define DEBUG_MOVEGENPRINT 1
//#define DEBUG_HTPRINT 1
#define DEBUG_MOVESGETCNT 1
#define DEBUG_MOVEPROCESSVALIDATE 1

//#define USE_HASHTABLE 1

typedef unsigned long long u64;

//#define PRG_VERSION "CEK1 v20131125_1430\n"
#define STM_WHITE 'w'
#define STM_BLACK 'b'

#define MOVES_BUFSIZE 512
#define NUMOFPARALLELMOVES 512
#define S1KTEMPBUFSIZE 1024
#define S32TEMPBUFSIZE 32
#define UCICMDBUFSIZE S1KTEMPBUFSIZE

#define VALUE_QUEEN 900
#define VALUE_ROOK 500
#define VALUE_BISHOP 300
#define VALUE_KNIGHT 300
#define VALUE_PAWN 100
#define VALUE_KING ((VALUE_QUEEN+VALUE_ROOK+VALUE_BISHOP+VALUE_KNIGHT+VALUE_PAWN)/2)

#define UCIOPTION_CUSTOM_SHOWCURRMOVE 0x0080ULL

#define LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN 100

struct cb {
	u64 wk,wq,wr,wb,wn,wp;
	u64 bk,bq,br,bb,bn,bp;
	char sideToMove;
	int bk_underattack,wk_underattack;
	char sMoves[MOVES_BUFSIZE];
};

int cb_findbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves);

#endif

