
#include <search.h>

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
	void *phashWTr;
	void *phashBTr;
	int WCnt;
	int BCnt;
	int phashCnt;
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

void phash_cleanup(struct phashtable *phtC)
{
	tdestroy(phtC->phashWTr,free);
	tdestroy(phtC->phashBTr,free);
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
	
	phC->aa = phC->wp + 2*phC->bp + phC->wo + phC->bo;
	phC->sideToMove = cbC->sideToMove;
}

void phash_print(struct phash *phT, char *sMsg)
{
	dbg_log(fLog,"INFO:phash:%s\n",sMsg);
	dbg_log(fLog,"....: aa[%lld] wp[%lld] wo[%lld] bp[%lld] bo[%lld] sideToMove[%c] sMoves[%s] sNBMoves[%s]\n",
				phT->aa, phT->wp, phT->wo, phT->bp, phT->bo, phT->sideToMove, phT->sMoves, phT->sNBMoves);
}

int compare(const void *pa, const void *pb)
{
	if(((struct phash *)pa)->aa < ((struct phash *)pb)->aa)
		return -1;
	else if(((struct phash *)pa)->aa > ((struct phash *)pb)->aa)
		return 1;
	return 0;
}

struct phash *phash_find(struct phashtable *phtC, struct cb *cbC, int val, char *sNextBestMoves, int curDepth, int hint)
{
	struct phash *phT;
	struct phash *phRes, **phResP;
	int *pCnt;

	phT = malloc(sizeof(struct phash));
	if(phT == NULL) {
		exit(-10);
	}
	phash_gen(phT,cbC);

		if(hint == HTFIND_FIND) {
			if(cbC->sideToMove == STM_WHITE)
				phResP = tfind((void*)phT, &phtC->phashWTr, compare);
			else
				phResP = tfind((void*)phT, &phtC->phashBTr, compare);
			if(phResP == NULL) {
				free(phT);
				return NULL;
			}
			phRes = *phResP;
			if((phRes->wp == phT->wp) && (phRes->wo == phT->wo) &&
				(phRes->bp == phT->bp) && (phRes->bo == phT->bo)) {
				if(phRes->curDepth <= curDepth) {
					phtC->HashHitCnt++;
					free(phT);
					return phRes;
				} else {
					phtC->CanBetterEvalCnt++;
					free(phT);
					return NULL;
				}
			} else {
#ifdef DEBUG_HTPRINT
				phT->curDepth = curDepth;
				phT->val = val;
				strncpy(phT->sMoves,cbC->sMoves,MOVES_BUFSIZE);
				strncpy(phT->sNBMoves,sNextBestMoves,MOVES_BUFSIZE);

				dbg_log(fLog,"INFO:phash_find:%c:HTCLASH:\n",cbC->sideToMove);
				phash_print(phRes,"FromTable");
				phash_print(phT,"NewBeingChecked");
#endif
				phtC->HashClashCnt++;
				free(phT);
				return NULL;
			}
		} else {	// HTFIND_ADD
			phT->curDepth = curDepth;
			phT->val = val;
			strncpy(phT->sMoves,cbC->sMoves,MOVES_BUFSIZE);
			strncpy(phT->sNBMoves,sNextBestMoves,MOVES_BUFSIZE);
			if(cbC->sideToMove == STM_WHITE) {
				pCnt = &(phtC->WCnt);
				phResP = tsearch((void*)phT, &phtC->phashWTr, compare);
			} else {
				pCnt = &(phtC->BCnt);
				phResP = tsearch((void*)phT, &phtC->phashBTr, compare);
			}
			if(phResP == NULL) {	// Memory error
				free(phT);
				return NULL;
			}
			phRes = *phResP;
			if(phRes == phT) {	// New item added, nothing else to do.
				*pCnt = *pCnt+1;
				return phRes;
			}
			// Another item which matches base aa found. Update if required.
			if((phRes->wp == phT->wp) && (phRes->wo == phT->wo) &&
				(phRes->bp == phT->bp) && (phRes->bo == phT->bo)) {
				if(phRes->curDepth <= curDepth) {
					free(phT);
					return phRes;
				} else {	// Previous version old, update to new
					phtC->BetterEvaldCnt++;
					phRes->curDepth = curDepth;
					phRes->val = val;
					strncpy(phRes->sMoves,cbC->sMoves,MOVES_BUFSIZE);
					strncpy(phRes->sNBMoves,sNextBestMoves,MOVES_BUFSIZE);
					free(phT);
					return phRes;
				}
			} else {
#ifdef DEBUG_HTPRINT
				dbg_log(fLog,"INFO:phash_find:%c:HTCLASH:\n",cbC->sideToMove);
				phash_print(phRes,"FromTable");
				phash_print(phT,"NewBeingChecked");
#endif
				phtC->HashClashCnt++;
				free(phT);
				return NULL;
			}
		}
}

void phash_add(struct phashtable *phtC, struct cb *cbC, int val, char *sMoves, char *sNextBestMoves, int curDepth)
{
/*
#ifdef DEBUG_HTPRINT
		dbg_log(fLog,"INFO:phash_add:HTBETTEREVALD:Position in table (depth=%d), but better eval (depth=%d) has been done\n",
					phRes->curDepth,curDepth);
		dbg_log(fLog,"....:phash_add:HT.sMoves[%s],HT.sNBMoves[%s] AND New.sMoves[%s],New.sNBMoves[%s]\n",
					phRes->sMoves, phRes->sNBMoves, sMoves, sNextBestMoves);
#endif
*/
}

