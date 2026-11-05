CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

lexer: src/main.cpp src/lexer.cpp src/lexer.h src/token.h src/keywords.h
	$(CXX) $(CXXFLAGS) src/main.cpp src/lexer.cpp -o lexer

test_lexer: tests/test_lexer.cpp src/lexer.cpp src/lexer.h src/token.h src/keywords.h
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp src/lexer.cpp -o test_lexer

test_grammar: tests/test_grammar.cpp parser_ll1/grammar.cpp parser_ll1/grammar.h
	$(CXX) $(CXXFLAGS) tests/test_grammar.cpp parser_ll1/grammar.cpp -o test_grammar

test: test_lexer test_grammar
	./test_lexer docs/Reporte_Pruebas.md
	./test_grammar

clean:
	rm -f lexer test_lexer test_grammar

.PHONY: test clean
