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

valgrind: mytar
  valgrind --leak-check=yes ./mtar

archive: mytar
	./mytar cf out tests/WhispyMeadow1234-Blu3Sky8734-CrimsonDawn4912-Gr33nLeaf5021-MysticBreeze9012-GoldenGrove2945/JadeEcho7562-SilverMist2981-CrimsonTide7410-AzureBloom5248-EbonHaven9375-GoldenPeak6102

extract: mytar
	./mytar xf out

clean:
	rm -f *.o out ref mytar
	clear