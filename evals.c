
//#define USE_POSEVAL 1
//#define USE_POSKNIGHTEVAL 1
#undef USE_POSKNIGHTEVAL

u64 CBRANKMASK[8] = {
	0x00000000000000FFULL,
	0x000000000000FF00ULL,
	0x0000000000FF0000ULL,
	0x00000000FF000000ULL,
	0x000000FF00000000ULL,
	0x0000FF0000000000ULL,
	0x00FF000000000000ULL,
	0xFF00000000000000ULL,
};

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

u64 CBKNIGHTPOSMASK[8] = {
	0xFF818181818181FFULL,
	0x007E7E7E7E7E7E00ULL,
};

int CBKNIGHTPOSEVAL[2] = {
	0,
	20
};


#define ATTACK_YES 1
#define ATTACK_NO 0

// xPos = 0 - 63
// Check how many bits set inbetween the attacker and attacked
// this will help identify if there is a positive attack or
// if there are other pieces inbetween which block the attack
// sPos = Position of Bishop/Queen which is attacking
// dPos = Position of the piece being attacked
// It should work even if sPos and dPos are swapped wrt attack
// but not wrt checking wrt the hint for Move generation.
int evalhlpr_diagattack(struct cb *cbC, int sPos, int dPos, int hint)
{
	int iRank,iFile,iDiff;
	u64 bbOcc,bbTOcc;
	int iSmall,iLarge;
	u64 uLRMask, uRLMask, cPos;
	int res1 = ATTACK_NO;
	int res2 = ATTACK_NO;
	u64 uTemp = 0;

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

	iRank = sPos/8;
	iFile = sPos%8;

	// Attacker and Attacked in a Diag 
	// BottomRight2TopLeft
	// BottomLeft2TopRight
	uRLMask = 0x0102040810204080ULL;
	uLRMask = 0x8040201008040201ULL;

	iDiff = iRank - iFile;
	if(iDiff < 0) { 
		uLRMask <<= abs(iDiff);
		uTemp = ((1ULL << (7+iDiff+1)*8) - 1);
		uLRMask = uLRMask & uTemp;
	} else if(iDiff > 0) {
		uLRMask >>= iDiff;
		uTemp = ~((1ULL << (iDiff)*8) - 1);
		uLRMask = uLRMask & uTemp;
	}
#ifdef DEBUG_DIAGATTACK
	dbg_log(fLog,"CHECK:DiagAttack:uLRMask (iDiff[%d])\n",iDiff);
#endif

	iDiff = 7 - iRank - iFile;
	if(iDiff < 0) {
		uRLMask <<= abs(iDiff);
		uTemp = ~((1ULL << ((abs(iDiff)*8)+1)) - 1);
		uRLMask = uRLMask & uTemp;
	} else if(iDiff > 0) {
		uRLMask >>= iDiff;
		uTemp = ((1ULL << (((7-iDiff)*8)+1)) - 1);
		uRLMask = uRLMask & uTemp;
	}

#ifdef DEBUG_DIAGATTACK
	dbg_log(fLog,"CHECK:DiagAttack:uRLMask (iDiff[%d])\n",iDiff);
	cb_bb_print(uRLMask | uLRMask);
#endif

	bbTOcc = bbOcc & uRLMask;
	while((cPos = ffsll(bbTOcc)) != 0) {
		cPos -= 1;
		bbTOcc &= ~(1ULL << cPos);
		if(cPos == iSmall) {
			if((cPos = ffsll(bbTOcc)) == 0) {
				res1 = ATTACK_NO; // Not on same line and No other pieces in the file
				break;
			} else {
				cPos -= 1;
				if(cPos == iLarge) {
					res1 = ATTACK_YES; // Yes no piece inbetween, so positive attack
					break;
				} else {
					res1 = ATTACK_NO; // Some other piece inbetween.
					break;
				}
			}
		}
	}

	bbTOcc = bbOcc & uLRMask;
	while((cPos = ffsll(bbTOcc)) != 0) {
		cPos -= 1;
		bbTOcc &= ~(1ULL << cPos);
		if(cPos == iSmall) {
			if((cPos = ffsll(bbTOcc)) == 0) {
				res2 = ATTACK_NO; // Not on same line and No other pieces in the file
				break;
			} else {
				cPos -= 1;
				if(cPos == iLarge) {
					res2 = ATTACK_YES; // Yes no piece inbetween, so positive attack
					break;
				} else {
					res2 = ATTACK_NO; // Some other piece inbetween.
					break;
				}
			}
		}
	}

	if( (res1 == ATTACK_YES) || (res2 == ATTACK_YES) )
		return ATTACK_YES;

	return ATTACK_NO; // Not on same line OR other pieces in Diag
}

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
		valW += __builtin_popcountl(bbKnightMoves[posP] & bbK) * VKING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}
	
	bbP = cbC->wp;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valW += __builtin_popcountl(bbWhitePawnAttackMoves[posP] & bbK) * VKING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->wr;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->wb;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_diagattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->wq;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VKING_ATTACKED;
			} else if(evalhlpr_diagattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valW += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	if(valW >= VKING_ATTACKED)
		cbC->bk_underattack += 1;
	else
		cbC->bk_underattack = 0;
	
	// White king being attacked
	valB = 0;
	bbK = cbC->wk;

	bbP = cbC->bn;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valB += __builtin_popcountl(bbKnightMoves[posP] & bbK) * VKING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}
	
	bbP = cbC->bp;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		valB += __builtin_popcountl(bbBlackPawnAttackMoves[posP] & bbK) * VKING_ATTACKED;
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->br;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->bb;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_diagattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	bbP = cbC->bq;
	while((posP = ffsll(bbP)) != 0) {
		posP -= 1;
		bbTK = bbK;
		while((posK = ffsll(bbTK)) != 0) {
			posK -= 1;
			if(evalhlpr_lineattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VKING_ATTACKED;
			} else if(evalhlpr_diagattack(cbC,posP,posK,-1) == ATTACK_YES) {
				valB += VKING_ATTACKED;
			}
			bbTK &= ~(1ULL << posK);
		}
		bbP &= ~(1ULL << posP);			
	}

	if(valB >= VKING_ATTACKED)
		cbC->wk_underattack += 1;
	else
		cbC->wk_underattack = 0;

	// Result
#ifdef DEBUG_EVALPRINT
	dbg_log(fLog,"INFO:kingunderattack: valW[%d] - valB[%d]\n", valW, valB);
#endif
	return ((valW-valB)/EVALSKINGUNDERATTACK_DIV);
}


void cb_check_kingkilled(struct cb *cbC)
{
	int valTemp;

	valTemp = __builtin_popcountll(cbC->bk)*VKING;
	if(valTemp == 0)
		cbC->bk_killed = 1;
	valTemp = __builtin_popcountll(cbC->wk)*VKING;
	if(valTemp == 0)
		cbC->wk_killed = 1;
}


// TODO: MAYBE
// A Pawn towards center or opposite end is more valuable than pawn at start pos
// A Knight towards side is less valuable than knight towards the center.
// All pieces in their start pos may be less valuable than towards center in beginning
// and mid game.

#define NOTE_SINGLEKING_ONLY 1

int cb_evalpw_mat(struct cb *cbC, int *iTotMat)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;
	int valTemp = 0;

	valB += __builtin_popcountll(cbC->bp)*VPAWN;
	valB += __builtin_popcountll(cbC->br)*VROOK;
	valB += __builtin_popcountll(cbC->bn)*VKNIGHT;
	valB += __builtin_popcountll(cbC->bb)*VBISHOP;
	valB += __builtin_popcountll(cbC->bq)*VQUEEN;
	valTemp = __builtin_popcountll(cbC->bk)*VKING;
	valB += valTemp;
#ifdef NOTE_SINGLEKING_ONLY
	cbC->bk_killed = valTemp ^ VKING;
#else
	if(valTemp == 0)
		cbC->bk_killed = 1;
#endif

	valW += __builtin_popcountll(cbC->wp)*VPAWN;
	valW += __builtin_popcountll(cbC->wr)*VROOK;
	valW += __builtin_popcountll(cbC->wn)*VKNIGHT;
	valW += __builtin_popcountll(cbC->wb)*VBISHOP;
	valW += __builtin_popcountll(cbC->wq)*VQUEEN;
	valTemp = __builtin_popcountll(cbC->wk)*VKING;
	valW += valTemp;
#ifdef NOTE_SINGLEKING_ONLY
	cbC->wk_killed = valTemp ^ VKING;
#else
	if(valTemp == 0)
		cbC->wk_killed = 1;
#endif

	valPW = (valW-valB)/EVALSMAT_DIV;

	*iTotMat = valW+valB;
	if(*iTotMat > MATTHRESHOLD_MIDGAME)
		cbC->gameState = GS_START;
	else if(*iTotMat > MATTHRESHOLD_ENDGAME)
		cbC->gameState = GS_MID;
	else
		cbC->gameState = GS_END;

#ifdef DEBUG_EVALPRINT
	dbg_log(fLog,"INFO:evalpw_mat: valW[%d] - valB[%d]\n", valW, valB);
#endif
	return valPW;
}


int cb_evalpw_pos(struct cb *cbC)
{
	int valPW = 0;
	int valB = 0;
	int valW = 0;
	int iCur;
	int valPWKnight = 0;
	int valPWPawn = 0;

	for(iCur = 1; iCur < 8; iCur++) {
		valW += __builtin_popcountll(cbC->wp & CBRANKMASK[iCur])*((VPAWN*(iCur-1))/10);
	}
	for(iCur = 6; iCur >= 0; iCur--) {
		valB += __builtin_popcountll(cbC->bp & CBRANKMASK[iCur])*((VPAWN*(6-iCur))/10);
	}
	valPWPawn = (valW-valB);
	if(cbC->gameState == GS_MID)
		valPWPawn = valPWPawn >> 2;
#ifdef USE_POSKNIGHTEVAL
	if(cbC->gameState == GS_START) {
		valW = 0;
		valB = 0;
		for(iCur = 0; iCur < 2; iCur++) {
			valW += __builtin_popcountll(cbC->wn & CBKNIGHTPOSMASK[iCur])*CBKNIGHTPOSEVAL[iCur];
		}
		for(iCur = 0; iCur < 2; iCur++) {
			valB += __builtin_popcountll(cbC->bn & CBKNIGHTPOSMASK[iCur])*CBKNIGHTPOSEVAL[iCur];
		}
		valPWKnight = (valW-valB);
	}
#endif

	valPW = (valPWPawn + valPWKnight)/EVALSPOS_DIV;
#ifdef DEBUG_EVALPRINT
	dbg_log(fLog,"INFO:evalpw_pos: valPWPawn[%d], valPWKnight[%d]\n", valPWPawn, valPWKnight);
#endif
	return valPW;
}


// NOTE: GENERIC
//
// The threat and protection value is bit complicated in that it depends on
// whether there are intermediate pieces in the path as well as whether the
// piece giving the threat or protection is knight, pawn or king OR if it is
// one of the other pieces like Queen, Rook or Bishop - which can slide thro
// multiple squares in a single move provided there are no intervening pieces
// in the board.
// In case of knight, has it can jump across other intervening pieces 
// calculating the threat and protection is relatively straight forward.
// Similarly in case of Pawn or King, as they can move only 1 square at any 
// given time/move again the calculation is straight forward.
// HOWEVER in case of the sliding pieces ie Queen, Rook or Bishop, while
// calculating the threat given or portection provided, one has to see if the
// there are intervening pieces which block the protection or threat and based
// on that give a weighted value for these.
//
// ALSO The protection value is ideally bit more complicated in that it is 
// dependent on whether there is a immidiate threat to the piece being protected 
// or whether that piece is safe in the short term. TODO:TOTHINK: Currently this 
// temporal dependence is not accounted directly. However indirectly it is
// accounted in that depth search for Moves will take these factors into
// account automatically because of the way the depth search works. So this may
// be good enough wrt this aspect of calculating protection value.
//

int cb_eval_tANDp_fromknights(struct cb *cbC, char activeSide)
{

	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wn;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION/2; // protection_provided
	} else {
		cSBB = cbC->bn;
		weightage1 = WEIGHTAGE_PROTECTION/2; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
	}
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromknights:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->bk) * OAVMULT(VKING) * WT_DIRECT;
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->bq) * OAVMULT(VQUEEN) * WT_DIRECT;
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->br) * OAVMULT(VROOK) * WT_DIRECT;
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->bn) * VKNIGHT * WT_DIRECT;
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->bb) * VBISHOP * WT_DIRECT;
		val1 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->bp) * VPAWN * WT_DIRECT;

		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wk) * OAVMULT(VKING) * WT_DIRECT;
		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wq) * OAVMULT(VQUEEN) * WT_DIRECT;
		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wr) * OAVMULT(VROOK) * WT_DIRECT;
		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wn) * VKNIGHT * WT_DIRECT;
		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wb) * VBISHOP * WT_DIRECT;
		val2 += __builtin_popcountl(bbKnightMoves[sPos] & cbC->wp) * VPAWN * WT_DIRECT;
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromKnights:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// TODO:TOTHINK: The protection value is bit more complicated in that it is dependent on whether
	// the threat from the opposite side is coming from a knight or other pieces.
	// Because if threat from knight, then the protection is full and needs to be fully accounted
	// However if threat from other pieces, then the protection needs to be fully or partially accounted
	// depending on whether other peices are in the path which block the threat in the first place or not.
	// FORNOW: Protection value is given same weihtage always.
	return val;
}

int eval_lineattack(struct cb *cbC, int sPos, u64 dBB, int pieceValue, int wDirect, int wIndirect)
{
	int dPos;
	int iVal = 0;

	while((dPos = ffsll(dBB)) != 0) {
		dPos -= 1;
		if(evalhlpr_lineattack(cbC,sPos,dPos,-1) == ATTACK_YES) {
			iVal += DIRMULT(pieceValue*wDirect);
		} else {
			iVal += INDMULT(pieceValue*wIndirect);
		}
		dBB &= ~(1ULL << dPos);			
	}
	return iVal;
}

int eval_diagattack(struct cb *cbC, int sPos, u64 dBB, int pieceValue, int wDirect, int wIndirect)
{
	int dPos;
	int iVal = 0;

	while((dPos = ffsll(dBB)) != 0) {
		dPos -= 1;
		if(evalhlpr_diagattack(cbC,sPos,dPos,-1) == ATTACK_YES) {
			iVal += DIRMULT(pieceValue*wDirect);
		} else {
			iVal += INDMULT(pieceValue*wIndirect);
		}
		dBB &= ~(1ULL << dPos);			
	}
	return iVal;
}

int cb_eval_tANDp_fromrooks(struct cb *cbC, char activeSide)
{

	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wr;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION; // protection_provided
	} else {
		cSBB = cbC->br;
		weightage1 = WEIGHTAGE_PROTECTION; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
	}
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromrooks:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bk), OAVMULT(VKING),WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bq), OAVMULT(VQUEEN),WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->br), VROOK,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bp), VPAWN,WT_DIRECT,WT_INDIRECT);

		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wk), OAVMULT(VKING),WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wq), OAVMULT(VQUEEN),WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wr), VROOK,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wp), VPAWN,WT_DIRECT,WT_INDIRECT);
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromRooks:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// FIXED: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_frombishops(struct cb *cbC, char activeSide)
{

	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wb;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION; // protection_provided
	} else {
		cSBB = cbC->bb;
		weightage1 = WEIGHTAGE_PROTECTION; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
	}
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_frombishops:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bk), OAVMULT(VKING),WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bq), OAVMULT(VQUEEN),WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->br), OAVMULT(VROOK),WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bp), VPAWN,WT_DIRECT,WT_INDIRECT);

		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wk), OAVMULT(VKING),WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wq), OAVMULT(VQUEEN),WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wr), OAVMULT(VROOK),WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wp), VPAWN,WT_DIRECT,WT_INDIRECT);
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromBishops:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// FIXED: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_fromqueens(struct cb *cbC, char activeSide)
{

	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wq;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION; // protection_provided
	} else {
		cSBB = cbC->bq;
		weightage1 = WEIGHTAGE_PROTECTION; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
	}
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromqueens:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bk), VKING,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bk), VKING,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bq), VQUEEN,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bq), VQUEEN,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->br), VROOK,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->br), VROOK,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val1 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->bp), UAVMULT(VPAWN),WT_DIRECT,WT_INDIRECT);
		val1 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->bp), UAVMULT(VPAWN),WT_DIRECT,WT_INDIRECT);

		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wk), VKING,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wk), VKING,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wq), VQUEEN,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wq), VQUEEN,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wr), VROOK,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wr), VROOK,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wn), VKNIGHT,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wb), VBISHOP,WT_DIRECT,WT_INDIRECT);
		val2 += eval_lineattack(cbC,sPos,(bbRookMoves[sPos] & cbC->wp), UAVMULT(VPAWN),WT_DIRECT,WT_INDIRECT);
		val2 += eval_diagattack(cbC,sPos,(bbBishopMoves[sPos] & cbC->wp), UAVMULT(VPAWN),WT_DIRECT,WT_INDIRECT);
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromQueens:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// FIXED: The threat and protection value is bit more complicated than provided above because
	// If there are intervening pieces in the path, then the effect of threat and protection is 
	// not the same as if there is a clear path.
	// FORNOW: Threat and Protection values given same weightage always, irrespective of intervening pieces or not.
	return val;
}

int cb_eval_tANDp_fromkings(struct cb *cbC, char activeSide)
{

	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wk;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION; // protection_provided
	} else {
		cSBB = cbC->bk;
		weightage1 = WEIGHTAGE_PROTECTION; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
	}
	// King cann't directly attack the other side king, but keeping for now, have to think bit more
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_fromkings:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->bk) * VKING * WT_DIRECT;
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->bq) * VQUEEN * WT_DIRECT;
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->br) * VROOK * WT_DIRECT;
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->bn) * VKNIGHT * WT_DIRECT;
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->bb) * VBISHOP * WT_DIRECT;
		val1 += __builtin_popcountl(bbKingMoves[sPos] & cbC->bp) * VPAWN * WT_DIRECT;

		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wk) * VKING * WT_DIRECT;
		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wq) * VQUEEN * WT_DIRECT;
		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wr) * VROOK * WT_DIRECT;
		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wn) * VKNIGHT * WT_DIRECT;
		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wb) * VBISHOP * WT_DIRECT;
		val2 += __builtin_popcountl(bbKingMoves[sPos] & cbC->wp) * VPAWN * WT_DIRECT;
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromKings:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// NOTE: The threat given and protection provided by King is straightforward, has it can only move 
	// one square at any given time.
	// However a King cann't attack the opposite King, this has to be thought thro and accounted in future
	return val;
}

int cb_eval_tANDp_frompawns(struct cb *cbC, char activeSide)
{
	int sPos = 0;
	int val = 0;
	int val1 = 0;
	int val2 = 0;
	u64 cSBB = 0;
	int weightage1, weightage2;
	u64 *bbPawnAttackMoves;

	if(activeSide == STM_WHITE) {
		cSBB = cbC->wp;
		weightage1 = WEIGHTAGE_THREAT; // threats_given
		weightage2 = WEIGHTAGE_PROTECTION; // protection_provided
		bbPawnAttackMoves = bbWhitePawnAttackMoves;
	} else {
		cSBB = cbC->bp;
		weightage1 = WEIGHTAGE_PROTECTION; // protection_provided
		weightage2 = WEIGHTAGE_THREAT; // threats_given
		bbPawnAttackMoves = bbBlackPawnAttackMoves;
	}
	while((sPos = ffsll(cSBB)) != 0) {
		sPos -= 1;
		//dbg_log(fLog,"DEBUG:tANDp_frompawns:STM[%c]:cSBB[%0llx]:sPos[%d]\n",activeSide,cSBB,sPos);
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->bk) * OAVMULT(VKING) * WT_DIRECT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->bq) * OAVMULT(VQUEEN) * WT_DIRECT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->br) * OAVMULT(VROOK) * WT_DIRECT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->bn) * OAVMULT(VKNIGHT) * WT_DIRECT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->bb) * OAVMULT(VBISHOP) * WT_DIRECT;
		val1 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->bp) * VPAWN * WT_DIRECT;

		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wk) * OAVMULT(VKING) * WT_DIRECT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wq) * OAVMULT(VQUEEN) * WT_DIRECT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wr) * OAVMULT(VROOK) * WT_DIRECT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wn) * OAVMULT(VKNIGHT) * WT_DIRECT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wb) * OAVMULT(VBISHOP) * WT_DIRECT;
		val2 += __builtin_popcountl(bbPawnAttackMoves[sPos] & cbC->wp) * VPAWN * WT_DIRECT;
		cSBB &= ~(1ULL << sPos);			
	}

	// For White val1 = threats_given; val2 = protection_provided
	// For Black val2 = threats_given; val1 = protection_provided
	val = ((val1 * weightage1) + (val2 * weightage2))/WEIGHTAGE_SCALE;
#ifdef DEBUG_EVALPRINT	
	dbg_log(fLog,"INFO:tANDp_fromPawns:val1[%d] * weightage1[%d] + val2[%d] * weightage2[%d] = val[%d]\n",
			val1,weightage1, val2, weightage2, val);
#endif
	// NOTE: The threat given and protection provided by Pawn is straightforward, has it can only move 
	// one square at any given time.
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

	valPW = (valW - valB)/EVALSTANDP_DIV;
#ifdef DEBUG_EVALPRINT
	dbg_log(fLog,"INFO:evalpw_tANDp: valW[%d] - valB[%d]\n", valW, valB);
#endif
	return valPW;
}

int cb_evalpw(struct cb *cbC)
{
	int valPW = 0;
	int valMat = 0;
	int valTandP = 0;
	int valKingAttacked = 0;
	int valPos = 0;
	int iTotalMaterial = 0;

	valMat = cb_evalpw_mat(cbC,&iTotalMaterial);
	//valTandP = cb_evalpw_threatsANDprotection(cbC);
	//valKingAttacked = cb_evalpw_king_underattack(cbC);
	// eval_misc
#ifdef USE_POSEVAL
	valPos = cb_evalpw_pos(cbC);
#endif

	valPW = valMat + valTandP + valKingAttacked + valPos;

#ifdef DEBUG_EVALSUMMARYPRINT	
	dbg_log(fLog,"valMat[%d] + valTandP[%d] + valKingAttacked[%d] + valPos[%d] = valPW[%d] <=> Moves[%s]\n",
				valMat, valTandP, valKingAttacked, valPos, valPW, cbC->sMoves);
#endif
	return valPW;
}

