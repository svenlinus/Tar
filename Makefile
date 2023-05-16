all: mytar

mytar.o: mytar.c
	gcc -c -Wall -Werror mytar.c -g

mytar: mytar.o
	gcc -o mytar -Wall -Werror mytar.o -g

valgrind: mytar
  valgrind --leak-check=yes ./mtar

test: mytar
	./mytar cf tarfile

clean:
	rm -f *.o out ref tarfile
	clear