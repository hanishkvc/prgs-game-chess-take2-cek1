
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

int moves_get(struct cb *cbC, char movs[512][32], int iCur)
{
	int iNew;

	iNew = moves_forknight(cbC, movs, iCur);
	return iNew;
}
