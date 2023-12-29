default: fsdump.o
	gcc -o fsdump fsdump.c -Wall -Wextra

clean:
	rm -rf fsdump *.o