
void process_debug_diagattack()
{
	int r,f;
	struct cb cbM;

	for(r = 0; r < 8; r++) {
		for(f = 0; f < 8; f++) {
			dbg_log(fLog,"CHECK:DiagAttackDBG:f[%d]r[%d]\n",f,r);
			evalhlpr_diagattack(&cbM,r*8+f,32,-1);
		}
	}
	exit(1);
}

void process_debug(char *sCmd)
{
	if(strncmp(sCmd,"debug diagattack",10) == 0) {
		process_debug_diagattack();
	}
}

