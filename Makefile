all: mytar

mytar: mytar.o encode.o decode.o integer.o
	gcc -o mytar -Wall -Werror mytar.o encode.o decode.o integer.o -g

mytar.o: mytar.c
	gcc -c -Wall -Werror mytar.c -g

encode.o: encode.c
	gcc -c -Wall -Werror encode.c -g

decode.o: decode.c
	gcc -c -Wall -Werror decode.c -g

integer.o: integer.c
	gcc -c -Wall -Werror integer.c -g

archive: mytar
	./mytar cf out tests

extract: mytar
	./mytar xf out

list: mytar
	./mytar tf out

clean:
	rm -f *.o out ref mytar
	clear