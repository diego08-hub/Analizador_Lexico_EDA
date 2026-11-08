CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Fuentes del lexer (Práctica 4)
LEXER_SRCS = src/lexer.cpp
LEXER_HDRS = src/lexer.h src/token.h src/keywords.h

# Fuentes del analizador sintáctico LL(1) (Práctica 5)
PARSER_SRCS = parser_ll1/parse_table.cpp parser_ll1/parser.cpp
PARSER_HDRS = parser_ll1/types.h parser_ll1/parse_table.h parser_ll1/tracer.h parser_ll1/parser.h

all: lexer parser_cli test_lexer test_parser

lexer: src/main.cpp $(LEXER_SRCS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(LEXER_SRCS) -o lexer

parser_cli: parser_ll1/main.cpp $(PARSER_SRCS) $(LEXER_SRCS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) parser_ll1/main.cpp $(PARSER_SRCS) $(LEXER_SRCS) -o parser_cli

test_lexer: tests/test_lexer.cpp $(LEXER_SRCS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp $(LEXER_SRCS) -o test_lexer

test_parser: tests/test_parser.cpp $(PARSER_SRCS) $(LEXER_SRCS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_parser.cpp $(PARSER_SRCS) $(LEXER_SRCS) -o test_parser

test: test_lexer test_parser
	./test_lexer docs/Reporte_Pruebas.md
	./test_parser docs/Reporte_Pruebas_Parser.md

clean:
	rm -f lexer parser_cli test_lexer test_parser

.PHONY: all test clean
