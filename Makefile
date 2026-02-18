


chip8: main.cpp
	g++ -std=c++20 -Wall main.cpp -o chip8 -lSDL2


clean:
	rm *.o chip8