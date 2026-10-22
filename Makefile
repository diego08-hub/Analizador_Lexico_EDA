CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

lexer: src/main.cpp src/lexer.cpp src/lexer.h src/token.h src/keywords.h
	$(CXX) $(CXXFLAGS) src/main.cpp src/lexer.cpp -o lexer

clean:
	rm -f lexer
