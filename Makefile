json.o: json.c json.h
	gcc -Wall json.c
clean:
	rm *.o

