CC = g++
CFLAGS = -O2 -Wall
SRCS = AG.cpp
EXEC = AG

compila: AG.cpp readFile.c readFile.h
	${CC} ${CFLAGS} ${SRCS} -o ${EXEC}

executa: compila
	./${EXEC} "instancias/Completos e Assimetricos nos Arcos/br17.atsp"

todas: compila
	./${EXEC}

clean:
	rm -f *.o ${EXEC} ${EXEC}.exe
