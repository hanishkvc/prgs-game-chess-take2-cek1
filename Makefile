
BASE4A_OPTS= -D_GNU_SOURCE -DENABLE_SM
BASE_OPTS= -DUSE_MOVELISTEVALAGING
DBG_MISCOPTS= -DDEBUG_EVALPRINT -DDEBUG_LEAFPRINT -DDEBUG_UNWINDSUMMARYPRINT -DDEBUG_UNWIND_SELECTION -DDEBUG_EVALSUMMARYPRINT \
		 -DDEBUG_MOVESGETCNTPRINT -DDEBUG_SMPRINT
DBG_ADDOPTS = -DDEBUG_UNWINDLOWLVLSUMMARYPRINT
DBG_OPTS= ${DBG_MISCOPTS} ${DBG_ADDOPTS}
NORM_ADDOPTS = ${BASE_OPTS} -DDEBUG_UNWINDSUMMARYPRINT -DDEBUG_UNWIND_SELECTION -DDEBUG_EVALSUMMARYPRINT -DDEBUG_SMPRINT
FAST_ADDOPTS = ${BASE_OPTS} -DDEBUG_SMPRINT
FASTAB_OPTS= -DDEBUG_SMPRINT -DUSE_ABPRUNING
#HT_OPTS= -DUSE_HASHTABLE -DDEBUG_HTPRINT -DDEBUG_HTSUMMARYPRINT
HT_OPTS= ${BASE_OPTS} -DUSE_HASHTABLE -DDEBUG_HTSUMMARYPRINT

cek1: cek1.c cek1.h moves.c evals.c generate_movebbs.c positioncmd.c positionhash.c
	echo -n "#define PRG_VERSION \"CEK1 v" > __MAKE__PREFIX.h
	date +%Y%m%d_%H%M\" > __MAKE__DT.h
	cat __MAKE__PREFIX.h __MAKE__DT.h > makeheader.h
	gcc -Wall -O2 -o cek1_pw cek1.c ${BASE4A_OPTS}
	gcc -Wall -g  -o cek1_pwxg cek1.c ${BASE4A_OPTS} ${DBG_OPTS} ${BASE_OPTS}
	gcc -Wall -O2 -o cek1 cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${NORM_ADDOPTS}
	gcc -Wall -g  -o cek1_xg cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${DBG_OPTS} ${BASE_OPTS}
	gcc -Wall -O2 -o cek1_fast cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS}
	gcc -Wall -O2 -o cek1_fastmt cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS} -DUSE_THREAD -lpthread
	gcc -Wall -O2 -o cek1_fastmtab cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FASTAB_OPTS} -DUSE_THREAD -lpthread
	gcc -Wall -g -o cek1_fastmtabxg cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${DBG_OPTS} ${FASTAB_OPTS} -DUSE_THREAD -lpthread
	gcc -Wall -O2 -o cek1_fastmtbm cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS} -DUSE_THREAD -lpthread -DUSE_BMPRUNING
	gcc -Wall -g -o cek1_fastmtbmxg cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS} -DUSE_THREAD -lpthread -DUSE_BMPRUNING
	gcc -Wall -g -o cek1_fastmtxg cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${FAST_ADDOPTS} -DUSE_THREAD -lpthread
	gcc -Wall -O2 -o cek1_ht cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS}
	gcc -Wall -g  -pg -o cek1_htxg cek1.c ${BASE4A_OPTS} -DCORRECTVALFOR_SIDETOMOVE ${HT_OPTS} ${DBG_OPTS}


install-base: cek1
	cp cek1 ~/local/bin/		|| /bin/true
	cp cek1_xg ~/local/bin/		|| /bin/true
	cp cek1_fast ~/local/bin/	|| /bin/true
	cp cek1_fastmt ~/local/bin/	|| /bin/true
	cp cek1_fastmtab ~/local/bin/	|| /bin/true
	cp cek1_fastmtabxg ~/local/bin/	|| /bin/true
	cp cek1_fastmtbm ~/local/bin/	|| /bin/true
	cp cek1_fastmtbmxg ~/local/bin/	|| /bin/true
	cp cek1_fastmtxg ~/local/bin/	|| /bin/true
	cp cek1_ht ~/local/bin/		|| /bin/true
	cp cek1_htxg ~/local/bin/	|| /bin/true
	touch ~/cek1.log		|| /bin/true
	touch ~/cek1_main.log		|| /bin/true

install: install-base
	ln -s ~/cek1.log /tmp/cek1.log	|| /bin/true

installn: install-base
	ln -s /dev/null /tmp/cek1.log	|| /bin/true

clean:
	rm __MAKE__*		|| /bin/true
	rm makeheader.h		|| /bin/true
	rm ./cek1_pw		|| /bin/true
	rm ./cek1_pwxg		|| /bin/true
	rm ./cek1		|| /bin/true
	rm ./cek1_xg		|| /bin/true
	rm ./cek1_fast		|| /bin/true
	rm ./cek1_fastmt	|| /bin/true
	rm ./cek1_fastmtxg	|| /bin/true
	rm ./cek1_fastmtab	|| /bin/true
	rm ./cek1_fastmtabxg	|| /bin/true
	rm ./cek1_fastmtbm	|| /bin/true
	rm ./cek1_fastmtbmxg	|| /bin/true
	rm ./cek1_ht		|| /bin/true
	rm ./cek1_htxg		|| /bin/true

clean-log:
	rm ~/cek1.log		|| /bin/true
	rm ~/cek1_main.log	|| /bin/true

clean-all: clean clean-log
	rm /tmp/cek1.log	|| /bin/true
	

test5:
	~/local/bin/cek1nonthread_fast < TestCases/AutoTest1/test_5.cmd > /tmp/tnon5.log || /bin/true
	~/local/bin/cek1_fastmt < TestCases/AutoTest1/test_5.cmd > /tmp/tthr5.log || /bin/true
	sed -e 's/depth 4.*pv/depth xxx pv/' /tmp/tnon5.log > /tmp/tnon5.simp.log
	sed -e 's/depth 4.*pv/depth xxx pv/' /tmp/tthr5.log > /tmp/tthr5.simp.log
	diff -ub /tmp/tnon5.simp.log /tmp/tthr5.simp.log | less

test6:
	~/local/bin/cek1nonthread_fast < TestCases/AutoTest1/test_6.cmd > /tmp/tnon6.log || /bin/true
	~/local/bin/cek1_fastmt < TestCases/AutoTest1/test_6.cmd > /tmp/tthr6.log || /bin/true
	sed -e 's/depth 5.*pv/depth xxx pv/' /tmp/tnon6.log > /tmp/tnon6.simp.log
	sed -e 's/depth 5.*pv/depth xxx pv/' /tmp/tthr6.log > /tmp/tthr6.simp.log
	diff -ub /tmp/tnon6.simp.log /tmp/tthr6.simp.log | less

clean-test:
	rm tnon5.log || /bin/true
	rm tthr5.log || /bin/true

