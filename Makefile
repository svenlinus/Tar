all: mytar

mytar: mytar.o encode.o
	gcc -o mytar -Wall -Werror mytar.o encode.o -g

mytar.o: mytar.c
	gcc -c -Wall -Werror mytar.c -g

encode.o: encode.c
	gcc -c -Wall -Werror encode.c -g

valgrind: mytar
  valgrind --leak-check=yes ./mtar

test: mytar
	./mytar cf mytar.c

clean:
	rm -f *.o out ref tarfile mytar
	clear