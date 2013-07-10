CC = c99 -O3

lisp: apply.o cli.o environment.o eval.o list.o main.o parser.o primitives.o printout.o util.o 
	c99 *.o -o lisp	

clean:
	rm *.o lisp
