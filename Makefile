

cek1: cek1.c
	gcc -g -o cek1 cek1.c

install: cek1.c
	cp cek1 ~/local/bin/
	ln -s ~/cek1.log /tmp/cek1.log
	touch ~/cek1.log

clean:
	rm ./cek1
	rm ~/cek1.log

clean-all: clean
	rm /tmp/cek1.log
	
