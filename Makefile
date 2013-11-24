

cek1: cek1.c
	gcc -o cek1 cek1.c

install: cek1.c
	cp cek1 ~/local/bin/
