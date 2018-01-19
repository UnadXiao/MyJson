test.exe: leptjson.o test.o
	gcc leptjson.o test.o -o test.exe
	test.exe

leptjson.o: leptjson.c leptjson.h
	gcc leptjson.c -c

test.o: test.c
	gcc test.c -c