defualt: Client Server

Client:
	gcc -o lab6C.run ./lab6C.c -lm -lrt -lpthread
Server:
	gcc -o lab6S.run ./lab6S.c -lm -lrt -lpthread

test:
	reset
	echo ===CLIENT===
	echo gcc:
	gcc -o lab6C.run ./lab6C.c -lm -lpthread -lrt -fsanitize=address
	echo cppcheck:
	cppcheck --enable=all --inconclusive --std=posix lab6C.c
	echo codepatch.pl:
	/home/batya/linux/scripts/checkpatch.pl -f /home/batya/labs/lab6/lab6C.c
	echo ===SERVER===
	echo gcc:
	gcc -o lab6S.run ./lab6S.c -lm -lpthread -lrt -fsanitize=address
	echo cppcheck:
	cppcheck --enable=all --inconclusive --std=posix lab6S.c
	echo codepatch.pl:
	/home/batya/linux/scripts/checkpatch.pl -f /home/batya/labs/lab6/lab6S.c


