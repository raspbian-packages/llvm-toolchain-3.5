all: clean
		$(CC) -arch i386 -g -c a.c
		ar -q liba-i386.a a.o
		ranlib liba-i386.a
		$(CC) -arch x86_64 -g -c a.c
		ar -q liba-x86_64.a a.o
		ranlib liba-x86_64.a
		lipo -create -output liba.a liba-i386.a liba-x86_64.a
		$(CC) -g -c main.c
		$(CC) -o a.out main.o -L. -la

clean:
		rm -rf a.o a.out liba-i386.a liba-x86_64.a liba.a *un~ .*un~ main.o *.pyc

