CFLAGS = -O3

lisp: prefix.txt cli.o environment.o list.o main.o parser.o primitives.o printout.o util.o marksweep.o eval-apply-tco.o
	$(CC) $(CFLAGS) *.o -o lisp	

prefix.txt: gen-prefix.py preprefix.txt
	python gen-prefix.py >prefix.txt
	cat preprefix.txt >>prefix.txt

clean:
	rm -f *.o prefix.txt lisp
