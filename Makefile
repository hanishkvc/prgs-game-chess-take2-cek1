

cek1: cek1.c moves.c evals.c generate_movebbs.c positioncmd.c
	echo -n "#define PRG_VERSION \"CEK1 v" > __MAKE__PREFIX.h
	date +%Y%m%d_%H%M\" > __MAKE__DT.h
	cat __MAKE__PREFIX.h __MAKE__DT.h > makeheader.h
	gcc -Wall -O2 -o cek1_pw cek1.c -D_GNU_SOURCE
	gcc -Wall -g  -o cek1_pwxg cek1.c -D_GNU_SOURCE
	gcc -Wall -O2 -o cek1 cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE
	gcc -Wall -O2 -o cek1_ht cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE -DUSE_HASHTABLE
	gcc -Wall -g  -o cek1_xg cek1.c -D_GNU_SOURCE -DCORRECTVALFOR_SIDETOMOVE

install: cek1.c
	cp cek1 ~/local/bin/
	cp cek1_ht ~/local/bin/
	touch ~/cek1.log
	ln -s ~/cek1.log /tmp/cek1.log	|| /bin/true

clean:
	rm __MAKE__*	|| /bin/true
	rm ./cek1_pw	|| /bin/true
	rm ./cek1_pwxg	|| /bin/true
	rm ./cek1	|| /bin/true
	rm ./cek1_ht	|| /bin/true
	rm ./cek1_xg	|| /bin/true
	rm ~/cek1.log	|| /bin/true

clean-all: clean
	rm /tmp/cek1.log	|| /bin/true
	
