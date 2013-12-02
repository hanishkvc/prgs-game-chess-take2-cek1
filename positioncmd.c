
int process_position(struct cb *cbC, char *sCmd)
{
	char *fenStr;
	char *fenSTM;
	int r,f;
	char *fenMisc;
	char *fenMoveNum;

	if(strncmp(strtok(sCmd," "),"position",8) != 0)
		return -1;
	if(strncmp(strtok(NULL," "),"fen",3) != 0)
		return -2;
	fenStr = strtok(NULL," ");
	if(fenStr == NULL)
		return -3;
	if((fenSTM = strtok(NULL," ")) == NULL)
		return -4;
	if((fenMisc = strtok(NULL,"-")) == NULL)
		return -5;
	if((fenMisc = strtok(NULL," ")) == NULL)
		return -6;
	if((fenMoveNum = strtok(NULL," ")) == NULL)
		return -7;

	gStartMoveNum = strtol(fenMoveNum,NULL,10);

	bzero(cbC,sizeof(struct cb));

	if((fenSTM[0] == 'w') || (fenSTM[0] == 'W'))
		cbC->sideToMove=STM_WHITE;
	else
		cbC->sideToMove=STM_BLACK;


	r = 7; f = 0;
	while(*fenStr != '\0') {
		dbg_log(fLog,"INFO:pp:val[%c]\n",*fenStr);
		if(fenStr[0] == 'p') {
			cb_bb_setpos(&(cbC->bp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'P') {
			cb_bb_setpos(&(cbC->wp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'r') {
			cb_bb_setpos(&(cbC->br),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'R') {
			cb_bb_setpos(&(cbC->wr),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'n') {
			cb_bb_setpos(&(cbC->bn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'N') {
			cb_bb_setpos(&(cbC->wn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'b') {
			cb_bb_setpos(&(cbC->bb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'B') {
			cb_bb_setpos(&(cbC->wb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'k') {
			cb_bb_setpos(&(cbC->bk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'K') {
			cb_bb_setpos(&(cbC->wk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'q') {
			cb_bb_setpos(&(cbC->bq),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'Q') {
			cb_bb_setpos(&(cbC->wq),r,f);
			f += 1; 
		}
		if(fenStr[0] == '/') {
			dbg_log(fLog,"INFO:pp:Next Row\n");
			r -= 1;
			f = 0;
		}
		if(fenStr[0] == '1') {
			f += 1; 
		}
		if(fenStr[0] == '2') {
			f += 2; 
		}
		if(fenStr[0] == '3') {
			f += 3; 
		}
		if(fenStr[0] == '4') {
			f += 4; 
		}
		if(fenStr[0] == '5') {
			f += 5; 
		}
		if(fenStr[0] == '6') {
			f += 6; 
		}
		if(fenStr[0] == '7') {
			f += 7; 
		}
		if(fenStr[0] == ' ') {
			dbg_log(fLog,"DEBUG:pp: strtok strange\n");
			break;
		}
		if((f < 0) || (f > 8)) {	// f checked against 8 instead of 7, as f increment can occur to 8
			dbg_log(fLog,"WARN:pp: file went beyond a-h\n");
		}
		if((r < 0) || (r > 7)) {
			dbg_log(fLog,"WARN:pp: rank went beyond 1-8\n");
		}
		fenStr++;
	}
	cb_print(cbC);
	return 0;
}

