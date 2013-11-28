
#define VALUE_KING_ATTACKED (VALUE_KING*4)


u64 CBFILEMASK[8] = { 
	0x0101010101010101ULL,
	0x0202020202020202ULL,
	0x0404040404040404ULL,
	0x0808080808080808ULL,
	0x1010101010101010ULL,
	0x2020202020202020ULL,
	0x4040404040404040ULL,
	0x8080808080808080ULL,
};

#define ATTACK_YES 1
#define ATTACK_NO 0

// xPos = 0 - 63
// Check how many bits set inbetween the attacker and attacked
// this will help identify if there is a positive attack or
// if there are other pieces inbetween which block the attack
// sPos = Position of Rook which is attacking
// dPos = Position of the piece being attacked
int evalhlpr_lineattack(struct cb *cbC, int sPos, int dPos, int hint)
{
	int iDiff;
	u64 bbOcc;
	int iCnt,iSmall,iLarge;
	u64 uMask, cPos;

	bbOcc = cbC->wk | cbC->wq | cbC->wr | cbC->wn | cbC->wb | cbC-> wp
		 | cbC->bk | cbC->bq | cbC->br | cbC->bn | cbC->bb | cbC-> bp;

	if(hint == LINEATTACK_HINT_PAWNSTART2CHECKINBETWEEN) {
		bbOcc |= (1ULL << sPos);
		bbOcc |= (1ULL << dPos);
	}
	
	if(sPos > dPos) {
		iSmall = dPos;
		iLarge = sPos;
	} else {
		iSmall = sPos;
		iLarge = dPos;
	}
	iDiff = abs(sPos - dPos);
	if( (iDiff <= 7) && ((iSmall/8) == (iLarge/8)) ) { // Attacker and Attacked in a Rank

		// Should be able to mask out every bit before the higher/larger number towards msb 
		// and everything after the smaller number (i.e pos) towards the lsb
		uMask = (1ULL << (iSmall));
		uMask -= 1;
		uMask = ~uMask;
		bbOcc = bbOcc & uMask;

		uMask = (1ULL << (iLarge));
		uMask = ((uMask-1) | (1ULL << iLarge));
		bbOcc = bbOcc & uMask;

		iCnt = __builtin_popcountll(bbOcc);
		if(iCnt > 2) {
			return ATTACK_NO; // Some other piece inbetween
		} else if(iCnt == 2) {
			return ATTACK_YES; // Yes no piece inbetween, so Positive attack
		} else {
			dbg_log(fLog,"FIXME:lineattack: BUG in code ????\n");
			exit(300); // FIXME: BUG IN CODE/Logic, if this is hit
		}
		
	} else { // Attacker and Attacked in a File
		uMask = 0x0101010101010101ULL;

		uMask <<= (sPos%8);
		bbOcc = bbOcc & uMask;
		while((cPos = ffsll(bbOcc)) != 0) {
			cPos -= 1;
			bbOcc &= ~(1ULL << cPos);
			if(cPos == iSmall) {
				if((cPos = ffsll(bbOcc)) == 0) {
					return ATTACK_NO; // Not on same line and No other pieces in the file
				} else {
					cPos -= 1;
					if(cPos == iLarge) {
						return ATTACK_YES; // Yes no piece inbetween, so positive attack
					} else {
						return ATTACK_NO; // Some other piece inbetween.
					}
				}
			}
		}
	}
	return ATTACK_NO; // Not on same line, other pieces in file
}

int cb_evalpw_king_underattack(struct cb *cbC)
{
	int posP;
	u64 bbK, bbTK;
	u64 bbP;
	int valW,valB;
	int posK;

	// Black king being attacked
	valW = 0;
	bbK = cbC->bk;

	bbP = cbC->wn;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valW += __builtin_popcountl(bbKnightMoves[posP] & bbK) * VALUE_KING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}
	
	bbP = cbC->wp;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valW += __builtin_popcountl(bbWhitePawnAttackMoves[posP] & bbK) * VALUE_KING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->wr;
	bbTK = bbK;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VALUE_KING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->wq;
	bbTK = bbK;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VALUE_KING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	if(valW >= VALUE_KING_ATTACKED)
		cbC->bk_underattack += 1;
	else
		cbC->bk_underattack = 0;
	
	// White king being attacked
	valB = 0;
	bbK = cbC->wk;

	bbP = cbC->bn;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valB += __builtin_popcountl(bbKnightMoves[posP] & bbK) * VALUE_KING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}
	
	bbP = cbC->bp;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valB += __builtin_popcountl(bbBlackPawnAttackMoves[posP] & bbK) * VALUE_KING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->br;
	bbTK = bbK;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VALUE_KING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->bq;
	bbTK = bbK;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VALUE_KING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	if(valB >= VALUE_KING_ATTACKED)
		cbC->wk_underattack += 1;
	else
		cbC->wk_underattack = 0;

	// Result
	dbg_log(fLog,"INFO:kingunderattack: valW[%d] - valB[%d]\n", valW, valB);
	return (valW-valB);
}

int cb_evalpw_mat(struct cb *cbC)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;

	valB += __builtin_popcountll(cbC->bp)*VALUE_PAWN;
	valB += __builtin_popcountll(cbC->br)*VALUE_ROOK;
	valB += __builtin_popcountll(cbC->bn)*VALUE_KNIGHT;
	valB += __builtin_popcountll(cbC->bb)*VALUE_BISHOP;
	valB += __builtin_popcountll(cbC->bq)*VALUE_QUEEN;

	valW += __builtin_popcountll(cbC->wp)*VALUE_PAWN;
	valW += __builtin_popcountll(cbC->wr)*VALUE_ROOK;
	valW += __builtin_popcountll(cbC->wn)*VALUE_KNIGHT;
	valW += __builtin_popcountll(cbC->wb)*VALUE_BISHOP;
	valW += __builtin_popcountll(cbC->wq)*VALUE_QUEEN;

	valPW = valW-valB;
	return valPW;
}

int cb_eval_tANDp_fromknights(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wn;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = cbC->bn;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromknights:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromKnights:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The protection value is bit more complicated in that it is dependent on whether
	// the threat from the opposite side is coming from a knight or other pieces.
	// Because if threat from knight, then the protection is full and needs to be fully accounted
	// However if threat from other pieces, then the protection needs to be fully or partially accounted
	// depending on whether other peices are in the path which block the threat in the first place or not.
	// FORNOW: Protection value is given same weihtage always.
	return val;
}

int cb_eval_tANDp_fromrooks(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wr;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = cbC->br;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromrooks:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromRooks:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_frombishops(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wb;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = cbC->bb;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_frombishops:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromBishops:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_fromqueens(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wq;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = cbC->bq;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromqueens:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromQueens:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_fromkings(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wk;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = cbC->bk;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	// King cann't directly attack the other side king, but keeping for now, have to think bit more
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromkings:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromKings:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_frompawns(struct cb *cbC, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;
	u64 *bbPawnAttackMoves;

	if(activeSide == STM_WHITE) {
		cNBB = cbC->wp;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
		bbPawnAttackMoves = bbWhitePawnAttackMoves;
	} else {
		cNBB = cbC->bp;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
		bbPawnAttackMoves = bbBlackPawnAttackMoves;
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_frompawns:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & cbC->wp) * VALUE_PAWN;
		cNBB &= ~(1ULL << nPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/10;
	dbg_log(fLog,"INFO:tANDp_fromPawns:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
	// FIXME: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_evalpw_threatsANDprotection(struct cb *cbC)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;

	valW += cb_eval_tANDp_fromknights(cbC,STM_WHITE);
	valB += cb_eval_tANDp_fromknights(cbC,STM_BLACK);
	valW += cb_eval_tANDp_fromrooks(cbC,STM_WHITE);
	valB += cb_eval_tANDp_fromrooks(cbC,STM_BLACK);
	valW += cb_eval_tANDp_frombishops(cbC,STM_WHITE);
	valB += cb_eval_tANDp_frombishops(cbC,STM_BLACK);
	valW += cb_eval_tANDp_fromqueens(cbC,STM_WHITE);
	valB += cb_eval_tANDp_fromqueens(cbC,STM_BLACK);
	valW += cb_eval_tANDp_fromkings(cbC,STM_WHITE);
	valB += cb_eval_tANDp_fromkings(cbC,STM_BLACK);
	valW += cb_eval_tANDp_frompawns(cbC,STM_WHITE);
	valB += cb_eval_tANDp_frompawns(cbC,STM_BLACK);
	valPW = valW - valB;
	return (valPW/10);
}

int cb_evalpw(struct cb *cbC)
{
	int valPW = 0;
	int valMat = 0;
	int valTandP = 0;
	int valKingAttacked = 0;

	valMat = cb_evalpw_mat(cbC);
	valTandP = cb_evalpw_threatsANDprotection(cbC);
	valKingAttacked = cb_evalpw_king_underattack(cbC);
	// eval_misc

	valPW = valMat + valTandP + valKingAttacked;

	dbg_log(fLog,"valMat[%d] + valTandP[%d] + valKingAttacked[%d] = valPW[%d] <=> Moves[%s]\n",
				valMat, valTandP, valKingAttacked, valPW, cbC->sMoves);
	return valPW;
}

