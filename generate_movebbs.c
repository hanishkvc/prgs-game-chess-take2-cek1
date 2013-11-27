
u64 bbKnightMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
	0x0000000000020400ULL,
	0x0000000000050800ULL,
	0x00000000000a1100ULL,
};

void generate_bb_knightmoves(u64 *bbc)
{
	int r,f;
	int nr,nf,curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbc[curPos] = 0;

			nr = r + 2;
			nf = f + 1;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r + 2;
			nf = f - 1;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r - 2;
			nf = f + 1;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r - 2;
			nf = f - 1;
			cb_bb_setpos(&bbc[curPos],nr,nf);

			nr = r + 1;
			nf = f + 2;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r - 1;
			nf = f + 2;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r + 1;
			nf = f - 2;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			nr = r - 1;
			nf = f - 2;
			cb_bb_setpos(&bbc[curPos],nr,nf);
			
			fprintf(fLog,"INFO:bb_knightmoves:pos[%d] attacksquares[%0llx]\n",curPos,bbc[curPos]);
			dbg_cb_bb_print(bbc[curPos]);
		}
	}
}

u64 bbRookMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
	0x01010101010101ffULL,
	0x02020202020202ffULL,
	0x04040404040404ffULL,
};

void generate_bb_rookmoves(u64 *bbc)
{
	int r,f;
	int nr,nf,curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbc[curPos] = 0;
			for(nr = r+1; nr <8; nr++) {
				nf = f;
				cb_bb_setpos(&bbc[curPos],nr,nf);
			}
			for(nr = r-1; nr >= 0; nr--) {
				nf = f;
				cb_bb_setpos(&bbc[curPos],nr,nf);
			}
			for(nf = f+1; nf <8; nf++) {
				nr = r;
				cb_bb_setpos(&bbc[curPos],nr,nf);
			}
			for(nf = f-1; nf >= 0; nf--) {
				nr = r;
				cb_bb_setpos(&bbc[curPos],nr,nf);
			}
			
			fprintf(fLog,"INFO:bb_rookmoves:pos[%d] attacksquares[%0llx]\n",curPos,bbc[curPos]);
			dbg_cb_bb_print(bbc[curPos]);
		}
	}
}

u64 bbBishopMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};

void generate_bb_bishopmoves(u64 *bbc)
{
	int r,f;
	int nr,nf1,nf2,curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbc[curPos] = 0;
			nf1 = f + 1;
			nf2 = f - 1;
			for(nr = r+1; nr <8; nr++) {
				cb_bb_setpos(&bbc[curPos],nr,nf1);
				cb_bb_setpos(&bbc[curPos],nr,nf2);
				nf1 = nf1 + 1;
				nf2 = nf2 - 1;
			}
			nf1 = f + 1;
			nf2 = f - 1;
			for(nr = r-1; nr >= 0; nr--) {
				cb_bb_setpos(&bbc[curPos],nr,nf1);
				cb_bb_setpos(&bbc[curPos],nr,nf2);
				nf1 = nf1 + 1;
				nf2 = nf2 - 1;
			}

			fprintf(fLog,"INFO:bb_bishopmoves:pos[%d] attacksquares[%0llx]\n",curPos,bbc[curPos]);
			dbg_cb_bb_print(bbc[curPos]);
		}
	}
}

u64 bbQueenMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};

void generate_bb_queenmoves(u64 *bbq, u64 *bbr, u64 *bbb)
{
	int r,f;
	int curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbq[curPos] = bbr[curPos] | bbb[curPos];
			fprintf(fLog,"INFO:bb_queenmoves:pos[%d] attacksquares[%0llx]\n",curPos,bbq[curPos]);
			dbg_cb_bb_print(bbq[curPos]);
		}
	}
}

u64 bbKingMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};

void generate_bb_kingmoves(u64 *bbc)
{
	int r,f;
	int curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbc[curPos] = 0;
			cb_bb_setpos(&bbc[curPos], r-1, f+1);
			cb_bb_setpos(&bbc[curPos], r+0, f+1);
			cb_bb_setpos(&bbc[curPos], r+1, f+1);

			cb_bb_setpos(&bbc[curPos], r-1, f+0);
			cb_bb_setpos(&bbc[curPos], r+1, f+0);

			cb_bb_setpos(&bbc[curPos], r-1, f-1);
			cb_bb_setpos(&bbc[curPos], r+0, f-1);
			cb_bb_setpos(&bbc[curPos], r+1, f-1);

			fprintf(fLog,"INFO:bb_kingmoves:pos[%d] attacksquares[%0llx]\n",curPos,bbc[curPos]);
			dbg_cb_bb_print(bbc[curPos]);
		}
	}
}


u64 bbBlackPawnNormalMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};
u64 bbBlackPawnAttackMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};
u64 bbWhitePawnNormalMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};
u64 bbWhitePawnAttackMoves[64] = {	// Only few initialized to show concept, it is auto generated at runtime
};

void generate_bb_pawnmoves(u64 *bbwn, u64 *bbwa, u64 *bbbn, u64 *bbba)
{
	int r,f;
	int curPos;

	for(r=0; r<8; r++) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbwn[curPos] = 0;
			bbwa[curPos] = 0;
			if(r == 0) {
			} else {
				if(r == 1)
					cb_bb_setpos(&bbwn[curPos], r+2, f+0);
				cb_bb_setpos(&bbwn[curPos], r+1, f+0);

				cb_bb_setpos(&bbwa[curPos], r+1, f+1);
				cb_bb_setpos(&bbwa[curPos], r+1, f-1);
			}
			fprintf(fLog,"INFO:bb_pawnmoves:White:pos[%d] movesquares[%0llx] attacksquares[%0llx]\n",curPos,bbwn[curPos],bbwa[curPos]);
			dbg_cb_bb_print(bbwn[curPos]);
			dbg_cb_bb_print(bbwa[curPos]);
		}
	}

	for(r=7; r>=0; r--) {
		for(f=0; f<8; f++) {
			curPos = r*8+f;
			bbbn[curPos] = 0;
			bbba[curPos] = 0;
			if(r == 7) {
			} else {
				if(r == 6)
					cb_bb_setpos(&bbbn[curPos], r-2, f+0);
				cb_bb_setpos(&bbbn[curPos], r-1, f+0);

				cb_bb_setpos(&bbba[curPos], r-1, f+1);
				cb_bb_setpos(&bbba[curPos], r-1, f-1);
			}
			fprintf(fLog,"INFO:bb_pawnmoves:Black:pos[%d] movesquares[%0llx] attacksquares[%0llx]\n",curPos,bbbn[curPos],bbba[curPos]);
			dbg_cb_bb_print(bbbn[curPos]);
			dbg_cb_bb_print(bbba[curPos]);
		}
	}
}

