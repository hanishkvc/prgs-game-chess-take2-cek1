

cek1: cek1.c moves.c evals.c generate_movebbs.c
	gcc -g -o cek1 cek1.c

install: cek1.c
	cp cek1 ~/local/bin/
	touch ~/cek1.log
	ln -s ~/cek1.log /tmp/cek1.log || /bin/true

clean:
	rm ./cek1
	rm ~/cek1.log

clean-all: clean
	rm /tmp/cek1.log
	
