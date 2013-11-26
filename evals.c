
#define VALUE_KING_ATTACKED (VALUE_KING*4)

int cb_evalpw_king_underattack(struct cb *cbC)
{
	int posP;
	u64 bbK;
	u64 bbP;
	int valW,valB;

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

	// Result
	fprintf(fLog,"INFO:kingunderattack: valW[%d] - valB[%d]\n", valW, valB);
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
		//fprintf(fLog,"DEBUG:tANDp_fromknights:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromKnights:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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
		//fprintf(fLog,"DEBUG:tANDp_fromrooks:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromRooks:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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
		//fprintf(fLog,"DEBUG:tANDp_frombishops:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromBishops:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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
		//fprintf(fLog,"DEBUG:tANDp_fromqueens:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromQueens:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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
		//fprintf(fLog,"DEBUG:tANDp_fromkings:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromKings:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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
		//fprintf(fLog,"DEBUG:tANDp_frompawns:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
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
	fprintf(fLog,"INFO:tANDp_fromPawns:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
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

	fprintf(fLog,"valMat[%d] + valTandP[%d] + valKingAttacked[%d] = valPW[%d] <=> Moves[%s]\n",
				valMat, valTandP, valKingAttacked, valPW, cbC->sMoves);
	return valPW;
}

