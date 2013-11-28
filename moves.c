
int moves_forpawnattacks(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;
	u64 bbEOcc = 0;

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
			bbD &= ~(1ULL << posD);			

			strncpy(movs[iCur],"P",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));

			if(abs(posS-posD) > 8) {
				if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
					dbg_log(fLog,"INFO:moves_forpawnnormal: DROPPING mov[%s] as others inbetween\n",movs[iCur]);
					strncpy(movs[iCur],"",32);
					continue;
				}
			}

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forbishop(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;
	u64 bbFOcc = 0;

	cb_print(cbC);

	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wb;
		bbFOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	} else {
		bbS = cbC->bb;
		bbFOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbBishopMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			bbD &= ~(1ULL << posD);			

			strncpy(movs[iCur],"B",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));

			if(evalhlpr_diagattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
				dbg_log(fLog,"INFO:moves_forbishop: DROPPING mov[%s] as others inbetween\n",movs[iCur]);
				strncpy(movs[iCur],"",32);
				continue;
			} else {
				dbg_log(fLog,"INFO:moves_forbishop: DOING mov[%s] no blockage inbetween\n",movs[iCur]);
			}

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forrook(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	char sTemp[8];
	int posS,posD;
	u64 bbFOcc = 0;

	cb_print(cbC);

	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wr;
		bbFOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	} else {
		bbS = cbC->br;
		bbFOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbRookMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			bbD &= ~(1ULL << posD);			

			strncpy(movs[iCur],"R",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));

			if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
				dbg_log(fLog,"INFO:moves_forrook: DROPPING mov[%s] as others inbetween\n",movs[iCur]);
				strncpy(movs[iCur],"",32);
				continue;
			}

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forqueen(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD, bbQ;
	char sTemp[8];
	int posS,posD;
	u64 bbFOcc = 0;

	cb_print(cbC);

	if(cbC->sideToMove == STM_WHITE) {
		bbQ = cbC->wq;
		bbFOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	} else {
		bbQ = cbC->bq;
		bbFOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}

	// Rook equivalent moves
	bbS = bbQ;
	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbRookMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			bbD &= ~(1ULL << posD);			

			strncpy(movs[iCur],"Q",32);
			strcat(movs[iCur],cb_bbpos2strloc(posS,sTemp));
			strcat(movs[iCur],"-");
			strcat(movs[iCur],cb_bbpos2strloc(posD,sTemp));

			if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
				dbg_log(fLog,"INFO:moves_forqueen: DROPPING mov[%s] as others inbetween\n",movs[iCur]);
				strncpy(movs[iCur],"",32);
				continue;
			}

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}

	// King equivalent moves
	bbS = bbQ;
	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbKingMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			strncpy(movs[iCur],"Q",32);
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
	iNew = moves_forbishop(cbC, movs, iNew);
	iNew = moves_forrook(cbC, movs, iNew);
	iNew = moves_forqueen(cbC, movs, iNew);
	
	if(iNew >= NUMOFPARALLELMOVES) {
		dbg_log(fLog,"FIXME:moves_get: List overflow ????\n");
		exit(200);
	}
	gMovesCnt += (iNew-iCur);
	return iNew;
}

