CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

lexer: src/main.cpp src/lexer.cpp src/lexer.h src/token.h src/keywords.h
	$(CXX) $(CXXFLAGS) src/main.cpp src/lexer.cpp -o lexer

test_lexer: tests/test_lexer.cpp src/lexer.cpp src/lexer.h src/token.h src/keywords.h
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp src/lexer.cpp -o test_lexer

test: test_lexer
	./test_lexer

clean:
	rm -f lexer test_lexer

.PHONY: test clean
