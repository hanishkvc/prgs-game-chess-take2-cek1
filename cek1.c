#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>

#define STM_WHITE 'w'
#define STM_BLACK 'b'

typedef unsigned long long u64;
FILE *fLog;

struct cb {
	u64 wk,wq,wr,wb,wn,wp;
	u64 bk,bq,br,bb,bn,bp;
	char sideToMove;
} gb;


#define send_resp_ex(sBuffer,sSize,...) snprintf(sBuffer,sSize,__VA_ARGS__); send_resp(sBuffer);

void send_resp(char *sBuf)
{
	printf("%s",sBuf);
	fflush(stdout);
	fprintf(fLog,"SENT:%s",sBuf);
	fflush(fLog);
}

int cb_bb_setpos(u64 *bb, int r, int f)
{
	int off = r*8+f;
	u64 pos = 0x1;

	if((f < 0) || (f > 7)) {
		fprintf(fLog,"ERROR:cb_bb_setpos: file went beyond a-h :%d\n",f);
		exit(100);
	}
	if((r < 0) || (r > 7)) {
		fprintf(fLog,"ERROR:cb_bb_setpos: rank went beyond 1-8 :%d\n",r);
		exit(100);
	}
	pos <<= off;
	*bb |= pos;
	fprintf(fLog,"INFO:cb_bb_setpos: f%d_r%d\n",f,r);
}

int cb_print(struct cb *mcb)
{
	int off;
	u64 pos;
	int r,f;

	fprintf(fLog,"INFO:cb_print:%c to move\n",mcb->sideToMove);
	for(r = 7; r >= 0; r--) {
		for(f = 0; f < 8; f++) {
			off = r*8+f;
			pos = 0x1; pos <<= off;
			if(mcb->wk & pos)
				fprintf(fLog,"K");
			else if(mcb->wq & pos)
				fprintf(fLog,"Q");
			else if(mcb->wr & pos)
				fprintf(fLog,"R");
			else if(mcb->wn & pos)
				fprintf(fLog,"N");
			else if(mcb->wb & pos)
				fprintf(fLog,"B");
			else if(mcb->wp & pos)
				fprintf(fLog,"P");
			else if(mcb->bk & pos)
				fprintf(fLog,"k");
			else if(mcb->bq & pos)
				fprintf(fLog,"q");
			else if(mcb->br & pos)
				fprintf(fLog,"r");
			else if(mcb->bn & pos)
				fprintf(fLog,"n");
			else if(mcb->bb & pos)
				fprintf(fLog,"b");
			else if(mcb->bp & pos)
				fprintf(fLog,"p");
			else
				fprintf(fLog,"*");
		}
		fprintf(fLog,"\n");
	}
}

int process_setoption(char *sCmd)
{
}

int cb_eval_mat(struct cb *mcb)
{
	int val = 0;
	int valB = 0;
	int valW = 0;

	valB += __builtin_popcountll(mcb->bp)*100;
	valB += __builtin_popcountll(mcb->br)*500;
	valB += __builtin_popcountll(mcb->bn)*300;
	valB += __builtin_popcountll(mcb->bb)*300;
	valB += __builtin_popcountll(mcb->bq)*900;

	valW += __builtin_popcountll(mcb->wp)*100;
	valW += __builtin_popcountll(mcb->wr)*500;
	valW += __builtin_popcountll(mcb->wn)*300;
	valW += __builtin_popcountll(mcb->wb)*300;
	valW += __builtin_popcountll(mcb->wq)*900;

	val = valW-valB;
	return val;
}

int cb_eval(struct cb *mcb)
{
	int val = 0;
	val += cb_eval_mat(mcb);
	// eval_threats
	// eval_misc
	return val;
}

#define CORRECTVALFOR_SIDETOMOVE

int cb_findbest(struct cb *mcb, int curDepth, int maxDepth, int secs)
{
	int val;
	char sBuf[1024];

	val = cb_eval(mcb);
#ifdef CORRECTVALFOR_SIDETOMOVE
	if(mcb->sideToMove != STM_WHITE)
		val = -1*val;
#endif
	send_resp_ex(sBuf,1024,"info score cp %d depth %d nodes %d time %d pv %s\n",val,curDepth,0,0,NULL);
	curDepth += 1;
	// Find possible moves 
	return val; // ToThink, all info in above send_resp info to be sent
}

int process_go(char *sCmd)
{
	cb_findbest(&gb,0,50,0);
	return 0;
}

int process_position(char *sCmd)
{
	char *fenStr;
	char *fenSTM;
	int r,f;

	if(strncmp(strtok(sCmd," "),"position",8) != 0)
		return -1;
	if(strncmp(strtok(NULL," "),"fen",3) != 0)
		return -2;
	fenStr = strtok(NULL," ");
	if(fenStr == NULL)
		return -3;
	if((fenSTM = strtok(NULL," ")) == NULL)
		return -4;

	bzero(&gb,sizeof(gb));

	if((fenSTM[0] == 'w') || (fenSTM[0] == 'W'))
		gb.sideToMove=STM_WHITE;
	else
		gb.sideToMove=STM_BLACK;


	r = 7; f = 0;
	while(*fenStr != (char)NULL) {
		fprintf(fLog,"INFO:pp:val[%c]\n",*fenStr);
		if(fenStr[0] == 'p') {
			cb_bb_setpos(&(gb.bp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'P') {
			cb_bb_setpos(&(gb.wp),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'r') {
			cb_bb_setpos(&(gb.br),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'R') {
			cb_bb_setpos(&(gb.wr),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'n') {
			cb_bb_setpos(&(gb.bn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'N') {
			cb_bb_setpos(&(gb.wn),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'b') {
			cb_bb_setpos(&(gb.bb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'B') {
			cb_bb_setpos(&(gb.wb),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'k') {
			cb_bb_setpos(&(gb.bk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'K') {
			cb_bb_setpos(&(gb.wk),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'q') {
			cb_bb_setpos(&(gb.bq),r,f);
			f += 1; 
		}
		if(fenStr[0] == 'Q') {
			cb_bb_setpos(&(gb.wq),r,f);
			f += 1; 
		}
		if(fenStr[0] == '/') {
			fprintf(fLog,"INFO:pp:Next Row\n");
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
			fprintf(fLog,"DEBUG:pp: strtok strange\n");
			break;
		}
		if((f < 0) || (f > 8)) {	// f checked against 8 instead of 7, as f increment can occur to 8
			fprintf(fLog,"WARN:pp: file went beyond a-h\n");
		}
		if((r < 0) || (r > 7)) {
			fprintf(fLog,"WARN:pp: rank went beyond 1-8\n");
		}
		fenStr++;
	}
	cb_print(&gb);
	return 0;
}

int process_uci()
{

	char sCmdBuf[1024];
	char *sCmd;

	sCmd = fgets(sCmdBuf, 1024, stdin);
	fprintf(fLog,"GOT:%s\n",sCmd);
	fflush(fLog);

	if(strncmp(sCmd,"uci",3) == 0) {
		send_resp("id name cek1 20131125_0136\n");
		send_resp("id author hkvc\n");
		send_resp("option name Ponder type check default true\n");
		send_resp("option name Hash type spin default 1 min 1 max 100\n");
		send_resp("uciok\n");
	}
	if(strncmp(sCmd,"isready",7) == 0) {
		send_resp("readyok\n");
	}
	if(strncmp(sCmd,"position",8) == 0) {
		process_position(sCmd);
	}
	if(strncmp(sCmd,"setoption",9) == 0) {
		process_setoption(sCmd);
	}
	if(strncmp(sCmd,"go",2) == 0) {
		process_go(sCmd);
	}
	if(strncmp(sCmd,"quit",4) == 0) {
		fprintf(fLog,"QUITING\n");
		exit(2);
	}
	fflush(fLog);
}

int run()
{
	fd_set fdIN;

	setbuf(stdin,NULL);

	FD_ZERO(&fdIN);
	FD_SET(0,&fdIN);
	while(1) {
		if(select(10,&fdIN,NULL,NULL,NULL) == 1)
			process_uci();
	}

	return 0;
}

int main(int argc, char **argv)
{

	if((fLog=fopen("/tmp/cek1.log","a+")) == NULL)
		return 1;
	puts("CEK1 v20131125_0137\n");
	run();
	fclose(fLog);
	return 0;
}

