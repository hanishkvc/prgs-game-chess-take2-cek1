
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
		}
	}
}

