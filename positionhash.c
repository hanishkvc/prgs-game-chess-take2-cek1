
#define PHASH_MAXCNT (1*1000*1000)

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
	char sMoves[MOVES_BUFSIZE];
	char sNBMoves[MOVES_BUFSIZE];
};

struct phashtable {
	struct phash phashArr[PHASH_MAXCNT];
	int phashNext;
	int phashCnt;
	int HashHitCnt;
	int ValMismatchCnt;
	int ValMatchCnt;
	int HashClashCnt;
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

int phash_find(struct phashtable *phtC, struct cb *cbC, struct phash *phT)
{
	int i;

	phash_gen(phT,cbC);
	for(i=0; i<phtC->phashCnt; i++) {
		if((phtC->phashArr[i].aa != phT->aa) || (phtC->phashArr[i].sideToMove != phT->sideToMove))
			continue;
		//dbg_log(fLog,"INFO:phash_find:Found\n");
		if((phtC->phashArr[i].wp == phT->wp) && (phtC->phashArr[i].wo == phT->wo) &&
			(phtC->phashArr[i].bp == phT->bp) && (phtC->phashArr[i].bo == phT->bo)) {
			phtC->HashHitCnt++;
			return i;
		} else {
			dbg_log(fLog,"INFO:phash_find:HTCLASH:HTPos[%d]:\n",i);
			phash_print(&(phtC->phashArr[i]),"FromTable");
			phash_print(phT,"NewBeingChecked");
			phtC->HashClashCnt++;
		}
	}
	return -1;
}

void phash_add(struct phashtable *phtC, struct cb *cbC, int val, char *sMoves, char *sNextBestMoves)
{
	int iPos = 0;
	struct phash phTemp;

	iPos = phash_find(phtC,cbC,&phTemp);
	if(iPos == -1) {
		memcpy(&(phtC->phashArr[phtC->phashNext]),&phTemp,sizeof(struct phash));
		phtC->phashArr[phtC->phashNext].val = val;
		strncpy(phtC->phashArr[phtC->phashNext].sMoves,sMoves,MOVES_BUFSIZE);
		strncpy(phtC->phashArr[phtC->phashNext].sNBMoves,sNextBestMoves,MOVES_BUFSIZE);
		phtC->phashNext++;
		if(phtC->phashCnt < PHASH_MAXCNT) {
			phtC->phashCnt = phtC->phashNext;
		}
		if(phtC->phashNext >= PHASH_MAXCNT) {
			phtC->phashNext = 0;
		}
	} else if(phtC->phashArr[iPos].val != val) {
		dbg_log(fLog,"DEBUG:phash_add:HTVALMISMATCH:Position in table, but Value doesn't match\n");
		phtC->ValMismatchCnt++;
	} else {
		phtC->ValMatchCnt++;
	}
}

