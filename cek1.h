#ifndef _cek1_
#define _cek1_

//#define DEBUG_DIAGATTACK 1
//#define DEBUG_MOVESPRINTCB 1
//#define DEBUG_EVALPRINT 1
//#define DEBUG_MOVEGENPRINT 1
//#define DEBUG_HTPRINT 1
//#define DEBUG_LEAFPRINT 1
//#define DEBUG_UNWINDSUMMARYPRINT 1
//#define DEBUG_UNWIND_SELECTION 1

//#define DEBUG_EVALSUMMARYPRINT 1

//#define DEBUG_UNWINDLOWLVLSUMMARYPRINT 1
//#define DEBUG_MOVESGETCNTPRINT 1

#define DEBUG_MOVEPROCESSVALIDATE 1

//#define USE_HASHTABLE 1

#define HTFIND_STRICTMODE 0
#define HTFIND_IGNORECURDEPTH 1

//#define MOVELIST_ADDMOVENUM_SMOVES 1
//#define MOVELIST_ADDMOVENUM_NBMOVES 1


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
#define VALUE_KING ((VALUE_QUEEN+VALUE_ROOK+VALUE_BISHOP+VALUE_KNIGHT+VALUE_PAWN)/4)

#define VALUE_KING_ATTACKED (VALUE_KING*2)

#define WEIGHTAGE_THREAT 8
#define WEIGHTAGE_PROTECTION 4
#define WEIGHTAGE_SCALE 8
#define WT_DIRECT 2
#define WT_INDIRECT 1

#define EVALSMAT_DIV 1
#define EVALSTANDP_DIV 100
#define EVALSKINGUNDERATTACK_DIV 100
// OverAchiever Value Multiplier
#define OAVMULT 2
// UnderAchiever Value Multiplier
#define UAVMULT(V) ((V*6)/8)

#define UCIOPTION_CUSTOM_SHOWCURRMOVE 0x0080ULL

#define LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN 100

#define SM_NORMAL		0
#define SM_KINGSIDECASTLE	'C'
#define SM_QUEENSIDECASTLE	'c'
#define SM_PROMOTE2QUEEN	'q'
#define SM_PROMOTE2ROOK		'r'
#define SM_PROMOTE2KNIGHT	'n'
#define SM_PROMOTE2BISHOP	'b'
#define SM_FALSE 1
#define SM_TRUE 0


struct cb {
	u64 wk,wq,wr,wb,wn,wp;
	u64 bk,bq,br,bb,bn,bp;
	char sideToMove,origSideToMove;
	int bk_underattack,wk_underattack;
	char sMoves[MOVES_BUFSIZE];
	int wkCanKsC, wkCanQsC;
	int bkCanKsC, bkCanQsC;
};

int cb_findbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves);

#endif

