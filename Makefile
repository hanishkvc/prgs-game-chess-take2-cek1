

cek1: cek1.c moves.c evals.c generate_movebbs.c
	echo -n "#define PRG_VERSION \"CEK1 v" > __MAKE__PREFIX.h
	date +%Y%m%d_%H%M\" > __MAKE__DT.h
	cat __MAKE__PREFIX.h __MAKE__DT.h > makeheader.h
	gcc -g -o cek1_pw cek1.c
	gcc -g -o cek1 cek1.c -DCORRECTVALFOR_SIDETOMOVE

install: cek1.c
	cp cek1 ~/local/bin/
	touch ~/cek1.log
	ln -s ~/cek1.log /tmp/cek1.log || /bin/true

clean:
	rm __MAKE__*
	rm ./cek1
	rm ./cek1_pw
	rm ~/cek1.log

clean-all: clean
	rm /tmp/cek1.log
	
