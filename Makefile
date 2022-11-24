myls: myls.o
	gcc myls.o -o myls

myls.o: myls.c
	gcc -c myls.c


clean:
	rm myls.o

install:
	cp myls /usr/bin

uninstall:
	rm /usr/bin/myls

