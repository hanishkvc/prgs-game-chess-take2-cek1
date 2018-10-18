
int moves_forpawnattacks(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
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
			movs[iCur][0] = 'P';
			movs[iCur][1] = posS;
			movs[iCur][2] = 'x';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;
			if(((posD >= 0) && (posD < 8)) || ((posD >= 56) && (posD < 64))) {
				movs[iCur][5] = 0;
				memcpy(movs[iCur+1],movs[iCur],6);
				memcpy(movs[iCur+2],movs[iCur],6);
				memcpy(movs[iCur+3],movs[iCur],6);
				movs[iCur][4] = SM_PROMOTE2QUEEN;
				movs[iCur+1][4] = SM_PROMOTE2KNIGHT;
				movs[iCur+2][4] = SM_PROMOTE2ROOK;
				movs[iCur+3][4] = SM_PROMOTE2BISHOP;
				iCur += 3;
			}
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

			movs[iCur][0] = 'P';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;

			if(abs(posS-posD) > 8) {
				if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
#ifdef DEBUG_MOVEGENPRINT
					dbg_log(fLog,"INFO:moves_forpawnnormal: DROPPING mov[%s] as others inbetween\n",movs[iCur]);
#endif
					movs[iCur][0] = 0;
					continue;
				}
			} else {
				if(((posD >= 0) && (posD < 8)) || ((posD >= 56) && (posD < 64))) {
					movs[iCur][5] = 0;
					memcpy(movs[iCur+1],movs[iCur],6);
					memcpy(movs[iCur+2],movs[iCur],6);
					memcpy(movs[iCur+3],movs[iCur],6);
					movs[iCur][4] = SM_PROMOTE2QUEEN;
					movs[iCur+1][4] = SM_PROMOTE2KNIGHT;
					movs[iCur+2][4] = SM_PROMOTE2ROOK;
					movs[iCur+3][4] = SM_PROMOTE2BISHOP;
					iCur += 3;
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
	int posS,posD;
	u64 bbFOcc = 0;

#ifdef DEBUG_MOVESPRINTCB
	cb_print(cbC);
#endif

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

			if(evalhlpr_diagattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
#ifdef DEBUG_MOVEGENPRINT
				dbg_log(fLog,"INFO:moves_forbishop: DROPPING mov %d->%d as others inbetween\n", posS, posD);
#endif
				continue;
			} else {
#ifdef DEBUG_MOVEGENPRINT
				dbg_log(fLog,"INFO:moves_forbishop: DOING mov %d->%d no blockage inbetween\n", posS, posD);
#endif
			}
			movs[iCur][0] = 'B';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forrook(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	int posS,posD;
	u64 bbFOcc = 0;

#ifdef DEBUG_MOVESPRINTCB
	cb_print(cbC);
#endif

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

			if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
#ifdef DEBUG_MOVEGENPRINT
				// this movs setup added here TEMP to only show that mov[%s] wouldn't have worked,
				// and the new %d->%d related logic is what will give proper values
				movs[iCur][0] = 'R';
				movs[iCur][1] = posS;
				movs[iCur][2] = '-';
				movs[iCur][3] = posD;
				movs[iCur][4] = 0;
				dbg_log(fLog,"INFO:moves_forrook: DROPPING mov[%s] %d->%d as others inbetween\n", movs[iCur], posS, posD);
				strncpy(movs[iCur],"",32);
#endif
				continue;
			}
			movs[iCur][0] = 'R';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

int moves_forqueen(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD, bbQ;
	int posS,posD;
	u64 bbFOcc = 0;

#ifdef DEBUG_MOVESPRINTCB
	cb_print(cbC);
#endif

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

			if(evalhlpr_lineattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
#ifdef DEBUG_MOVEGENPRINT
				dbg_log(fLog,"INFO:moves_forqueen: DROPPING mov %d->%d as others inbetween\n", posS, posD);
#endif
				continue;
			}
			movs[iCur][0] = 'Q';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}
	
	// Bishop equivalent moves
	bbS = bbQ;
	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbBishopMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			bbD &= ~(1ULL << posD);			

			if(evalhlpr_diagattack(cbC,posS,posD,LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) != ATTACK_YES) {
#ifdef DEBUG_MOVEGENPRINT
				dbg_log(fLog,"INFO:moves_forqueen: DROPPING mov %d->%d as others inbetween\n", posS, posD);
#endif
				continue;
			}
			movs[iCur][0] = 'Q';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;

			iCur += 1;
		}
		bbS &= ~(1ULL << posS);			
	}

	/* Has both Rook and Bishop equivalent moves have been added now. The king moves is not needed.
         * King moves was providing a minimal movement and attack to Queen, when Rook and Bishop moves where not there.
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
	*/

	return iCur;
}

int moves_forknight(struct cb *cbC, char movs[512][32], int iCur)
{
	u64 bbS, bbD;
	int posS,posD;
	u64 bbFOcc = 0;

	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wn;
		bbFOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	} else {
		bbS = cbC->bn;
		bbFOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbKnightMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			movs[iCur][0] = 'N';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;
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
	int posS,posD;
	u64 bbFOcc = 0;

	if(cbC->sideToMove == STM_WHITE) {
		bbS = cbC->wk;
		bbFOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC->wp;
	} else {
		bbS = cbC->bk;
		bbFOcc = cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC->bp;
	}

	while((posS = ffsll(bbS)) != 0) {
		posS -= 1;
		bbD = bbKingMoves[posS];
		bbD = bbD & ~bbFOcc;
		while((posD = ffsll(bbD)) != 0) {
			posD -= 1;
			movs[iCur][0] = 'K';
			movs[iCur][1] = posS;
			movs[iCur][2] = '-';
			movs[iCur][3] = posD;
			movs[iCur][4] = 0;
			//check_iskingattacked(posD);
			iCur += 1;
			bbD &= ~(1ULL << posD);
		}
		bbS &= ~(1ULL << posS);			
	}
	return iCur;
}

#define TRACK_GLOBALMOVECNT 1

int moves_get(struct cb *cbC, char movs[512][32], int iCur)
{
	int iNew,iDiff;

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
#ifdef TRACK_GLOBALMOVECNT
	iDiff = (iNew-iCur);
	gMovesCnt += iDiff;
#ifdef DEBUG_MOVESGETCNTPRINT
	dbg_log(fLog,"INFO:moves_get: NewMoves[%d], AllTotalMovesTillNow[%lld]\n",iDiff,gMovesCnt);
#endif
#endif
	return iNew;
}

