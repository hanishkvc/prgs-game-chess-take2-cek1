
//#define PHASH_MAXCNT (1000*1000)
#define PHASH_MAXCNT (8*1000)

// NOTE: Ideally 6 bits was good enough has there are only 64 possible
// squares in which a piece can be. However as there are 64 bits in
// the hash structure, using 8 bits per piece position instead.

#define PHASH_PPOSMASK 0xFF
#define PHASH_R1SHIFT 0
#define PHASH_R2SHIFT 8
#define PHASH_N1SHIFT 16
#define PHASH_N2SHIFT 24
#define PHASH_B1SHIFT 32
#define PHASH_B2SHIFT 40
#define PHASH_QSHIFT 48
#define PHASH_KSHIFT 56

struct phash {
	u64 wp,wo,bp,bo;
	u64 aa;
	int val;
	char sideToMove;
	int curDepth;
	char sMoves[MOVES_BUFSIZE];
	char sNBMoves[MOVES_BUFSIZE];
};

struct phashtable {
	struct phash phashWArr[PHASH_MAXCNT];
	int phashWNext;
	int phashWCnt;
	struct phash phashBArr[PHASH_MAXCNT];
	int phashBNext;
	int phashBCnt;
	int HashHitCnt;
	int ValMismatchCnt;
	int ValMatchCnt;
	int HashClashCnt;
	int BetterEvaldCnt;
	int CanBetterEvalCnt;
};

struct phashtable *gHashTable;

void phash_init(struct phashtable *phtC)
{
	bzero(phtC,sizeof(struct phashtable));
}

// NOTE: It currently accounts for a standard chess game only
// and inturn no pawn promotions, which can lead to more than
// 2 bishop,knight or rook and more than 1 queen.
// MAYBE a simple workaround could be to add the value of
// wr+wn+wb+wq to aa i.e to handle pawn promotions.
void phash_gen(struct phash *phC, struct cb *cbC)
{
	u64 bbT;
	u64 tPos;

	bzero(phC,sizeof(struct phash));
	phC->wp = cbC->wp;
	phC->bp = cbC->bp;

	bbT = cbC->wr;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_R1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_R2SHIFT;
	
	bbT = cbC->wn;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_N1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_N2SHIFT;
	
	bbT = cbC->wb;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_B1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_B2SHIFT;
	
	bbT = cbC->wq;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_QSHIFT;
	bbT = cbC->wk;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->wo |= tPos<<PHASH_KSHIFT;
	

	bbT = cbC->br;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_R1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_R2SHIFT;
	
	bbT = cbC->bn;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_N1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_N2SHIFT;
	
	bbT = cbC->bb;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_B1SHIFT;
	bbT &= ~(1ULL << (tPos-1));
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_B2SHIFT;
	
	bbT = cbC->bq;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_QSHIFT;
	bbT = cbC->bk;
	tPos = (ffsll(bbT) & PHASH_PPOSMASK);
	phC->bo |= tPos<<PHASH_KSHIFT;
	
	phC->aa = phC->wp + phC->bp + phC->wo + phC->bo;
	phC->sideToMove = cbC->sideToMove;
}

void phash_print(struct phash *phT, char *sMsg)
{
	dbg_log(fLog,"INFO:phash:%s\n",sMsg);
	dbg_log(fLog,"....: aa[%lld] wp[%lld] wo[%lld] bp[%lld] bo[%lld] sideToMove[%c] sMoves[%s] sNBMoves[%s]\n",
				phT->aa, phT->wp, phT->wo, phT->bp, phT->bo, phT->sideToMove, phT->sMoves, phT->sNBMoves);
}

struct phash *phash_find(struct phashtable *phtC, struct cb *cbC, struct phash *phT, int *iPos, int curDepth, int hint)
{
	int i;

	phash_gen(phT,cbC);
	if(phT->sideToMove == STM_WHITE) {

		for(i=0; i<phtC->phashWCnt; i++) {
			if((phtC->phashWArr[i].aa != phT->aa) || (phtC->phashWArr[i].sideToMove != phT->sideToMove))
				continue;
			if((phtC->phashWArr[i].wp == phT->wp) && (phtC->phashWArr[i].wo == phT->wo) &&
				(phtC->phashWArr[i].bp == phT->bp) && (phtC->phashWArr[i].bo == phT->bo)) {
				if((phtC->phashWArr[i].curDepth <= curDepth) || (hint == HTFIND_IGNORECURDEPTH)) {
					phtC->HashHitCnt++;
					//dbg_log(fLog,"INFO:phash_find:W:HTHIT:HTPos[%d]\n",i);
					*iPos = i;
					return &(phtC->phashWArr[i]);
				} else {
					phtC->CanBetterEvalCnt++;
					break;
				}
			} else {
#ifdef DEBUG_HTPRINT
				dbg_log(fLog,"INFO:phash_find:W:HTCLASH:HTPos[%d]:\n",i);
				phash_print(&(phtC->phashWArr[i]),"FromTable");
				phash_print(phT,"NewBeingChecked");
#endif
				phtC->HashClashCnt++;
			}
		}
	
	} else {

		for(i=0; i<phtC->phashBCnt; i++) {
			if((phtC->phashBArr[i].aa != phT->aa) || (phtC->phashBArr[i].sideToMove != phT->sideToMove))
				continue;
			if((phtC->phashBArr[i].wp == phT->wp) && (phtC->phashBArr[i].wo == phT->wo) &&
				(phtC->phashBArr[i].bp == phT->bp) && (phtC->phashBArr[i].bo == phT->bo)) {
				if((phtC->phashBArr[i].curDepth <= curDepth) || (hint == HTFIND_IGNORECURDEPTH)) {
					phtC->HashHitCnt++;
					//dbg_log(fLog,"INFO:phash_find:B:HTHIT:HTPos[%d]\n",i);
					*iPos = i;
					return &(phtC->phashBArr[i]);
				} else {
					phtC->CanBetterEvalCnt++;
					break;
				}
			} else {
#ifdef DEBUG_HTPRINT
				dbg_log(fLog,"INFO:phash_find:B:HTCLASH:HTPos[%d]:\n",i);
				phash_print(&(phtC->phashBArr[i]),"FromTable");
				phash_print(phT,"NewBeingChecked");
#endif
				phtC->HashClashCnt++;
			}
		}
	
	}
	*iPos = -1;
	return NULL;
}

void phash_add(struct phashtable *phtC, struct cb *cbC, int val, char *sMoves, char *sNextBestMoves, int curDepth)
{
	int iPos = 0;
	struct phash phTemp;
	struct phash *phashCArr;
	int *pphashCNext;
	int *pphashCCnt;
	struct phash *phRes;

	phRes = phash_find(phtC,cbC,&phTemp,&iPos,curDepth,HTFIND_IGNORECURDEPTH);
	if(phTemp.sideToMove == STM_WHITE) {
		phashCArr = phtC->phashWArr;
		pphashCNext = &phtC->phashWNext;
		pphashCCnt = &phtC->phashWCnt;
	} else {
		phashCArr = phtC->phashBArr;
		pphashCNext = &phtC->phashBNext;
		pphashCCnt = &phtC->phashBCnt;
	}

	if(phRes == NULL) {
		memcpy(&(phashCArr[*pphashCNext]),&phTemp,sizeof(struct phash));
		phashCArr[*pphashCNext].val = val;
		phashCArr[*pphashCNext].curDepth = curDepth;
		strncpy(phashCArr[*pphashCNext].sMoves,sMoves,MOVES_BUFSIZE);
		strncpy(phashCArr[*pphashCNext].sNBMoves,sNextBestMoves,MOVES_BUFSIZE);
		(*pphashCNext)++;
		if(*pphashCCnt < PHASH_MAXCNT) {
			*pphashCCnt = *pphashCNext;
		}
		if(*pphashCNext >= PHASH_MAXCNT) {
			*pphashCNext = 0;
		}
	} else if(phRes->curDepth > curDepth) { 
		// The new position info is a better eval as it has more moves searched along depth
		// the curDepth here limits to what extent moves will be searched along the depth
		// So a Higher curDepth means less depth searched,
		// while a Lower curDepth means more depth searched, so a better eval result.
#ifdef DEBUG_HTPRINT
		dbg_log(fLog,"INFO:phash_add:HTBETTEREVALD:Position in table (depth=%d), but better eval (depth=%d) has been done\n",
					phRes->curDepth,curDepth);
		dbg_log(fLog,"....:phash_add:HT.sMoves[%s],HT.sNBMoves[%s] AND New.sMoves[%s],New.sNBMoves[%s]\n",
					phRes->sMoves, phRes->sNBMoves, sMoves, sNextBestMoves);
#endif
		phRes->curDepth = curDepth;
		phRes->val = val;
		strncpy(phRes->sMoves,sMoves,MOVES_BUFSIZE);
		strncpy(phRes->sNBMoves,sNextBestMoves,MOVES_BUFSIZE);
		phtC->BetterEvaldCnt++;
	} else if(phRes->curDepth < curDepth) {
		dbgs_log(fLog,"FIXME:BUG: moveprocess phash_find missed this ?! or a corner case in logic/thought.HTCurDepth[%d],NewCurDepth[%d]\n",
					phRes->curDepth,curDepth);
		dbg_log(fLog,"....:phash_add:HT.sMoves[%s],HT.sNBMoves[%s] AND New.sMoves[%s],New.sNBMoves[%s]\n",
					phRes->sMoves, phRes->sNBMoves, sMoves, sNextBestMoves);
		exit(-1);
	} else if(phashCArr[iPos].val != val) {
#ifdef DEBUG_HTPRINT
		dbg_log(fLog,"DEBUG:phash_add:HTVALMISMATCH:TableHitBut?:HT[Pos:%d=Val:%d]NewVal[%d]RetVal[%d], HTCurDepth[%d]=NewCurDepth[%d]\n",
					iPos,phashCArr[iPos].val,val,phRes->val,phRes->curDepth,curDepth);
		dbg_log(fLog,"....:phash_add:HT.sMoves[%s],HT.sNBMoves[%s] AND New.sMoves[%s],New.sNBMoves[%s]\n",
					phRes->sMoves, phRes->sNBMoves, sMoves, sNextBestMoves);
#endif
		phtC->ValMismatchCnt++;
	} else {
		phtC->ValMatchCnt++;
	}
}

