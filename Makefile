
DBG_MISCOPTS= -DDEBUG_EVALPRINT -DDEBUG_LEAFPRINT -DDEBUG_UNWINDPRINT -DDEBUG_UNWIND_SELECTION -DDEBUG_EVALSUMMARYPRINT
HT_OPTS= -DUSE_HASHTABLE -DDEBUG_HTPRINT

cek1: cek1.c cek1.h moves.c evals.c generate_movebbs.c positioncmd.c positionhash.c
	echo -n "#define PRG_VERSION \"CEK1 v" > __MAKE__PREFIX.h
	date +%Y%m%d_%H%M\" > __MAKE__DT.h
	cat __MAKE__PREFIX.h __MAKE__DT.h > makeheader.h
	gcc -Wall -O2 -o cek1_pw cek1.c -D_GNU_SOURCE
	gcc -Wall -g  -o cek1_pwxg cek1.c -D_GNU_SOURCE ${DBG_MISCOPTS}
	gcc -Wall -O2 -o cek1 cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE
	gcc -Wall -g  -o cek1_xg cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${DBG_MISCOPTS}
	gcc -Wall -O2 -o cek1_ht cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS}
	gcc -Wall -g  -pg -o cek1_htxg cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS} ${DBG_MISCOPTS}

install-base: cek1
	cp cek1 ~/local/bin/
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
	rm ./cek1_xg	|| /bin/true
	rm ./cek1_ht	|| /bin/true
	rm ./cek1_htxg	|| /bin/true

clean-log:
	rm ~/cek1.log		|| /bin/true
	rm ~/cek1_main.log	|| /bin/true

clean-all: clean clean-log
	rm /tmp/cek1.log	|| /bin/true
	
