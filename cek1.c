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

#define DEBUG_UNWIND_SELECTION

#include "makeheader.h"
#include "cek1.h"

FILE *fLog;

struct cb gb;
struct timespec gtsStart, gtsDiff;
long gDTime = 0;
int gMovesCnt = 0;
int gStartMoveNum = 0;
int gUCIOption = 0;

// Has this is a multiline macro, always use it inside braces
#define send_resp_ex(sBuffer,sSize,...) snprintf(sBuffer,sSize,__VA_ARGS__); send_resp(sBuffer);
#define dbg_cb_bb_print dummy
#define dbg_log(file,...) fprintf(file,__VA_ARGS__)
//#define dbg_log(file,...) dummy()

void dummy() 
{
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
char gNOTSBUF[32];

char *cb_2longnot(char *sIMov)
{
	int i;
	char *gDest = gNOTSBUF;

	for(i=0;i<strlen(sIMov);i++) {
		if((sIMov[i] != '-') && (sIMov[i] != 'P') && (sIMov[i] != 'x')) {
			*gDest=sIMov[i];
			gDest++;
		}
	}
	*gDest = '\0';
	return gNOTSBUF;
}

char *cb_2simpnot(char *sIMov)
{
	return sIMov;
}

int process_setoption(char *sCmd)
{
	//if(gUCIOption | UCIOPTION_CUSTOM_SHOWCURRMOVE)
	return 0;
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
	iPos = cb_strloc2bbpos(&sMov[4]);
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

void mvhlpr_domove(struct cb *cbC, char mPiece, int mSPos, int mDPos)
{
	
	if(cbC->sideToMove == STM_WHITE) {
		if(mPiece == 'K') {
			cbC->wk &= ~(1ULL << mSPos);
			cbC->wk |= (1ULL << mDPos);
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
			cbC->wp |= (1ULL << mDPos);
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
			cbC->bp |= (1ULL << mDPos);
		}
		cbC->wk &= ~(1ULL << mDPos);
		cbC->wq &= ~(1ULL << mDPos);
		cbC->wr &= ~(1ULL << mDPos);
		cbC->wn &= ~(1ULL << mDPos);
		cbC->wb &= ~(1ULL << mDPos);
		cbC->wp &= ~(1ULL << mDPos);
	}

}

int move_process(struct cb *cbC, char *sMov, int curDepth, int maxDepth, int secs, int movNum, int altMovNum,char *sNextBestMoves)
{
	struct cb cbN;
	int iRes;
	char mPiece;
	int mSPos, mDPos;
	char sBuf[S1KTEMPBUFSIZE];
	char sNBMoves[MOVES_BUFSIZE];

	bzero(sNBMoves,8);

	iRes = move_validate(cbC, sMov);
	if(iRes == DO_ERROR)
		return DO_ERROR;
	memcpy(&cbN,cbC,sizeof(struct cb));

	mPiece = sMov[0];
	mSPos = cb_strloc2bbpos(&sMov[1]);
	mDPos = cb_strloc2bbpos(&sMov[4]);
	
	if(gUCIOption & UCIOPTION_CUSTOM_SHOWCURRMOVE) {
		send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info depth %d currmove %s currmovenumber %d\n", curDepth, cb_2longnot(sMov), altMovNum);
	}
	mvhlpr_domove(&cbN,mPiece,mSPos,mDPos);
	if(cbC->sideToMove == STM_WHITE) {
		sprintf(sBuf,"%d.",movNum);
		strcat(cbN.sMoves,sBuf);
		cbN.sideToMove = STM_BLACK;
	}
	else {
		cbN.sideToMove = STM_WHITE;
		movNum += 1;
	}
	strcat(cbN.sMoves,sMov);
	strcat(cbN.sMoves," ");
	iRes = cb_findbest(&cbN,curDepth,maxDepth,secs,movNum,sNBMoves);
	strcat(sNextBestMoves,sNBMoves); // FIXME: CAN BE REMOVED, CROSSVERIFY
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

int cb_findbest(struct cb *cbC, int curDepth, int maxDepth, int secs, int movNum, char *sNextBestMoves)
{
	int valPW;
	int val;
	char sBuf[S1KTEMPBUFSIZE];
	char movs[NUMOFPARALLELMOVES][32];
	int movsEval[NUMOFPARALLELMOVES];
	int iMCnt, iCur;
	int iMaxPosVal,iMaxPosInd,iMaxNegVal,iMaxNegInd;
	int iMaxVal, iMaxInd;
	char sNBMoves[MOVES_BUFSIZE];
	char sMaxPosNBMoves[MOVES_BUFSIZE],sMaxNegNBMoves[MOVES_BUFSIZE];

#ifdef DEBUG_UNWIND_SELECTION
	char movsNBMovesDBG[NUMOFPARALLELMOVES][2048];
#endif

	valPW = cb_evalpw(cbC);
#ifdef CORRECTVALFOR_SIDETOMOVE
	// FIXME: Should use the original sideToMove (i.e curDepth = 0) info and not the current sideToMove (curDepth > 0)
	// Have to add a variable to struct cb to store the sideToMoveORIG
	val = cb_valpw2valpstm(cbC->sideToMove,valPW); 
#else
	val = valPW;
#endif

	gDTime = diff_clocktime(&gtsStart);
	//*depthReached = curDepth;
	//The check for king underattack has to be thought thro and updated if required.
	if((curDepth == maxDepth) || (cbC->wk_underattack > 1) || (cbC->bk_underattack > 1)) {
		char sTemp[64];
		if((cbC->wk_underattack > 1) || (cbC->bk_underattack > 1)) 
			strcpy(sTemp,"string UnderCheckINVALIDMOVE");
		else
			strcpy(sTemp,"");
		send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %d time %ld pv %s %s\n",
				val,curDepth,gMovesCnt,gDTime,cbC->sMoves,sTemp);
		if((cbC->wk_underattack > 1) || (cbC->bk_underattack > 1))
			return DO_ERROR;
		return valPW;
	}

	// sideToMove is actually nextSideToMove from symantic perspective
	// No point of checking what is the best move for the next side to move,
	// when the previous side is already under check and its move hasn't moved
	// it out of that check. OR the previous sides move has put itself
	// under check, as that is a invalid move which is not legally allowed
	// by chess rules.
	if( ((cbC->sideToMove == STM_BLACK) && (cbC->wk_underattack)) ||
			((cbC->sideToMove == STM_WHITE) && (cbC->bk_underattack)) ) {
		send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %d time %ld pv %s string EntersCheckINVALIDMOVE\n",
				val,curDepth,gMovesCnt,gDTime,cbC->sMoves);
		return DO_ERROR;
	}

	curDepth += 1;
	iMaxPosVal = 0; iMaxNegVal = 0; iMaxPosInd = -1; iMaxNegInd = -1;
	iMCnt = moves_get(cbC,movs,0);
	for(iCur = 0; iCur < iMCnt; iCur++) {
		//send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info currmove %s currmovenumber %d\n", movs[iCur], iCur);
		strcpy(sNBMoves,"");
		movsEval[iCur] = move_process(cbC, movs[iCur], curDepth, maxDepth, secs, movNum, iCur,sNBMoves); 
#ifdef DEBUG_UNWIND_SELECTION
		strcpy(movsNBMovesDBG[iCur],sNBMoves);
#endif
		if(movsEval[iCur] == DO_ERROR)
			continue;
		if(iMaxPosInd == -1) {
			iMaxPosInd = iCur;
			iMaxPosVal = movsEval[iCur];
			iMaxNegInd = iCur;
			iMaxNegVal = movsEval[iCur];
			strcpy(sMaxPosNBMoves,sNBMoves);
			strcpy(sMaxNegNBMoves,sNBMoves);
		}
		if(movsEval[iCur] > iMaxPosVal) {
			iMaxPosVal = movsEval[iCur];
			iMaxPosInd = iCur;
			strcpy(sMaxPosNBMoves,sNBMoves);
		}
		if(movsEval[iCur] < iMaxNegVal) {
			iMaxNegVal = movsEval[iCur];
			iMaxNegInd = iCur;
			strcpy(sMaxNegNBMoves,sNBMoves);
		}
	}

	// FIXME:TODO:LATER:Maybe (to think) Send best value from the moves above or best N in multipv case

#ifdef DEBUG_UNWIND_SELECTION
	dbg_log(fLog,"DEBUG:findbest:curDepth[%d] unwind *** SelectFROM ***\n",curDepth);
	for(iCur = 0; iCur < iMCnt; iCur++) {
		dbg_log(fLog,"DEBUG:%d:Move[%s],eval[%d],NBMoves[%s]\n",iCur,movs[iCur],movsEval[iCur],movsNBMovesDBG[iCur]);
	}
#endif
	if(cbC->sideToMove == STM_WHITE) {
		if(iMaxPosInd == -1) {
			dbg_log(fLog,"DEBUG:findbest:curDepth[%d] sideToMove[%c] NO VALID MOVE\n",curDepth,cbC->sideToMove);
			iMaxVal = VALPW_WHITESTUCK_GOODFORBLACK;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"WHITE_NOP");
		} else {
			dbg_log(fLog,"INFO:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] mov[%s] eval[%d] NBMoves[%s]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,movs[iMaxPosInd],movsEval[iMaxPosInd],sMaxPosNBMoves);
			iMaxVal = iMaxPosVal;
			iMaxInd = iMaxPosInd;
#ifdef MOVELIST_ADDMOVENUM
			sprintf(sNextBestMoves,"%d. %s %s", movNum, cb_2longnot(movs[iMaxInd]), sMaxPosNBMoves);
#else
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd]), sMaxPosNBMoves);
#endif
		}
	} else {
		if(iMaxNegInd == -1) {
			dbg_log(fLog,"DEBUG:findbest:curDepth[%d] sideToMove[%c] NO VALID MOVE\n",curDepth,cbC->sideToMove);
			iMaxVal = VALPW_BLACKSTUCK_GOODFORWHITE;
			iMaxInd = -1;
			strcpy(sNextBestMoves,"BLACK_NOP");
		} else {
			dbg_log(fLog,"INFO:findbest:curDepth[%d] sideToMove[%c] PMoves[%s] mov[%s] eval[%d] NBMoves[%s]\n",
						curDepth,cbC->sideToMove,cbC->sMoves,movs[iMaxNegInd],movsEval[iMaxNegInd],sMaxNegNBMoves);
			iMaxVal = iMaxNegVal;
			iMaxInd = iMaxNegInd;
#ifdef MOVELIST_ADDMOVENUM
			sprintf(sNextBestMoves,"%d... %s %s", movNum, cb_2longnot(movs[iMaxInd]), sMaxNegNBMoves);
#else
			sprintf(sNextBestMoves,"%s %s", cb_2longnot(movs[iMaxInd]), sMaxNegNBMoves);
#endif
		}
	}
#ifdef CORRECTVALFOR_SIDETOMOVE
	val = cb_valpw2valpstm(cbC->sideToMove,iMaxVal); // FIXME: Orig sideToMove
#else
	val = iMaxVal;
#endif
	gDTime = diff_clocktime(&gtsStart);
	//if(curDepth == 1) // FIXME: Have to think, may avoid this check
	{
		if(iMaxInd == -1) {
			send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %d time %ld spv %s pv %s %s\n",
				val,curDepth,0,gDTime,sNextBestMoves,cbC->sMoves,"NO VALID MOVE");
		}
		else {
			// Dummy time sent
			// Nodes simply mapped to total Moves generated for now, which may be correct or wrong, to check
			if(curDepth == 1) {
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info score cp %d depth %d nodes %d time %ld multipv 1 pv %s\n",
					val,maxDepth-curDepth+1,gMovesCnt,gDTime,sNextBestMoves); //FIXME: Change to maxDepth
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"bestmove %s\n",cb_2longnot(movs[iMaxInd]));
			} else {
				send_resp_ex(sBuf,S1KTEMPBUFSIZE,"info string unwinddepth cp %d depth %d nodes %d time %ld pv %s\n",
					val,curDepth,gMovesCnt,gDTime,sNextBestMoves);
			}
		}
	}
	return iMaxVal; 
}

// TODO: Have to extract proper move number from position fen process logic below
// and pass this proper move number to cb_findbest

int process_go(char *sCmd)
{
	char sNextBestMoves[MOVES_BUFSIZE];

	if(clock_gettime(CLOCK_REALTIME_COARSE,&gtsStart) != 0) {
		dbg_log(fLog,"FIXME:process_go:clock_gettime failed\n");
		exit(50);
	}
	bzero(sNextBestMoves,MOVES_BUFSIZE);
	gMovesCnt = 0;
	cb_findbest(&gb,0,3,0,gStartMoveNum,sNextBestMoves);
	return 0;
}

int process_position(char *sCmd)
{
	char *fenStr;
	char *fenSTM;
	int r,f;
	char *fenMisc;
	char *fenMoveNum;

	if(strncmp(strtok(sCmd," "),"position",8) != 0)
		return -1;
	if(strncmp(strtok(NULL," "),"fen",3) != 0)
		return -2;
	fenStr = strtok(NULL," ");
	if(fenStr == NULL)
		return -3;
	if((fenSTM = strtok(NULL," ")) == NULL)
		return -4;
	if((fenMisc = strtok(NULL,"-")) == NULL)
		return -5;
	if((fenMisc = strtok(NULL," ")) == NULL)
		return -6;
	if((fenMoveNum = strtok(NULL," ")) == NULL)
		return -7;

	gStartMoveNum = strtol(fenMoveNum,NULL,10);

	bzero(&gb,sizeof(gb));

	if((fenSTM[0] == 'w') || (fenSTM[0] == 'W'))
		gb.sideToMove=STM_WHITE;
	else
		gb.sideToMove=STM_BLACK;


	r = 7; f = 0;
	while(*fenStr != '\0') {
		dbg_log(fLog,"INFO:pp:val[%c]\n",*fenStr);
		if(fenStr[0] == 'p') {
			cb_bb_setpos(&(gb.bp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'P') {
			cb_bb_setpos(&(gb.wp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'r') {
			cb_bb_setpos(&(gb.br),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'R') {
			cb_bb_setpos(&(gb.wr),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'n') {
			cb_bb_setpos(&(gb.bn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'N') {
			cb_bb_setpos(&(gb.wn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'b') {
			cb_bb_setpos(&(gb.bb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'B') {
			cb_bb_setpos(&(gb.wb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'k') {
			cb_bb_setpos(&(gb.bk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'K') {
			cb_bb_setpos(&(gb.wk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'q') {
			cb_bb_setpos(&(gb.bq),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'Q') {
			cb_bb_setpos(&(gb.wq),r,f);
			f += 1; 
		}
		if(fenStr[0] == '/') {
			dbg_log(fLog,"INFO:pp:Next Row\n");
			r -= 1;
			f = 0;
		}
		if(fenStr[0] == '1') {
			f += 1; 
		}
		if(fenStr[0] == '2') {
			f += 2; 
		}
		if(fenStr[0] == '3') {
			f += 3; 
		}
		if(fenStr[0] == '4') {
			f += 4; 
		}
		if(fenStr[0] == '5') {
			f += 5; 
		}
		if(fenStr[0] == '6') {
			f += 6; 
		}
		if(fenStr[0] == '7') {
			f += 7; 
		}
		if(fenStr[0] == ' ') {
			dbg_log(fLog,"DEBUG:pp: strtok strange\n");
			break;
		}
		if((f < 0) || (f > 8)) {	// f checked against 8 instead of 7, as f increment can occur to 8
			dbg_log(fLog,"WARN:pp: file went beyond a-h\n");
		}
		if((r < 0) || (r > 7)) {
			dbg_log(fLog,"WARN:pp: rank went beyond 1-8\n");
		}
		fenStr++;
	}
	cb_print(&gb);
	return 0;
}

int process_uci()
{

	char sCmdBuf[S1KTEMPBUFSIZE];
	char *sCmd;
	char sTemp[S1KTEMPBUFSIZE];

	sCmd = fgets(sCmdBuf, S1KTEMPBUFSIZE, stdin);
	dbg_log(fLog,"GOT:%s\n",sCmd);
	fflush(fLog);

	if(strncmp(sCmd,"uci",3) == 0) {
		send_resp_ex(sTemp,S1KTEMPBUFSIZE,"id name %s\n",PRG_VERSION);
		send_resp("id author hkvc\n");
		send_resp("option name Ponder type check default true\n");
		send_resp("option name Hash type spin default 1 min 1 max 100\n");
		send_resp("uciok\n");
	}
	if(strncmp(sCmd,"isready",7) == 0) {
		send_resp("readyok\n");
	}
	if(strncmp(sCmd,"position",8) == 0) {
		if(process_position(sCmd) != 0)
			send_resp("info string error parsing fen");
	}
	if(strncmp(sCmd,"setoption",9) == 0) {
		process_setoption(sCmd);
	}
	if(strncmp(sCmd,"go",2) == 0) {
		process_go(sCmd);
	}
	if(strncmp(sCmd,"quit",4) == 0) {
		dbg_log(fLog,"QUITING\n");
		exit(2);
	}
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

	if((fLog=fopen("/tmp/cek1.log","a+")) == NULL)
		return 1;
	send_resp_ex(sTemp,S32TEMPBUFSIZE,"%s\n",PRG_VERSION);
	prepare();
	run();
	fclose(fLog);
	return 0;
}

