
int moves_forpawnattacks(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;
	u64 bbEOcc = 0;
	u64 bbTOcc = 0;

	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wp;
		bbEOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}
	else {
		bbS = cbC->bp;
		bbEOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		if(cbC->sideToMove == STM_WHITE) {
			bbD = bbWhitePawnAttackMoves[posS];
			bbD = bbD & bbEOcc;
		} else {
			bbD = bbBlackPawnAttackMoves[posS];
			bbD = bbD & bbEOcc;
		}
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			strncpy(movs[iCur],"P",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"x");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));
			iCur += 1;
			bbD &= ~(1ULL << posD);			
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forpawnnormal(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;
	u64 bbWOcc = 0;
	u64 bbBOcc = 0;
	u64 bbOcc = 0;

	bbBOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	bbWOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	bbOcc = bbBOcc | bbWOcc;
	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wp;
	}
	else {
		bbS = cbC->bp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		if(cbC->sideToMove == STM_WHITE) {
			bbD = bbWhitePawnNormalMoves[posS];
		} else {
			bbD = bbBlackPawnNormalMoves[posS];
		}
		bbD = bbD & ~bbOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			strncpy(movs[iCur],"P",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));
			iCur += 1;
			bbD &= ~(1ULL << posD);			
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forknight(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;

	if(cbC->sideToMove == STM_WHITE)
		bbS = cbC->wn;
	else
		bbS = cbC->bn;

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbKnightMoves[posS];
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			strncpy(movs[iCur],"N",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));
			iCur += 1;
			bbD &= ~(1ULL << posD);			
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forking(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;

	if(cbC->sideToMove == STM_WHITE)
		bbS = cbC->wk;
	else
		bbS = cbC->bk;

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbKingMoves[posS];
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			strncpy(movs[iCur],"K",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));
			//check_iskingattacked(posD);
			iCur += 1;
			bbD &= ~(1ULL << posD);
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_get(struct cb *cbC, char movs[512][32], int iCur)
{
	int iNew;

	iNew = moves_forknight(cbC, movs, iCur);
	iNew = moves_forpawnattacks(cbC, movs, iNew);
	iNew = moves_forpawnnormal(cbC, movs, iNew);
	iNew = moves_forking(cbC, movs, iNew);
	if(iNew >=512)
		exit(200);
	gMovesCnt += (iNew-iCur);
	return iNew;
}

