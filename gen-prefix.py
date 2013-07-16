import sys

ch = ['stop', 'a', 'd']
ch2 = [' ', '(car ', '(cdr ']
n = 0

QTY = 5

print ";; Boolean literals "
print "(define true (= 1 1))"
print "(define false (= 1 0))"
print "(define #t true)"
print "(define #f false)"
print ""

print ";; Streams"
print ";; See mitpress.mit.edu/sicp/full-text/book/book-Z-H-24.html"
print "(define (force u) (u))"
print "(define stream-car car)"
print "(define (stream-cdr s) (force (cdr s)))"
print "(define stream-null? null?)"
print "(define the-empty-stream '())"
print ""

print ";; Composite car/cdr primitives"

tab = []
for i in range(QTY):
	tab.append(0)

# skip car, cdr
tab[0] = 2

while tab[QTY - 1] == 0:
	# bump and carry
	tab[0] += 1
	for i in range(len(tab)):
		if tab[i] > 2:
			tab[i] = 1
			tab[i + 1] += 1
		else:
			break
		
	# print ' (define (c...r x) '
	sys.stdout.write("(define (c")
	for i in range(len(tab)):
		if tab[i] == 0:
			break
		sys.stdout.write(ch[tab[i]])

	sys.stdout.write("r x) ")

	# print (cxr (cxr ... 
	paren = 0
	for i in range(len(tab)):
		if tab[i] == 0:
			break
		sys.stdout.write(ch2[tab[i]])
		paren += 1

	sys.stdout.write("x")
	for i in range(paren):
		sys.stdout.write(")")
	print ") "

