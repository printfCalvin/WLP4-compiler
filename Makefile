CXX = g++-6
CXXFLAGS = -g -std=c++14 -Wall -MMD -Werror=vla

EXEC = a.out
OBJECTS = tree.o error.o wlp4gen.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC} -lX11

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
