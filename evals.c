
int cb_evalpw_mat(struct cb *mcb)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;

	valB += __builtin_popcountll(mcb->bp)*VALUE_PAWN;
	valB += __builtin_popcountll(mcb->br)*VALUE_ROOK;
	valB += __builtin_popcountll(mcb->bn)*VALUE_KNIGHT;
	valB += __builtin_popcountll(mcb->bb)*VALUE_BISHOP;
	valB += __builtin_popcountll(mcb->bq)*VALUE_QUEEN;

	valW += __builtin_popcountll(mcb->wp)*VALUE_PAWN;
	valW += __builtin_popcountll(mcb->wr)*VALUE_ROOK;
	valW += __builtin_popcountll(mcb->wn)*VALUE_KNIGHT;
	valW += __builtin_popcountll(mcb->wb)*VALUE_BISHOP;
	valW += __builtin_popcountll(mcb->wq)*VALUE_QUEEN;

	valPW = valW-valB;
	return valPW;
}

int cb_eval_tANDp_fromknights(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wn;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = mcb->bn;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_fromknights:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbKnightMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_eval_tANDp_fromrooks(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wr;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = mcb->br;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_fromrooks:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbRookMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbRookMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_eval_tANDp_frombishops(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wb;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = mcb->bb;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_frombishops:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbBishopMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_eval_tANDp_fromqueens(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wq;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = mcb->bq;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_fromqueens:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbQueenMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_eval_tANDp_fromkings(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wk;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
	} else {
		cNBB = mcb->bk;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
	}
	// King cann't directly attack the other side king, but keeping for now, have to think bit more
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_fromkings:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbKingMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbKingMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_eval_tANDp_frompawns(struct cb *mcb, char activeSide)
{

	int nPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cNBB = 0;
	int weightage1, weightage2;
	u64 *bbPawnAttackMoves;

	if(activeSide == STM_WHITE) {
		cNBB = mcb->wp;
		weightage1 = 10; // threats_given
		weightage2 = 8; // protection_provided
		bbPawnAttackMoves = bbWhitePawnAttackMoves;
	} else {
		cNBB = mcb->bp;
		weightage1 = 8; // protection_provided
		weightage2 = 10; // threats_given
		bbPawnAttackMoves = bbBlackPawnAttackMoves;
	}
	while((nPos = ffsll(cNBB)) != 0) {
		nPos -= 1;
		//fprintf(fLog,"DEBUG:tANDp_frompawns:STM[%c]:cNBB[%0llx]:nPos[%d]\n",activeSide,cNBB,nPos);
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->bk) * VALUE_KING;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->bq) * VALUE_QUEEN;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->br) * VALUE_ROOK;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->bn) * VALUE_KNIGHT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->bb) * VALUE_BISHOP;
		val1 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->bp) * VALUE_PAWN;

		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wk) * VALUE_KING;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wq) * VALUE_QUEEN;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wr) * VALUE_ROOK;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wn) * VALUE_KNIGHT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wb) * VALUE_BISHOP;
		val2 += __builtin_popcountl(bbPawnAttackMoves[nPos] & mcb->wp) * VALUE_PAWN;
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

int cb_evalpw_threatsANDprotection(struct cb *mcb)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;

	valW += cb_eval_tANDp_fromknights(mcb,STM_WHITE);
	valB += cb_eval_tANDp_fromknights(mcb,STM_BLACK);
	valW += cb_eval_tANDp_fromrooks(mcb,STM_WHITE);
	valB += cb_eval_tANDp_fromrooks(mcb,STM_BLACK);
	valW += cb_eval_tANDp_frombishops(mcb,STM_WHITE);
	valB += cb_eval_tANDp_frombishops(mcb,STM_BLACK);
	valW += cb_eval_tANDp_fromqueens(mcb,STM_WHITE);
	valB += cb_eval_tANDp_fromqueens(mcb,STM_BLACK);
	valW += cb_eval_tANDp_fromkings(mcb,STM_WHITE);
	valB += cb_eval_tANDp_fromkings(mcb,STM_BLACK);
	valW += cb_eval_tANDp_frompawns(mcb,STM_WHITE);
	valB += cb_eval_tANDp_frompawns(mcb,STM_BLACK);
	valPW = valW - valB;
	return (valPW/10);
}

int cb_evalpw(struct cb *mcb)
{
	int valPW = 0;
	int valMat = 0;
	int valTandP = 0;

	valMat = cb_evalpw_mat(mcb);
	valTandP = cb_evalpw_threatsANDprotection(mcb);
	// eval_misc

	valPW = valMat + valTandP;
	fprintf(fLog,"valMat[%d] + valTandP[%d] = valPW[%d]\n", valMat, valTandP, valPW);
	return valPW;
}

