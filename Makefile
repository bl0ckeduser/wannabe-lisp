CC = c99 -O3

lisp: cli.o environment.o list.o main.o parser.o primitives.o printout.o util.o marksweep.o eval-apply-tco.o
	c99 *.o -o lisp	

clean:
	rm *.o lisp
