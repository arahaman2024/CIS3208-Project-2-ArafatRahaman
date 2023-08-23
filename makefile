shell: shell.o helpers.o
	gcc -o shell shell.o helpers.o -Wall -Werror

shell.o: shell.c
	gcc -c shell.c -Wall -Werror

helpers.o: helpers.c
	gcc -c helpers.c -Wall -Werror

clean:
	rm -rf *.o