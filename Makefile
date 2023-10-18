VPATH+=inc:src

vector_3d.o: vector_3d.c vector_3d.h
	gcc -Wall -c $@ $< -Iinc

.PHONY: clean
clean:
	rm -rf vector_3d*