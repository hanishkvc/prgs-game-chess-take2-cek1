
DBG_MISCOPTS= -DDEBUG_EVALPRINT -DDEBUG_LEAFPRINT -DDEBUG_UNWINDSUMMARYPRINT -DDEBUG_UNWIND_SELECTION -DDEBUG_EVALSUMMARYPRINT \
		 -DDEBUG_MOVESGETCNTPRINT -DDEBUG_SMPRINT
DBG_ADDOPTS = -DDEBUG_UNWINDLOWLVLSUMMARYPRINT
DBG_OPTS=${DBG_MISCOPTS} ${DBG_ADDOPTS}
NORM_ADDOPTS = -DDEBUG_UNWINDSUMMARYPRINT -DDEBUG_UNWIND_SELECTION -DDEBUG_EVALSUMMARYPRINT -DDEBUG_SMPRINT
FAST_ADDOPTS = -DDEBUG_SMPRINT
HT_OPTS= -DUSE_HASHTABLE -DDEBUG_HTPRINT
#HT_OPTS= -DUSE_HASHTABLE

cek1: cek1.c cek1.h moves.c evals.c generate_movebbs.c positioncmd.c positionhash.c
	echo -n "#define PRG_VERSION \"CEK1 v" > __MAKE__PREFIX.h
	date +%Y%m%d_%H%M\" > __MAKE__DT.h
	cat __MAKE__PREFIX.h __MAKE__DT.h > makeheader.h
	gcc -Wall -O2 -o cek1_pw cek1.c -D_GNU_SOURCE
	gcc -Wall -g  -o cek1_pwxg cek1.c -D_GNU_SOURCE ${DBG_OPTS}
	gcc -Wall -O2 -o cek1 cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${NORM_ADDOPTS}
	gcc -Wall -O2 -o cek1_fast cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS}
	gcc -Wall -g  -o cek1_xg cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${DBG_OPTS}
	gcc -Wall -O2 -o cek1_ht cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS}
	gcc -Wall -g  -pg -o cek1_htxg cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS} ${DBG_OPTS}

install-base: cek1
	cp cek1 ~/local/bin/
	cp cek1_fast ~/local/bin/
	cp cek1_ht ~/local/bin/
	cp cek1_htxg ~/local/bin/
	touch ~/cek1.log
	touch ~/cek1_main.log

install: install-base
	ln -s ~/cek1.log /tmp/cek1.log	|| /bin/true

installn: install-base
	ln -s /dev/null /tmp/cek1.log	|| /bin/true

clean:
	rm __MAKE__*	|| /bin/true
	rm ./cek1_pw	|| /bin/true
	rm ./cek1_pwxg	|| /bin/true
	rm ./cek1	|| /bin/true
	rm ./cek1_fast	|| /bin/true
	rm ./cek1_xg	|| /bin/true
	rm ./cek1_ht	|| /bin/true
	rm ./cek1_htxg	|| /bin/true

clean-log:
	rm ~/cek1.log		|| /bin/true
	rm ~/cek1_main.log	|| /bin/true

clean-all: clean clean-log
	rm /tmp/cek1.log	|| /bin/true
	
