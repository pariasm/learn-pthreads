all:
	gcc --std=c99 ex1.c -pthread -o ex1
	gcc --std=c99 ex2.c -pthread -o ex2
	gcc --std=c99 ex3.c -pthread -o ex3
	gcc --std=c99 ex4.c -pthread -o ex4
	gcc --std=c99 ex5.c -pthread -o ex5
	gcc --std=c99 ex6.c -pthread -o ex6

clean:
	rm ex1 ex2 ex3 ex4 ex5 ex6
