# test makefiles
test: main.o
	g++ -Wall -ggdb main.o -o test
main.o: main.cpp
	g++ -Wall -ggdb -c main.cpp

# all
all: serverM.o serverA.o serverB.o serverC.o clientA.o clientB.o
	g++ -Wall -ggdb serverM.o serverA.o serverB.o serverC.o clientA.o clientB.o -o all

# server M
serverM: serverM.o
	g++ -Wall -ggdb serverM.o -o serverM
serverM.o: serverM.cpp
	g++ -Wall -ggdb -c serverM.cpp
# server A
serverA: serverA.o
	g++ -Wall -ggdb serverA.o -o serverA
serverA.o: serverA.cpp
	g++ -Wall -ggdb -c serverA.cpp

# server B
serverB: serverB.o
	g++ -Wall -ggdb serverB.o -o serverB
serverB.o: serverB.cpp
	g++ -Wall -ggdb -c serverB.cpp

# server C
serverC: serverC.o
	g++ -Wall -ggdb serverC.o -o serverC
serverC.o: serverC.cpp
	g++ -Wall -ggdb -c serverC.cpp

#client A
clientA: clientA.o
	g++ -Wall -ggdb clientA.o -o clientA
clientA.o: clientA.cpp
	g++ -Wall -ggdb -c clientA.cpp

#client B
clientB: clientB.o
	g++ -Wall -ggdb clientB.o -o clientB
clientB.o: clientB.cpp
	g++ -Wall -ggdb -c clientB.cpp

#clean
clean:
	rm serverM.o serverA.o serverB.o serverC.o clientA.o clientB.o