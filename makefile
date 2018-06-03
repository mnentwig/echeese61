all:
	gcc -std=c99 -I/usr/include -L/usr/lib -DWINDOWS -Wall -Ofast -g main.c -lportaudio -lwinmm
#	gcc -I/usr/local/include -L/usr/local/lib -DLINUX -Wall -O2 -g main.c -lportaudio
clean:
	rm -f a.out patch.c
	find . -name "*~" -exec rm {} \;
