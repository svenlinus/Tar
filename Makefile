all: mytar

mytar: mytar.o create.o extract.o integer.o
	gcc -o mytar -Wall -Werror mytar.o create.o extract.o integer.o -g

mytar.o: mytar.c
	gcc -c -Wall -Werror mytar.c -g

create.o: create.c
	gcc -c -Wall -Werror create.c -g

extract.o: extract.c
	gcc -c -Wall -Werror extract.c -g

integer.o: integer.c
	gcc -c -Wall -Werror integer.c -g

archive: mytar
	./mytar cvf out parent

extract: mytar
	./mytar xvf out

list: mytar
	./mytar tvf out

clean:
	rm -f *.o out ref mytar
	clear