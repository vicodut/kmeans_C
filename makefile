Sortie: main.o 
	gcc -g $^ -o $@  -pedantic -std=c99 -lm

main.o: main.c 
	gcc -g -c $< -o $@  -pedantic -std=c99 -lm

