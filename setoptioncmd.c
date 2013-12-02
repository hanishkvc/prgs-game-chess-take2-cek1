
#define OPTTYPE_INT 0

int process_setoption(char *sCmd)
{
	char *sCheck, *sName, *sValue;
	int *pOpt;
	int optType = -1;

	//if(gUCIOption | UCIOPTION_CUSTOM_SHOWCURRMOVE)
	if(strncmp(strtok(sCmd," "),"setoption",9) != 0)
		return -1;

	if((sCheck = strtok(NULL," ")) == NULL)
		return -2;
	if(strncmp(sCheck,"name",4) != 0) {
		return -3;
	}
	
	if((sName = strtok(NULL," ")) == NULL)
		return -4;

	if((sCheck = strtok(NULL," ")) == NULL)
		return -5;
	if(strncmp(sCheck,"value",5) != 0) {
		return -6;
	}

	if((sValue = strtok(NULL," ")) == NULL)
		return -7;

	if(strncmp(sName,"depth",5) == 0) {
		pOpt = &gGameDepth;
		optType = OPTTYPE_INT;
		dbg_log(fLog,"INFO:setoption:setting depth=");
	} else 	if(strncmp(sName,"Hash",4) == 0) {
		pOpt = &gGameHash;
		optType = OPTTYPE_INT;
		dbg_log(fLog,"INFO:setoption:setting Hash=");
	}

	if(optType == OPTTYPE_INT) {
		*pOpt = strtol(sValue,NULL,0);
		dbg_log(fLog,"%d\n",*pOpt);
	}
	
	dbg_log(fLog,"INFO:setoption:depth=%d\n",gGameDepth);
	dbg_log(fLog,"INFO:setoption:Hash=%d\n",gGameHash);
	return 0;
}

