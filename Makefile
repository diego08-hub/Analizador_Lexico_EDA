CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Fuentes del lexer (Práctica 4)
LEXER_SRCS = src/lexer.cpp
LEXER_HDRS = src/lexer.h src/token.h src/keywords.h

# Fuentes del analizador sintáctico LL(1) (Práctica 5)
PARSER_SRCS = parser_ll1/parse_table.cpp parser_ll1/parser.cpp
PARSER_HDRS = parser_ll1/types.h parser_ll1/parse_table.h parser_ll1/tracer.h parser_ll1/parser.h

# Fuentes del módulo de gramática y cálculo FIRST/FOLLOW
GRAMMAR_SRCS = parser_ll1/grammar.cpp
GRAMMAR_HDRS = parser_ll1/grammar.h

# Fuentes del analizador semántico (Práctica 6)
SEMANTIC_SRCS = semantic_analyzer/ast.cpp semantic_analyzer/catalog.cpp semantic_analyzer/ast_builder.cpp semantic_analyzer/semantic_analyzer.cpp
SEMANTIC_HDRS = semantic_analyzer/semantic_types.h semantic_analyzer/ast.h semantic_analyzer/catalog.h semantic_analyzer/ast_builder.h semantic_analyzer/semantic_analyzer.h

all: lexer parser_cli semantic_cli test_lexer test_grammar test_parser test_semantic

lexer: src/main.cpp $(LEXER_SRCS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) src/main.cpp $(LEXER_SRCS) -o lexer

parser_cli: parser_ll1/main.cpp $(PARSER_SRCS) $(LEXER_SRCS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) parser_ll1/main.cpp $(PARSER_SRCS) $(LEXER_SRCS) -o parser_cli

semantic_cli: semantic_analyzer/main.cpp $(SEMANTIC_SRCS) $(PARSER_SRCS) $(LEXER_SRCS) $(SEMANTIC_HDRS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) semantic_analyzer/main.cpp $(SEMANTIC_SRCS) $(PARSER_SRCS) $(LEXER_SRCS) -o semantic_cli

test_lexer: tests/test_lexer.cpp $(LEXER_SRCS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_lexer.cpp $(LEXER_SRCS) -o test_lexer

test_grammar: tests/test_grammar.cpp $(GRAMMAR_SRCS) $(GRAMMAR_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_grammar.cpp $(GRAMMAR_SRCS) -o test_grammar

test_parser: tests/test_parser.cpp $(PARSER_SRCS) $(LEXER_SRCS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_parser.cpp $(PARSER_SRCS) $(LEXER_SRCS) -o test_parser

test_semantic: tests/test_semantic.cpp $(SEMANTIC_SRCS) $(PARSER_SRCS) $(LEXER_SRCS) $(SEMANTIC_HDRS) $(PARSER_HDRS) $(LEXER_HDRS)
	$(CXX) $(CXXFLAGS) tests/test_semantic.cpp $(SEMANTIC_SRCS) $(PARSER_SRCS) $(LEXER_SRCS) -o test_semantic

test: test_lexer test_grammar test_parser test_semantic
	./test_lexer docs/Reporte_Pruebas.md
	./test_grammar
	./test_parser docs/Reporte_Pruebas_Parser.md
	./test_semantic

clean:
	rm -f lexer parser_cli semantic_cli test_lexer test_grammar test_parser test_semantic

.PHONY: all test clean
