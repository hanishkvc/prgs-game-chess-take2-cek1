/*
 * cek1 - My 2nd chess engine (first in recent years). It is based on bitboards concept.
 * FIXME:
 *   1. Have to identify moves which attack the king and increase the value greatly.
 *      So that A given side gives high priority to attack the king.
 *      AND the side whose king is under attack can take actions to escape if possible.
 */
#include <stdio.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

#ifdef USE_THREAD
#include <pthread.h>
#endif

/*
#define handle_error_en(en, msg) \
		do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
*/

void handle_error_en(int en, char *msg)
{
	do {
		errno = en;
		perror(msg);
		exit(EXIT_FAILURE);
	} while (0);
}

#include "makeheader.h"
#include "cek1.h"

FILE *fLog;
FILE *fLogM;
pid_t myPID = 0;

struct cb gb;
struct timespec gtsStart, gtsDiff;
long gPrevDTime = 0;
u64 gMovesCnt = 0;
int gStartMoveNum = 0;
int gUCIOption = 0;
int gGameDepth = 3;
int gGameHash = 32;

// Has this is a multiline macro, always use it inside braces
#define send_resp_ex(sBuffer,sSize,...) snprintf(sBuffer,sSize,__VA_ARGS__); send_resp(sBuffer);
#define dbg_cb_bb_print dummy
#define dbg_log(file,...) fprintf(file,__VA_ARGS__)
#define dbgs_log dbgex_log
#define ddbg_log(file,...) dummy()
/*
#define ddbg_log(file,...) #ifdef DDEBUGLOG	\n\
				dbg_log(file,__VA_ARGS__); \n\
				#endif
*/

void dummy() 
{
}

void dbgex_log(FILE *f,char *fmt,...)
{
	va_list vaC;
	va_start(vaC,fmt);
	vfprintf(f,fmt,vaC);
	va_end(vaC);
	fflush(f);
}

long diff_clocktime(struct timespec *tsStart)
{
	long diffS,diffNS,msDiff;

	struct timespec tsCur;
	if(clock_gettime(CLOCK_REALTIME_COARSE,&tsCur) != 0) {
		dbg_log(fLog,"FIXME:diff_clocktime:clock_gettime failed\n");
		exit(50);
	}
	diffS = tsCur.tv_sec - tsStart->tv_sec;
	diffNS = abs(tsCur.tv_nsec - tsStart->tv_nsec);
	msDiff = diffS*1000 + (diffNS/1000000);
	return msDiff;
}

void send_resp(char *sBuf)
{
	printf("%s",sBuf);
	fflush(stdout);
	dbg_log(fLog,"SENT:%s",sBuf);
	fflush(fLog);
	dbg_log(fLogM,"SENT:%s",sBuf);
	fflush(fLogM);
}

int cb_strloc2bbpos(char *sMov)
{
	int r,f;

	f=(int)sMov[0]-'a';
	r=(int)sMov[1]-'1';
	if(((r < 0) || (r > 7)) || ((f < 0) || (f > 7)))
		return -1;
	return(r*8+f);	
}

char bbpos2strloc_array[64][4] = {
	"a1","b1","c1","d1","e1","f1","g1","h1",
	"a2","b2","c2","d2","e2","f2","g2","h2",
	"a3","b3","c3","d3","e3","f3","g3","h3",
	"a4","b4","c4","d4","e4","f4","g4","h4",
	"a5","b5","c5","d5","e5","f5","g5","h5",
	"a6","b6","c6","d6","e6","f6","g6","h6",
	"a7","b7","c7","d7","e7","f7","g7","h7",
	"a8","b8","c8","d8","e8","f8","g8","h8" };

char *cb_bbpos2strloc(int iPos, char *sBuf)
{
	if(iPos > 63)
		return NULL;
	strncpy(sBuf,bbpos2strloc_array[iPos],4);
	return sBuf;
}

int cb_bb_setpos(u64 *bb, int r, int f)
{
	int off = r*8+f;
	u64 pos = 0x1;

	if((f < 0) || (f > 7)) {
		dbg_log(fLog,"ERROR:cb_bb_setpos: file went beyond a-h :%d\n",f);
		//exit(100);
		return -1;
	}
	if((r < 0) || (r > 7)) {
		dbg_log(fLog,"ERROR:cb_bb_setpos: rank went beyond 1-8 :%d\n",r);
		//exit(100);
		return -2;
	}
	pos <<= off;
	*bb |= pos;
	dbg_log(fLog,"INFO:cb_bb_setpos: f%d_r%d\n",f,r);
	return 0;
}

void cb_bb_print(u64 bb)
{
	int off;
	u64 pos;
	int r,f;

	dbg_log(fLog,"INFO:cb_bb_print:\n");
	for(r = 7; r >= 0; r--) {
		for(f = 0; f < 8; f++) {
			off = r*8+f;
			pos = 0x1; pos <<= off;
			if(bb & pos)
				dbg_log(fLog,"*");
			else
				dbg_log(fLog,"-");
		}
		dbg_log(fLog,"\n");
	}
}

void cb_print(struct cb *cbC)
{
	int off;
	u64 pos;
	int r,f;

	dbg_log(fLog,"INFO:cb_print:%c to move\n",cbC->sideToMove);
	for(r = 7; r >= 0; r--) {
		for(f = 0; f < 8; f++) {
			off = r*8+f;
			pos = 0x1; pos <<= off;
			if(cbC->wk & pos)
				dbg_log(fLog,"K");
			else if(cbC->wq & pos)
				dbg_log(fLog,"Q");
			else if(cbC->wr & pos)
				dbg_log(fLog,"R");
			else if(cbC->wn & pos)
				dbg_log(fLog,"N");
			else if(cbC->wb & pos)
				dbg_log(fLog,"B");
			else if(cbC->wp & pos)
				dbg_log(fLog,"P");
			else if(cbC->bk & pos)
				dbg_log(fLog,"k");
			else if(cbC->bq & pos)
				dbg_log(fLog,"q");
			else if(cbC->br & pos)
				dbg_log(fLog,"r");
			else if(cbC->bn & pos)
				dbg_log(fLog,"n");
			else if(cbC->bb & pos)
				dbg_log(fLog,"b");
			else if(cbC->bp & pos)
				dbg_log(fLog,"p");
			else
				dbg_log(fLog,"*");
		}
		dbg_log(fLog,"\n");
	}
}

// Later will have to remove the program specific metadata
// from the move string

char *cb_2longnot(char *sIMov, char *sDest)
{
	int i;
	char *tDest = sDest;

	*tDest = '\0';
	for(i=0;i<strlen(sIMov);i++) {
		if((sIMov[i] != '-') && (sIMov[i] != 'P') && (sIMov[i] != 'x')) {
			*tDest=sIMov[i];
			tDest++;
		}
	}
	*tDest = '\0';
	return sDest;
}

char *cb_2simpnot(char *sIMov)
{
	return sIMov;
}

#include "generate_movebbs.c"
#include "evals.c"
#include "moves.c"

#define DO_FREE 0
#define DO_SAMESIDE 1
#define DO_ENEMYSIDE 2
#define DO_ERROR 0x0FFFFFFF
#define VAL_ERROR DO_ERROR
#define VALPW_BLACKSTUCK_GOODFORWHITE  32000
#define VALPW_WHITESTUCK_GOODFORBLACK -32000

int mv_dest_occupied(struct cb *cbC, char *sMov)
{
	int iPos;

	u64 bbSOcc = 0;
	u64 bbEOcc = 0;

	if(cbC->sideToMove == STM_WHITE) {
		bbSOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
		bbEOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	} else {
		bbSOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
		bbEOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	}
	iPos = sMov[3];
	if(iPos == -1)
		return DO_ERROR;
	if(bbSOcc & (1ULL<<iPos))
		return DO_SAMESIDE;
	else if(bbEOcc & (1ULL<<iPos))
		return DO_ENEMYSIDE;
	else
		return DO_FREE;
}

int move_validate(struct cb *cbC, char *sMov)
{
	int iRes;

	iRes = mv_dest_occupied(cbC,sMov);
	if((iRes == DO_ERROR) || (iRes == DO_SAMESIDE))
		return DO_ERROR;
	return iRes;
}

void mvhlpr_domovel_oncb(struct cb *cbC, char mPiece, int mSPos, int mDPos, int hint)
{
	int bEnPasFlagSet = 0;
	
	if(cbC->sideToMove == STM_WHITE) {
		if(mPiece == 'K') {
			cbC->wk &= ~(1ULL << mSPos);
			cbC->wk |= (1ULL << mDPos);
			if(hint != SM_NORMAL) {
#ifdef DEBUG_SMPRINT
				dbg_log(fLog,"INFO:domovel:Castling?\n");
				cb_print(cbC);
#endif
				if(hint == SM_KINGSIDECASTLE) {
					mvhlpr_domovel_oncb(cbC,'R',7,5,SM_NORMAL);
				} else if(hint == SM_QUEENSIDECASTLE) {
					mvhlpr_domovel_oncb(cbC,'R',0,3,SM_NORMAL);
				}
				cbC->wkCanKsC = SM_FALSE;
				cbC->wkCanQsC = SM_FALSE;
#ifdef DEBUG_SMPRINT
				cb_print(cbC);
#endif
			}
		} else if(mPiece == 'Q') {
			cbC->wq &= ~(1ULL << mSPos);
			cbC->wq |= (1ULL << mDPos);
		} else if(mPiece == 'R') {
			cbC->wr &= ~(1ULL << mSPos);
			cbC->wr |= (1ULL << mDPos);
		} else if(mPiece == 'N') {
			cbC->wn &= ~(1ULL << mSPos);
			cbC->wn |= (1ULL << mDPos);
		} else if(mPiece == 'B') {
			cbC->wb &= ~(1ULL << mSPos);
			cbC->wb |= (1ULL << mDPos);
		} else if(mPiece == 'P') {
			cbC->wp &= ~(1ULL << mSPos);
#ifndef ENABLE_SM
			cbC->wp |= (1ULL << mDPos);
#else
			if(hint == SM_NORMAL) {
				cbC->wp |= (1ULL << mDPos);
				if((abs(mDPos-mSPos)/8) == 2) {
					cbC->bCanEnPas = (mDPos%8)+1;
					bEnPasFlagSet = 1;
				}
				if((cbC->wCanEnPas) && ((mSPos%8) != (mDPos%8))) {
					if((40+(cbC->wCanEnPas-1)) == mDPos) {
						cbC->bp &= ~(1ULL << (mDPos-8));
					}
				}
			} else if(hint == SM_PROMOTE2QUEEN) {
				cbC->wq |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2ROOK) {
				cbC->wr |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2KNIGHT) {
				cbC->wn |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2BISHOP) {
				cbC->wb |= (1ULL << mDPos);
			}
#endif
		}
		cbC->bk &= ~(1ULL << mDPos);
		cbC->bq &= ~(1ULL << mDPos);
		cbC->br &= ~(1ULL << mDPos);
		cbC->bn &= ~(1ULL << mDPos);
		cbC->bb &= ~(1ULL << mDPos);
		cbC->bp &= ~(1ULL << mDPos);
	} else {
		if(mPiece == 'K') {
			cbC->bk &= ~(1ULL << mSPos);
			cbC->bk |= (1ULL << mDPos);
			if(hint != SM_NORMAL) {
#ifdef DEBUG_SMPRINT
				dbg_log(fLog,"INFO:domovel:Castling?\n");
				cb_print(cbC);
#endif
				if(hint == SM_KINGSIDECASTLE) {
					mvhlpr_domovel_oncb(cbC,'R',63,61,SM_NORMAL);
				} else if(hint == SM_QUEENSIDECASTLE) {
					mvhlpr_domovel_oncb(cbC,'R',56,59,SM_NORMAL);
				}
				cbC->bkCanKsC = SM_FALSE;
				cbC->bkCanQsC = SM_FALSE;
#ifdef DEBUG_SMPRINT
				cb_print(cbC);
#endif
			}
		} else if(mPiece == 'Q') {
			cbC->bq &= ~(1ULL << mSPos);
			cbC->bq |= (1ULL << mDPos);
		} else if(mPiece == 'R') {
			cbC->br &= ~(1ULL << mSPos);
			cbC->br |= (1ULL << mDPos);
		} else if(mPiece == 'N') {
			cbC->bn &= ~(1ULL << mSPos);
			cbC->bn |= (1ULL << mDPos);
		} else if(mPiece == 'B') {
			cbC->bb &= ~(1ULL << mSPos);
			cbC->bb |= (1ULL << mDPos);
		} else if(mPiece == 'P') {
			cbC->bp &= ~(1ULL << mSPos);
#ifndef ENABLE_SM
			cbC->bp |= (1ULL << mDPos);
#else
			if(hint == SM_NORMAL) {
				cbC->bp |= (1ULL << mDPos);
				if((abs(mDPos-mSPos)/8) == 2) {
					cbC->wCanEnPas = (mDPos%8)+1;
					bEnPasFlagSet = 1;
				}
				if((cbC->bCanEnPas) && ((mSPos%8) != (mDPos%8))) {
					if((16+(cbC->bCanEnPas-1)) == mDPos) {
						cbC->wp &= ~(1ULL << (mDPos+8));
					}
				}
			} else if(hint == SM_PROMOTE2QUEEN) {
				cbC->bq |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2ROOK) {
				cbC->br |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2KNIGHT) {
				cbC->bn |= (1ULL << mDPos);
			} else if(hint == SM_PROMOTE2BISHOP) {
				cbC->bb |= (1ULL << mDPos);
			}
#endif
		}
		cbC->wk &= ~(1ULL << mDPos);
		cbC->wq &= ~(1ULL << mDPos);
		cbC->wr &= ~(1ULL << mDPos);
		cbC->wn &= ~(1ULL << mDPos);
		cbC->wb &= ~(1ULL << mDPos);
		cbC->wp &= ~(1ULL << mDPos);
	}
#ifdef ENABLE_SM
	if(bEnPasFlagSet != 1) {
		cbC->wCanEnPas = 0;
		cbC->bCanEnPas = 0;
	}
#endif
}

int mvhlpr_domoveh_oncb(struct cb *cbC, char *sMov)
{
	int sPos = 0;
	int dPos = 0;
	int hint = SM_NORMAL;

	sPos = cb_strloc2bbpos(sMov);
	dPos = cb_strloc2bbpos(&sMov[2]);

	if((cbC->wk & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w') {
			if(sPos == 4) {
				if(dPos == 6) {
					hint = SM_KINGSIDECASTLE;
				} else if (dPos == 2) {
					hint = SM_QUEENSIDECASTLE;
				} else {
					cbC->wkCanKsC = SM_FALSE;
					cbC->wkCanQsC = SM_FALSE;
				}
			}
			mvhlpr_domovel_oncb(cbC,'K',sPos,dPos,hint);
		} else {
			return -1;
		}
	} else if((cbC->wq & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w')
			mvhlpr_domovel_oncb(cbC,'Q',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->wr & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w') {
			if(sPos == 0) {
				cbC->wkCanQsC = SM_FALSE;
			} else if(sPos == 7) {
				cbC->wkCanKsC = SM_FALSE;
			}
			mvhlpr_domovel_oncb(cbC,'R',sPos,dPos,hint);
		} else {
			return -1;
		}
	} else if((cbC->wn & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w')
			mvhlpr_domovel_oncb(cbC,'N',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->wb & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w')
			mvhlpr_domovel_oncb(cbC,'B',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->wp & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'w') {
			if((sMov[4] == SM_PROMOTE2QUEEN) || (sMov[4] == SM_PROMOTE2ROOK)
				|| (sMov[4] == SM_PROMOTE2KNIGHT) || (sMov[4] == SM_PROMOTE2BISHOP) ) {
				hint = sMov[4];
			}
			mvhlpr_domovel_oncb(cbC,'P',sPos,dPos,hint);
		} else {
			return -1;
		}
	}

	if((cbC->bk & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b') {
			if(sPos == 60) {
				if(dPos == 62) {
					hint = SM_KINGSIDECASTLE;
				} else if (dPos == 58) {
					hint = SM_QUEENSIDECASTLE;
				} else {
					cbC->bkCanKsC = SM_FALSE;
					cbC->bkCanQsC = SM_FALSE;
				}
			}
			mvhlpr_domovel_oncb(cbC,'K',sPos,dPos,hint);
		} else {
			return -1;
		}
	} else if((cbC->bq & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b')
			mvhlpr_domovel_oncb(cbC,'Q',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->br & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b') {
			if(sPos == 0) {
				cbC->bkCanQsC = SM_FALSE;
			} else if(sPos == 7) {
				cbC->bkCanKsC = SM_FALSE;
			}
			mvhlpr_domovel_oncb(cbC,'R',sPos,dPos,hint);
		} else {
			return -1;
		}
	} else if((cbC->bn & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b')
			mvhlpr_domovel_oncb(cbC,'N',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->bb & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b')
			mvhlpr_domovel_oncb(cbC,'B',sPos,dPos,hint);
		else {
			return -1;
		}
	} else if((cbC->bp & (1ULL << sPos)) != 0) {
		if(cbC->sideToMove == 'b') {
			if((sMov[4] == SM_PROMOTE2QUEEN) || (sMov[4] == SM_PROMOTE2ROOK)
				|| (sMov[4] == SM_PROMOTE2KNIGHT) || (sMov[4] == SM_PROMOTE2BISHOP) ) {
				hint = sMov[4];
			}
			mvhlpr_domovel_oncb(cbC,'P',sPos,dPos,hint);
		} else {
			return -1;
		}
	}
	return 0;
}

void cb_cmov2smov(char *cMov, char *sMov)
{
	char sTemp[8];
	char sDest[16];
	sDest[0] = cMov[0]; sDest[1] = 0;
	strcat(&sDest[1],cb_bbpos2strloc(cMov[1],sTemp));
	sDest[3] = cMov[2]; sDest[4] = 0;
	strcat(&sDest[4],cb_bbpos2strloc(cMov[3],sTemp));
	if(cMov[4] != SM_NORMAL) {
		cMov[5] = 0;
		if((cMov[4] != SM_KINGSIDECASTLE) && (cMov[4] != SM_QUEENSIDECASTLE))
			strcat(&sDest[6],&cMov[4]);
	}
	//sDest[6] = 0;
	strncpy(sMov,sDest,8);
}

#include "positionhash.c"

int move_process(struct cb *cbC, char *sMov, int curDepth, int maxDepth, int secs, int movNum, int altMovNum,char *sNextBestMoves, int hint,
			int bestW, int bestB)
{
	struct cb cbN;
	int iRes;
	char mPiece;
	int mSPos, mDPos;
	int mHint = SM_NORMAL;
	char sBuf[S1KTEMPBUFSIZE];
	char sNBMoves[MOVES_BUFSIZE];
	char s2LN[MOVES_BUFSIZE];
#ifdef USE_HASHTABLE
	//struct phash phTemp;
	int iVal;
	//int iPos;
	struct phash *phRes;
#endif

	bzero(sNBMoves,8);

	iRes = move_validate(cbC, sMov);
	if(iRes == DO_ERROR) {
		fprintf(stderr,"BUG:move_process\n");
		exit(-3);
#ifdef DEBUG_MOVEPROCESSVALIDATE
		cb_cmov2smov(sMov,sMov);
		dbg_log(fLog,"INFO:move_process: Move[%s] Dropped/Error\n",sMov);
#endif
		return DO_ERROR;
	}
	memcpy(&cbN,cbC,sizeof(struct cb));

	mPiece = sMov[0];
	mSPos = sMov[1];
	mDPos = sMov[3];
	mHint = sMov[4];
	cb_cmov2smov(sMov,sMov);
	if(mHint != SM_NORMAL) {
#ifdef DEBUG_SMPRINT_LL
		dbg_log(fLog,"INOF:move_process: SpecialMoveType[%c] Move[%s]\n",mHint,sMov);
#endif
	}

	if(gUCIOption & UCIOPTION_CUSTOM_SHOWCURRMOVE) {
		send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info depth %d currmove %s currmovenumber %d\n", curDepth, cb_2longnot(sMov,s2LN), altMovNum);
	}
	mvhlpr_domovel_oncb(&cbN,mPiece,mSPos,mDPos,mHint);
	if(cbC->sideToMove == STM_WHITE) {
#ifdef MOVELIST_ADDMOVENUM_SMOVES
		sprintf(sBuf,"%d.",movNum);
		strcat(cbN.sMoves,sBuf);
#endif
		cbN.sideToMove = STM_BLACK;
	}
	else {
		cbN.sideToMove = STM_WHITE;
		movNum += 1;
	}
	strcat(cbN.sMoves,sMov);
	strcat(cbN.sMoves," ");
#ifdef USE_HASHTABLE
	phRes = phash_find(gHashTable,&cbN,0x55aa,"",curDepth,HTFIND_FIND);
	if(phRes != NULL) {
		strcat(sNextBestMoves,phRes->sNBMoves);
		iVal = phRes->val;
#ifdef DEBUG_HTPRINT
		dbg_log(fLog,"INFO:move_process:HTHIT:%c:HTPos[%d]:val[%d]:cbNsMoves[%s],sNBMoves[%s]\n", 
				cbN.sideToMove, 9999, iVal, cbN.sMoves, sNextBestMoves);
		phash_print(phRes,"FromHashTable");
#endif
		return iVal;
	}
#endif
	if(hint == FBHINT_QUICK)
		iRes = cb_qfindbest(&cbN,curDepth,maxDepth,secs,movNum,sNBMoves,hint,bestW,bestB);
	else
		iRes = cb_findbest(&cbN,curDepth,maxDepth,secs,movNum,sNBMoves,hint,bestW,bestB);
	strcat(sNextBestMoves,sNBMoves); // FIXME: CAN BE REMOVED, CROSSVERIFY i.e sNBMoves can be replaced with sNextBestMoves in findbest
#ifdef DO_HANDLEKILLED
	if(cbN.wk_killed || cbN.bk_killed) {
		iRes = iRes*(maxDepth-curDepth+1);
	}
#endif
#ifdef USE_ABSHORTKILLED
	// if [w|b]k_killed is set, it means the eval returned by cb_findbest is for the move just executed
	// here before calling that cb_findbest.
	if(cbN.wk_killed) {
		if(cbN.sideToMove == STM_WHITE) // Black made a move which killed whites king, which means white shouldn't have played the previous level move or rather it was a illegal move
			return MAXBLACKEVAL;
		else {
			fprintf(stderr,"BUG:100:FIXME:TOTHINK: Cann't occur, can it now ????\n");
			exit(100);
		}
	} else if(cbN.bk_killed) {
		if(cbN.sideToMove == STM_BLACK) // White made a move which killed blacks king, which means black shouldn't have played the previous level move or rather it was a illegal move
			return MAXWHITEEVAL;
		else {
			fprintf(stderr,"BUG:101:FIXME:TOTHINK: Cann't occur, can it now ????\n");
			exit(101);
		}
	}
#endif
	return iRes;
}

//#undef CORRECTVALFOR_SIDETOMOVE
//#define CORRECTVALFOR_SIDETOMOVE

int cb_valpw2valpstm(char sideToMove, int valPW)
{
	if(sideToMove != STM_WHITE)
		return (-1*valPW);
	else
		return valPW;
}

struct moveprocessHlpr {
	struct cb *cbC;
	char *sMov;
	int curDepth;
	int maxDepth;
	int secs;
	int movNum;
	int altMovNum;
	char *sNextBestMoves;
	int hint;
	int bestW, bestB;
	int iRes;
};

void *moveprocess_hlpr(void *arg)
{
	struct moveprocessHlpr *mpH;
	mpH = arg;
	mpH->iRes = move_process(mpH->cbC,mpH->sMov,mpH->curDepth,mpH->maxDepth,mpH->secs,mpH->movNum,mpH->altMovNum,mpH->sNextBestMoves, mpH->hint,
					mpH->bestW, mpH->bestB);
	return (void*)0;
}

int cb_findbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves, int hint, int bestW, int bestB)
{
	int valPWStatic;
	int val;
	char sBuf[S1KTEMPBUFSIZE];
	char movs[NUMOFPARALLELMOVES][32];
	int movsEval[NUMOFPARALLELMOVES];
	int iMCnt, iCur, jCur;
	int iMaxPosVal,iMaxPosInd,iMaxNegVal,iMaxNegInd;
	int iMaxVal, iMaxInd;
	char sMaxPosNBMoves[MOVES_BUFSIZE],sMaxNegNBMoves[MOVES_BUFSIZE];
	char s2LN[MOVES_BUFSIZE];
	long lDTime = 0;
	char movsNBMoves[NUMOFPARALLELMOVES][MOVES_BUFSIZE];
	struct moveprocessHlpr mpH[NUMOFTHREADS];
	int xCur;
	int bShortCircuitSearch = 0;
	int bKingEntersCheck = 0;
	int kingEntersCheckEval = 0;
#ifdef USE_ABPRUNING
	char sABPNextBestMoves[MOVES_BUFSIZE];
	int iABPEval = 0;
#endif
#ifdef USE_BMPRUNING
	char movsInitial[NUMOFPARALLELMOVES][32];
	int tInd;
	int movsInd[NUMOFPARALLELMOVES];
	int iSkip;
	int iTMCnt;
	char *possibleBlundMove = NULL;
#endif
#ifdef USE_THREAD
	pthread_t ptIds[NUMOFTHREADS];
	int iTRes[NUMOFTHREADS];
	void *vpTRet[NUMOFTHREADS];
	int iTResSingle;
	pthread_attr_t attr;
#endif

	cb_check_kingkilled(cbC);
	lDTime = diff_clocktime(&gtsStart);
	if((cbC->wk_killed != 0) || (cbC->bk_killed != 0)) {
		valPWStatic = cb_evalpw(cbC);
		return valPWStatic;
	}

#if 0
	//*depthReached = curDepth;
	//The check for king underattack has to be thought thro and updated if required.
	if((curDepth == maxDepth) || (cbC->wk_underattack > 1) || (cbC->bk_underattack > 1)) {
		char sTemp[64];
		if((cbC->wk_underattack > 1) || (cbC->bk_underattack > 1)) 
			strcpy(sTemp,"string UnderCheckINVALIDMOVE");
		else
			strcpy(sTemp,"");
#ifdef DEBUG_LEAFPRINT
		dbg_log(fLog,"INFO:info score cp %d depth %d nodes %lld time %ld pv %s %s\n",
				val,curDepth,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN),sTemp);
#endif
		if((cbC->wk_underattack > 1) || (cbC->bk_underattack > 1))
			return DO_ERROR;
		return valPWStatic;
	}

	// sideToMove is actually nextSideToMove from symantic perspective
	// No point of checking what is the best move for the next side to move,
	// when the previous side is already under check and its move hasn't moved
	// it out of that check. OR the previous sides move has put itself
	// under check, as that is a invalid move which is not legally allowed
	// by chess rules.
	if( ((cbC->sideToMove == STM_BLACK) && (cbC->wk_underattack)) ||
			((cbC->sideToMove == STM_WHITE) && (cbC->bk_underattack)) ) {
#ifdef DEBUG_LEAFPRINT
		dbg_log(fLog,"info score cp %d depth %d nodes %lld time %ld pv %s string EntersCheckINVALIDMOVE\n",
				val,curDepth,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN));
#endif
		return DO_ERROR;
	}
#endif

	if(hint == FBHINT_STATICEVALONLY) {
		valPWStatic = cb_evalpw(cbC);
		return valPWStatic;
	}

	curDepth += 1;
	iMaxPosVal = 0; iMaxNegVal = 0; iMaxPosInd = -1; iMaxNegInd = -1;
#ifdef USE_BMPRUNING
	iMCnt = moves_get(cbC,movsInitial,0);

	for(iCur = 0; iCur < iMCnt; iCur+=1) {
		char sTMov[32];
		movsInd[iCur] = iCur;
		memcpy(sTMov,movsInitial[iCur],10);
		movsEval[iCur] = move_process(cbC,sTMov,curDepth,maxDepth,secs,movNum,iCur,movsNBMoves[iCur], FBHINT_STATICEVALONLY,bestW,bestB);
	}
	for(jCur = 0; jCur < iMCnt-1; jCur++) {
		for(iCur = 0; iCur < (iMCnt-1-jCur); iCur+=1) {
			if(movsEval[movsInd[iCur]] > movsEval[movsInd[iCur+1]]) {
				continue;
			} else {
				tInd = movsInd[iCur];
				movsInd[iCur] = movsInd[iCur+1];
				movsInd[iCur+1] = tInd;
			}
		}
	}
	iSkip = 0;
	possibleBlundMove = NULL;
	if(cbC->sideToMove == STM_WHITE) {
		for(jCur = 0; jCur < iMCnt; jCur++) {
			if(movsEval[movsInd[jCur]] == DO_ERROR) {
				iSkip++;
				continue;
			}
			memcpy(movs[jCur-iSkip],movsInitial[movsInd[jCur]],10);
		}
		if(iMCnt > 1)
			possibleBlundMove = movsInitial[movsInd[iMCnt-1]];
	} else {
		for(jCur = iMCnt-1; jCur >= 0; jCur--) {
			memcpy(movs[iMCnt-1-jCur],movsInitial[movsInd[jCur]],10);
			if((movsEval[movsInd[jCur]] == DO_ERROR) && (jCur != (iMCnt-1)) && (possibleBlundMove == NULL)) {
				possibleBlundMove = movsInitial[movsInd[jCur+1]];
			}
		}
	}
	iMCnt = iMCnt - iSkip;
	//fprintf(stderr,"INFO:TEMP:Check here\n");
#else
	iMCnt = moves_get(cbC,movs,0);
#endif

#ifdef USE_THREAD
	iTResSingle = pthread_attr_init(&attr);
	if(iTResSingle != 0)
		handle_error_en(iTResSingle,"attr_init");
	iTResSingle = pthread_attr_setstacksize(&attr,32000000);
	if (iTResSingle != 0)
		handle_error_en(iTResSingle,"attr_setstacksize");
#endif

#ifdef USE_BMPRUNING
	iTMCnt = iMCnt;
	if(curDepth > 2) {
		if(curDepth < 6)
			iTMCnt = 6;
		else if(curDepth < 8)
			iTMCnt = 4;
		else if(curDepth < 10)
			iTMCnt = 3;
		else
			iTMCnt = 2;
	}
	if(iMCnt > iTMCnt)
		iMCnt = iTMCnt;
#endif
	bShortCircuitSearch = 0;
	bKingEntersCheck = 0;
	kingEntersCheckEval = 0;
	for(jCur = 0; (jCur < iMCnt) && (bShortCircuitSearch == 0); jCur+=NUMOFTHREADS) {
		for(iCur = jCur; iCur < (jCur+NUMOFTHREADS); iCur++) {
			if(iCur >= iMCnt) {
				continue;
			}
			xCur=iCur-jCur;
			//send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info currmove %s currmovenumber %d\n", movs[iCur], iCur);
			strcpy(movsNBMoves[iCur],"");
			mpH[xCur].cbC = cbC;
			mpH[xCur].sMov = movs[iCur];
			mpH[xCur].curDepth = curDepth;
			mpH[xCur].maxDepth = maxDepth;
			mpH[xCur].secs = secs;
			mpH[xCur].movNum = movNum;
			mpH[xCur].altMovNum = iCur;
			mpH[xCur].sNextBestMoves = movsNBMoves[iCur];
			if(curDepth >= maxDepth)
				mpH[xCur].hint = FBHINT_QUICK;
			else
				mpH[xCur].hint = FBHINT_NORMAL;
			mpH[xCur].bestW = bestW;
			mpH[xCur].bestB = bestB;
#ifdef USE_THREAD
			if(curDepth <= THREAD_DEPTH)
			{
				iTRes[xCur]=pthread_create(&ptIds[xCur],&attr,moveprocess_hlpr,&mpH[xCur]);
				if(iTRes[xCur] != 0) {
					handle_error_en(iTRes[xCur],"pthread_create");
				}
#ifdef DEBUG_THREADPRINT
				dbg_log(fLog,"INFO:jCur[%d]iCur[%d]xCur[%d]:Created thread[%ld]\n",jCur,iCur,xCur,ptIds[xCur]);
#endif
			} 
			else
#endif 
			{
				moveprocess_hlpr(&mpH[xCur]);
				movsEval[iCur] = mpH[xCur].iRes;
			}
		}

		for(iCur = jCur; iCur < (jCur+NUMOFTHREADS); iCur++) {
			if(iCur >= iMCnt) {
				continue;
			}
#ifdef USE_THREAD
			if(curDepth <= THREAD_DEPTH)
			{
				xCur=iCur-jCur;
#ifdef DEBUG_THREADPRINT
				dbg_log(fLog,"INFO:jCur[%d]iCur[%d]xCur[%d]:Joining thread[%ld]\n",jCur,iCur,xCur,ptIds[xCur]);
#endif
				iTRes[xCur]=pthread_join(ptIds[xCur],&vpTRet[xCur]);
				if(iTRes[xCur] != 0) {
					handle_error_en(iTRes[xCur],"pthread_join");
				}
				movsEval[iCur] = mpH[xCur].iRes;
			}
#endif
			if(movsEval[iCur] == DO_ERROR) {
				continue;
			}
#ifdef USE_ABSHORTKILLED
			//if((movsEval[iCur] == MAXBLACKEVAL) || (movsEval[iCur] == MAXWHITEEVAL))
			//	return movsEval[iCur];
			// TODO: Have to also check about whose move it was and whose king was killed.
			// Currently this check is missing.
			if(movsEval[iCur] == MAXBLACKEVAL) {
				bKingEntersCheck = 1;
				movsEval[iCur] = movsEval[iCur]+1;
				kingEntersCheckEval = movsEval[iCur];
				strcpy(sNextBestMoves,"WHITE_BLUNDER");
				//return DO_ERROR;
				//continue;
			} else {
				 if(movsEval[iCur] == MAXWHITEEVAL) {
					bKingEntersCheck = 1;
					movsEval[iCur] = movsEval[iCur]-1;
					kingEntersCheckEval = movsEval[iCur];
					strcpy(sNextBestMoves,"BLACK_BLUNDER");
					//return DO_ERROR;
					//continue;
				}
			}
#endif
			if(iMaxPosInd == -1) {
				iMaxPosInd = iCur;
				iMaxPosVal = movsEval[iCur];
				iMaxNegInd = iCur;
				iMaxNegVal = movsEval[iCur];
				strcpy(sMaxPosNBMoves,movsNBMoves[iCur]);
				strcpy(sMaxNegNBMoves,movsNBMoves[iCur]);
			}
#ifdef DO_FINDBEST_ONLYIFBETTER
			if(movsEval[iCur] > iMaxPosVal) {
#else
			if(movsEval[iCur] >= iMaxPosVal) {
#endif
				iMaxPosVal = movsEval[iCur];
				iMaxPosInd = iCur;
				strcpy(sMaxPosNBMoves,movsNBMoves[iCur]);
			}
#ifdef DO_FINDBEST_ONLYIFBETTER
			if(movsEval[iCur] < iMaxNegVal) {
#else
			if(movsEval[iCur] <= iMaxNegVal) {
#endif
				iMaxNegVal = movsEval[iCur];
				iMaxNegInd = iCur;
				strcpy(sMaxNegNBMoves,movsNBMoves[iCur]);
			}

#ifdef USE_ABPRUNING
			if(cbC->sideToMove == STM_WHITE) {
				if(movsEval[iCur] > bestW) {
					bestW = movsEval[iCur];
				}
#ifdef DO_ABPRUN_ONLYIFBETTER
				if(movsEval[iCur] > bestB) {
#else
				if(movsEval[iCur] >= bestB) {
#endif
				// PrevLevel is B and it is searching for one of its moves' which provides the least value(which is also
				// same has least gain to White and Max gain for Black).
				// So for the Black's Move being evaluated currently, if a White move is found in the current level
				// which gives a value larger (which is good for White) than what was the response for earlier Black Moves.
				// Then NO WAY is this particular black move which is currently under study going to be selected. As this
				// move is relatively bad for Black. So we can skip testing/evaluating rest of the whites' response moves
				// - WHICH ALSO MEANS
				// the best white move identified till now is not the best White Response Move in absolute terms for the
				// given Black move under study currently, but is good enough for the decision to be taken that no further
				// moves require to be tested.
					if(!bShortCircuitSearch) {
					bShortCircuitSearch = 1;
					sprintf(sABPNextBestMoves,"%s %s", cb_2longnot(movs[iCur],s2LN), movsNBMoves[iCur]);
					iABPEval = movsEval[iCur];
					}
				}
			} else {
				if(movsEval[iCur] < bestB) {
					bestB = movsEval[iCur];
				}
#ifdef DO_ABPRUN_ONLYIFBETTER
				if(movsEval[iCur] < bestW) {
#else
				if(movsEval[iCur] <= bestW) {
#endif
					if(!bShortCircuitSearch) {
					bShortCircuitSearch = 1;
					sprintf(sABPNextBestMoves,"%s %s", cb_2longnot(movs[iCur],s2LN), movsNBMoves[iCur]);
					iABPEval = movsEval[iCur];
					}
				}
			}
#endif
		}
		if( bKingEntersCheck == 1) {
#ifdef DEBUG_LEAFPRINT
			dbg_log(fLog,"INFO:%c:BLUNDER_BY_OPPONENT_DETECTED,Shorting unwinding for CBwithPrevMoves[%s],MyNewMoveEval[%d]\n",
						cbC->sideToMove,cbC->sMoves,kingEntersCheckEval);
#endif
			return kingEntersCheckEval;
			//return DO_ERROR;
		}
	}

#ifdef USE_ABPRUNING
	if(bShortCircuitSearch) {
		strncpy(sNextBestMoves,sABPNextBestMoves,MOVES_BUFSIZE);
		return iABPEval;
	}
#endif

	// TODO:LATER:Maybe (to think) Send best value from the moves above or best N in multipv case

#ifdef DEBUG_UNWIND_SELECTION
	// This is to make the skipped moves if any printable, as well as easily identifiable in debug logs
	for(; (jCur < iMCnt) && (bShortCircuitSearch == 1); jCur+=1) {
			cb_cmov2smov(movs[jCur],movs[jCur]);
			movsEval[jCur] = 111222333;
	}

	dbg_log(fLog,"DEBUG:findbest:%c:curDepth[%d] Pos[%s] unwind *** SelectFROM ***\n",cbC->sideToMove,curDepth,cbC->sMoves);
	for(iCur = 0; iCur < iMCnt; iCur++) {
		dbg_log(fLog,"DEBUG:%d:Move[%s],eval[%d],NBMoves[%s]\n",iCur,movs[iCur],movsEval[iCur],movsNBMoves[iCur]);
	}
#endif
	if(cbC->sideToMove == STM_WHITE) {
		if(iMaxPosInd == -1) {
			iMaxVal = VALPW_WHITESTUCK_GOODFORBLACK;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"WHITE_NOP");
#ifdef DEBUG_UNWINDLOWLVLSUMMARYPRINT
			dbg_log(fLog,"DEBUG:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] NO VALID MOVE eval[%d]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,iMaxVal);
#endif
		} else {
#ifdef DEBUG_UNWINDLOWLVLSUMMARYPRINT
			dbg_log(fLog,"INFO:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] mov[%s] eval[%d] NBMoves[%s]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,movs[iMaxPosInd],movsEval[iMaxPosInd],sMaxPosNBMoves);
#endif
			iMaxVal = iMaxPosVal;
			iMaxInd = iMaxPosInd;
#ifdef MOVELIST_ADDMOVENUM_NBMOVES
			sprintf(sNextBestMoves,"%d. %s %s", movNum, cb_2longnot(movs[iMaxInd],s2LN), sMaxPosNBMoves);
#else
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd],s2LN), sMaxPosNBMoves);
#endif
		}
	} else {
		if(iMaxNegInd == -1) {
			iMaxVal = VALPW_BLACKSTUCK_GOODFORWHITE;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"BLACK_NOP");
#ifdef DEBUG_UNWINDLOWLVLSUMMARYPRINT
			dbg_log(fLog,"DEBUG:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] NO VALID MOVE eval[%d]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,iMaxVal);
#endif
		} else {
#ifdef DEBUG_UNWINDLOWLVLSUMMARYPRINT
			dbg_log(fLog,"INFO:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] mov[%s] eval[%d] NBMoves[%s]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,movs[iMaxNegInd],movsEval[iMaxNegInd],sMaxNegNBMoves);
#endif
			iMaxVal = iMaxNegVal;
			iMaxInd = iMaxNegInd;
#ifdef MOVELIST_ADDMOVENUM_NBMOVES
			sprintf(sNextBestMoves,"%d... %s %s", movNum, cb_2longnot(movs[iMaxInd],s2LN), sMaxNegNBMoves);
#else
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd],s2LN), sMaxNegNBMoves);
#endif
		}
	}
	
#ifdef USE_MOVELISTEVALAGING
	// PURPOSEFULLY degrade the value by a small value of 1, so that 
	// if there are two moves at a given position with same value ideally (in the long run) at a given level, 
	// we should select the one which achieves the desired result faster i.e with less moves.
	// So we purposefully degrade the value by a small value/amount of 1.
	if(iMaxVal > 0) {
		iMaxVal = iMaxVal-1;
	} else {
		iMaxVal = iMaxVal+1;
	}
#endif

	//iMaxVal += valPWStatic;

#ifdef CORRECTVALFOR_SIDETOMOVE 
	// DONE:TOTHINK:TOCHECK: Orig sideToMove or current sideToMove, assuming UCI expects current
	// Rather UCI expects the eval score to be wrt the original sideToMove, when it gave the go command
	val = cb_valpw2valpstm(cbC->origSideToMove,iMaxVal);
#else
	val = iMaxVal;
#endif
	lDTime = diff_clocktime(&gtsStart);
#ifdef DO_SENDPERIODICINFO
	if((lDTime-gPrevDTime) >= 30000) {
		send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info depth %d nodes %lld\n", maxDepth-curDepth+1,gMovesCnt);
		gPrevDTime = lDTime;
	}
#endif
	//if(curDepth == 1) // TODO:TOTHINK: may avoid this check (Avoided, but still think thro when additional logic added)
	{
		if(iMaxInd == -1) {
			if(curDepth <= 2) {
			send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %lld time %ld pv %s string %s - %s\n",
				val,maxDepth-curDepth+1,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN),sNextBestMoves,"NO VALID MOVE");
			} else {
#ifdef DEBUG_UNWINDSUMMARYPRINT
			dbg_log(fLog,"INFO:info score cp %d depth %d nodes %lld time %ld pv %s string %s - %s\n",
				val,maxDepth-curDepth+1,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN),sNextBestMoves,"NO VALID MOVE");
#endif
			}
		}
		else {
			// Nodes simply mapped to total Moves generated for now, which may be correct or wrong, to check
			if(curDepth == 1) {
				dbgs_log(fLogM,"%d:SENT:%c:evalPW[%d],Depth[%d],Nodes[%lld],Time[%d],Moves[%s]\n",
						myPID,cbC->sideToMove,iMaxVal,maxDepth-curDepth+1,gMovesCnt,lDTime,sNextBestMoves);
#ifdef DEBUG_HTSUMMARYPRINT
				dbgs_log(fLogM,"%d:HTINFO:%c:CLASH[%d],HIT[%d],VALMISMATCH[%d],VALMATCH[%d],BETTEREVALD[%d],CANBETTEREVAL[%d],TableSize[%d]\n", 
						myPID, cbC->sideToMove,
						gHashTable->HashClashCnt,gHashTable->HashHitCnt,gHashTable->ValMismatchCnt,gHashTable->ValMatchCnt,
						gHashTable->BetterEvaldCnt,gHashTable->CanBetterEvalCnt,gHashTable->WCnt+gHashTable->BCnt);
#endif
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %lld time %ld multipv 1 pv %s\n",
					val,maxDepth-curDepth+1,gMovesCnt,lDTime,sNextBestMoves); //FIXED: Changed to maxDepth, using generic formula
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"bestmove %s\n",cb_2longnot(movs[iMaxInd],s2LN));
			} else if(curDepth == 2){
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %lld time %ld pv %s %s\n",
					val,maxDepth-curDepth+1,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN),sNextBestMoves);
			} else { 
				//FIXED: Changed to Amount of depth unwinded
#ifdef DEBUG_UNWINDSUMMARYPRINT
				dbg_log(fLog,"INFO:info score cp %d depth %d nodes %lld time %ld pv %s %s\n",
					val,maxDepth-curDepth+1,gMovesCnt,lDTime,cb_2longnot(cbC->sMoves,s2LN),sNextBestMoves);
#endif
			}
		}
	}
#ifdef USE_HASHTABLE
	//if(curDepth <= 5) 
	{
	phash_find(gHashTable,cbC,iMaxVal,sNextBestMoves,curDepth,HTFIND_ADD);
	}
#endif
	return iMaxVal; 
}

int cb_qfindbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves, int hint, int bestW, int bestB)
{
	int valPWStatic;
	char movs[NUMOFPARALLELMOVES][32];
	int movsEval[NUMOFPARALLELMOVES];
	int iMCnt, iCur, jCur;
	int iMaxPosVal,iMaxPosInd,iMaxNegVal,iMaxNegInd;
	int iMaxVal, iMaxInd;
	char sMaxPosNBMoves[MOVES_BUFSIZE],sMaxNegNBMoves[MOVES_BUFSIZE];
	char s2LN[MOVES_BUFSIZE];
	long lDTime = 0;
	char movsNBMoves[NUMOFPARALLELMOVES][MOVES_BUFSIZE];
	struct moveprocessHlpr mpH[NUMOFTHREADS];
	int xCur;
	int bShortCircuitSearch = 0;
#ifdef USE_ABPRUNING
	char sABPNextBestMoves[MOVES_BUFSIZE];
	int iABPEval = 0;
#endif

	valPWStatic = cb_evalpw(cbC);

	lDTime = diff_clocktime(&gtsStart);
	if((cbC->wk_killed != 0) || (cbC->bk_killed != 0)) {
		return valPWStatic;
	}

	if (curDepth > maxDepth) {
		return valPWStatic;
	}

	curDepth += 1;
	iMaxPosVal = 0; iMaxNegVal = 0; iMaxPosInd = -1; iMaxNegInd = -1;
	iMCnt = moves_get(cbC,movs,0);

	bShortCircuitSearch = 0;
	for(jCur = 0; (jCur < iMCnt) && (bShortCircuitSearch == 0); jCur+=NUMOFTHREADS) {
		for(iCur = jCur; iCur < (jCur+NUMOFTHREADS); iCur++) {
			if(iCur >= iMCnt) {
				continue;
			}
			xCur=iCur-jCur;
			strcpy(movsNBMoves[iCur],"");
			mpH[xCur].cbC = cbC;
			mpH[xCur].sMov = movs[iCur];
			mpH[xCur].curDepth = curDepth;
			mpH[xCur].maxDepth = maxDepth;
			mpH[xCur].secs = secs;
			mpH[xCur].movNum = movNum;
			mpH[xCur].altMovNum = iCur;
			mpH[xCur].sNextBestMoves = movsNBMoves[iCur];
			mpH[xCur].hint = FBHINT_QUICK;
			mpH[xCur].bestW = bestW;
			mpH[xCur].bestB = bestB;
			{
				moveprocess_hlpr(&mpH[xCur]);
				movsEval[iCur] = mpH[xCur].iRes;
			}
		}

		for(iCur = jCur; iCur < (jCur+NUMOFTHREADS); iCur++) {
			if(iCur >= iMCnt) {
				continue;
			}
			if(movsEval[iCur] == DO_ERROR) {
				continue;
			}
			if(iMaxPosInd == -1) {
				iMaxPosInd = iCur;
				iMaxPosVal = movsEval[iCur];
				iMaxNegInd = iCur;
				iMaxNegVal = movsEval[iCur];
				strcpy(sMaxPosNBMoves,movsNBMoves[iCur]);
				strcpy(sMaxNegNBMoves,movsNBMoves[iCur]);
			}
#ifdef DO_FINDBEST_ONLYIFBETTER
			if(movsEval[iCur] > iMaxPosVal) {
#else
			if(movsEval[iCur] >= iMaxPosVal) {
#endif
				iMaxPosVal = movsEval[iCur];
				iMaxPosInd = iCur;
				strcpy(sMaxPosNBMoves,movsNBMoves[iCur]);
			}
#ifdef DO_FINDBEST_ONLYIFBETTER
			if(movsEval[iCur] < iMaxNegVal) {
#else
			if(movsEval[iCur] <= iMaxNegVal) {
#endif
				iMaxNegVal = movsEval[iCur];
				iMaxNegInd = iCur;
				strcpy(sMaxNegNBMoves,movsNBMoves[iCur]);
			}

#ifdef USE_ABPRUNING
			if(cbC->sideToMove == STM_WHITE) {
				if(movsEval[iCur] > bestW) {
					bestW = movsEval[iCur];
				}
#ifdef DO_ABPRUN_ONLYIFBETTER
				if(movsEval[iCur] > bestB) {
#else
				if(movsEval[iCur] >= bestB) {
#endif
					if(!bShortCircuitSearch) {
					bShortCircuitSearch = 1;
					sprintf(sABPNextBestMoves,"%s %s", cb_2longnot(movs[iCur],s2LN), movsNBMoves[iCur]);
					iABPEval = movsEval[iCur];
					}
				}
			} else {
				if(movsEval[iCur] < bestB) {
					bestB = movsEval[iCur];
				}
#ifdef DO_ABPRUN_ONLYIFBETTER
				if(movsEval[iCur] < bestW) {
#else
				if(movsEval[iCur] <= bestW) {
#endif
					if(!bShortCircuitSearch) {
					bShortCircuitSearch = 1;
					sprintf(sABPNextBestMoves,"%s %s", cb_2longnot(movs[iCur],s2LN), movsNBMoves[iCur]);
					iABPEval = movsEval[iCur];
					}
				}
			}
#endif
		}
	}

#ifdef USE_ABPRUNING
	if(bShortCircuitSearch) {
		strncpy(sNextBestMoves,sABPNextBestMoves,MOVES_BUFSIZE);
		return iABPEval;
	}
#endif

	if(cbC->sideToMove == STM_WHITE) {
		if(iMaxPosInd == -1) {
			//iMaxVal = VALPW_WHITESTUCK_GOODFORBLACK;
			iMaxVal = bestW;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"WHITE_NOP");
		} else {
			iMaxVal = iMaxPosVal;
			iMaxInd = iMaxPosInd;
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd],s2LN), sMaxPosNBMoves);
		}
	} else {
		if(iMaxNegInd == -1) {
			//iMaxVal = VALPW_BLACKSTUCK_GOODFORWHITE;
			iMaxVal = bestB;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"BLACK_NOP");
		} else {
			iMaxVal = iMaxNegVal;
			iMaxInd = iMaxNegInd;
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd],s2LN), sMaxNegNBMoves);
		}
	}
	
	return iMaxVal;
}


int process_go(char *sCmd)
{
	char sNextBestMoves[MOVES_BUFSIZE];

#ifdef USE_HASHTABLE
	if((gHashTable = malloc(sizeof(struct phashtable))) == NULL) {
		fprintf(fLog,"INFO:process_go:Allocating HashTable of size[%ld] FAILED\n",sizeof(struct phashtable));
		exit(-1);
	}
	phash_init(gHashTable);
#endif
	
	if(clock_gettime(CLOCK_REALTIME_COARSE,&gtsStart) != 0) {
		dbg_log(fLog,"FIXME:process_go:clock_gettime failed\n");
		exit(50);
	}
	bzero(sNextBestMoves,MOVES_BUFSIZE);
	gMovesCnt = 0;
	cb_findbest(&gb,0,gGameDepth,0,gStartMoveNum,sNextBestMoves, FBHINT_NORMAL,MAXBLACKEVAL,MAXWHITEEVAL);
#ifdef USE_HASHTABLE
	free(gHashTable);
#endif
	fflush(fLogM);
	fflush(fLog);
	return 0;
}

#include "setoptioncmd.c"
#include "positioncmd.c"
#include "debugcmd.c"

int process_uci()
{

	char sCmdBuf[UCICMDBUFSIZE];
	char *sCmd;
	char sTemp[S1KTEMPBUFSIZE];
	char sPNBuf[32];

	sCmd = fgets(sCmdBuf, S1KTEMPBUFSIZE, stdin);
	if(sCmd == NULL) {
		dbg_log(fLog,"WARN:process_uci:fgets NULL\n");
		exit(3);
	}
	dbg_log(fLog,"GOT:%s\n",sCmd);
	fflush(fLog);
	dbgs_log(fLogM,"%d:GOT:%s\n",myPID,sCmd);
	fflush(fLogM);

#ifdef USE_HASHTABLE
	strcpy(sPNBuf,"HT");
#else
	strcpy(sPNBuf,"NORM");
#endif
#ifdef USE_THREAD
	strcat(sPNBuf,"MT");
#endif
#ifdef USE_BMPRUNING
	strcat(sPNBuf,"BM");
#endif
#ifdef USE_ABPRUNING
	strcat(sPNBuf,"AB");
#endif
#ifdef USE_ABSHORTKILLED
	strcat(sPNBuf,"SK");
#endif
#ifdef USE_MOVELISTEVALAGING
	strcat(sPNBuf,"EA");
#endif
#ifdef USE_POSEVAL
#ifdef USE_POSKNIGHTEVAL
	strcat(sPNBuf,"EPK");
#else
	strcat(sPNBuf,"EP");
#endif
#endif

	if(strncmp(sCmd,"ucinewgame",10) == 0) {
	} else if(strncmp(sCmd,"uci",3) == 0) {
		send_resp_ex(sTemp,S1KTEMPBUFSIZE,"id name %s-%s\n",PRG_VERSION,sPNBuf);
		send_resp("id author hkvc\n");
		send_resp("option name Ponder type check default true\n");
		send_resp("option name Hash type spin default 1 min 1 max 100\n");
		send_resp("option name depth type spin default 3 min 1 max 100\n");
		send_resp("uciok\n");
	}
	if(strncmp(sCmd,"isready",7) == 0) {
		send_resp("readyok\n");
	}
	if(strncmp(sCmd,"position",8) == 0) {
		if(process_position(&gb,sCmd) != 0)
			send_resp("info string error parsing position");
	}
	if(strncmp(sCmd,"setoption",9) == 0) {
		process_setoption(sCmd);
	}
	if(strncmp(sCmd,"go",2) == 0) {
		process_go(sCmd);
	}
	if(strncmp(sCmd,"debug",5) == 0) {
		process_debug(sCmd);
	}
	if(strncmp(sCmd,"quit",4) == 0) {
		dbg_log(fLog,"QUITING\n");
		exit(2);
	}
	dbg_log(fLog,"DONE:%s\n",sCmd);
	fflush(fLog);
	return 0;
}

int run()
{
	fd_set fdIN;

	setbuf(stdin,NULL);

	FD_ZERO(&fdIN);
	FD_SET(0,&fdIN);
	while(1) {
		if(select(10,&fdIN,NULL,NULL,NULL) == 1)
			process_uci();
	}

	return 0;
}

int prepare()
{
	generate_bb_knightmoves(bbKnightMoves);
	generate_bb_rookmoves(bbRookMoves);
	generate_bb_bishopmoves(bbBishopMoves);
	generate_bb_queenmoves(bbQueenMoves, bbRookMoves, bbBishopMoves);
	generate_bb_kingmoves(bbKingMoves);
	generate_bb_pawnmoves(bbWhitePawnNormalMoves, bbWhitePawnAttackMoves, bbBlackPawnNormalMoves, bbBlackPawnAttackMoves);

	gUCIOption = 0;

	return 0;
}

int main(int argc, char **argv)
{
	char sTemp[S32TEMPBUFSIZE];

	myPID = getpid();
	if((fLog=fopen("/tmp/cek1.log","a+")) == NULL)
		return 1;
	if((fLogM=fopen("/home/hanishkvc/cek1_main.log","a+")) == NULL)
		return 2;
	send_resp_ex(sTemp,S32TEMPBUFSIZE,"%s\n",PRG_VERSION);
	prepare();
	run();
	fclose(fLogM);
	fclose(fLog);
	return 0;
}

