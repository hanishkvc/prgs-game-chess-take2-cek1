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

//#define USE_ABSHORTKILLED 1

#define HTFIND_STRICTMODE 0
#define HTFIND_IGNORECURDEPTH 1

#define HTFIND_ADD 0
#define HTFIND_FIND 1

//#define MOVELIST_ADDMOVENUM_SMOVES 1
//#define MOVELIST_ADDMOVENUM_NBMOVES 1

//#define DO_SENDPERIODICINFO 1

#define DO_HANDLEKILLED 1

#ifdef DO_ONLYIFBETTER_ALWAYS
#define DO_FINDBEST_ONLYIFBETTER 1
#define DO_ABPRUN_ONLYIFBETTER 1
#else
#ifdef DO_ONLYIFBETTER_NEVER
#undef DO_FINDBEST_ONLYIFBETTER
#undef DO_ABPRUN_ONLYIFBETTER
#else
#define DO_FINDBEST_ONLYIFBETTER 1
#undef DO_ABPRUN_ONLYIFBETTER
#endif
#endif

typedef unsigned long long u64;

//#define PRG_VERSION "CEK1 v20131125_1430\n"
#define STM_WHITE 'w'
#define STM_BLACK 'b'

#define MOVES_BUFSIZE 512
#define CBSMOVES_BUFSIZE 1024
#define NUMOFPARALLELMOVES 512
#define S1KTEMPBUFSIZE 1024
#define S32TEMPBUFSIZE 32
#define UCICMDBUFSIZE S1KTEMPBUFSIZE

#define VQUEEN 900
#define VROOK 500
#define VBISHOP 300
#define VKNIGHT 300
#define VPAWN 100
#define VKING ((VQUEEN+VROOK+VBISHOP+VKNIGHT+VPAWN)*10)

#define VKING_ATTACKED (VKING*2)

//#define MATTHRESHOLD_MIDGAME (VKING*2+VQUEEN*2+VROOK*2+VBISHOP*2+VKNIGHT*2+VPAWN*8)
#define MATTHRESHOLD_MIDGAME (VKING*2+VQUEEN*2+VROOK*2+VBISHOP*2+VKNIGHT*2)
#define MATTHRESHOLD_ENDGAME (VKING*2+VROOK*2+VBISHOP+VKNIGHT)

// May be for side corresponding to OrigSideToMove logic should use a conservative strategy
// i.e threat weightage is smaller than protection weightage (say 4 and 8)
// AND for the opponent/other side the logic should use a balanced/even or aggressive strategy
// (say 8 and 8 or 8 and 4)
#define WEIGHTAGE_THREAT 8
#define WEIGHTAGE_PROTECTION 6
#define WEIGHTAGE_SCALE 8
#define WT_DIRECT 1
#define WT_INDIRECT 1

#define DIRMULT(V) (V*1)
#define INDMULT(V) ((V*1)/6)

#define EVALSMAT_DIV 1
#define EVALSTANDP_DIV 1
#define EVALSKINGUNDERATTACK_DIV 100
#define EVALSPOS_DIV 1
// OverAchiever Value Multiplier
#define OAVMULT(V) ((V*5)/4)
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

//#define NUMOFTHREADS 2
#ifndef NUMOFTHREADS
#define NUMOFTHREADS 1
#endif
#define THREAD_DEPTH 1

#define FBHINT_NORMAL 0
#define FBHINT_STATICEVALONLY 1
#define FBHINT_QUICK 2

#define MAXWHITEEVAL INT_MAX
#define MAXBLACKEVAL INT_MIN

#define GS_START	0
#define GS_MID		1
#define GS_END		2

struct cb {
	u64 wk,wq,wr,wb,wn,wp;
	u64 bk,bq,br,bb,bn,bp;
	char sideToMove,origSideToMove;
	int bk_underattack,wk_underattack;
	char sMoves[MOVES_BUFSIZE];
	int wkCanKsC, wkCanQsC;
	int bkCanKsC, bkCanQsC;
	int bk_killed, wk_killed;
	int gameState;
	int bCanEnPas, wCanEnPas;
};

int cb_findbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves, int hint, int bestW, int bestB);
int cb_qfindbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves, int hint, int bestW, int bestB);

#endif

