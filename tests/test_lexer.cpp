#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../src/lexer.h"

// Mini framework de pruebas (sin dependencias externas)
static int passed = 0;
static int failed = 0;

// Contexto de la ultima entrada analizada (se muestra junto a cada prueba)
static std::string ctxInput;
static std::vector<Token> ctxToks;
static std::vector<std::string> ctxErrors;

// Registro de cada prueba para generar el reporte en Markdown
struct Record {
    std::string section, name, input, tokens, errors;
    bool ok;
};
static std::vector<Record> records;
static std::string currentSection;

static void section(const std::string& title) {
    currentSection = title;
    std::cout << title << "\n";
}

static std::string escape(const std::string& s) {
    std::string r;
    for (char c : s) {
        if (c == '\n') r += "\\n";
        else if (c == '\t') r += "\\t";
        else if (c == '\r') r += "\\r";
        else r += c;
    }
    return r;
}

static std::string describe(const std::vector<Token>& toks) {
    std::string s;
    for (const Token& t : toks) {
        if (t.type == TokenType::END_OF_FILE) { s += "EOF"; continue; }
        s += tokenTypeToString(t.type) + "(" + escape(t.lexeme) + ")@" +
             std::to_string(t.line) + ":" + std::to_string(t.column) + "  ";
    }
    return s;
}

// Analiza la entrada y guarda el contexto para mostrarlo en el reporte
static std::vector<Token> run(Lexer& lexer, const std::string& input) {
    ctxInput = input;
    ctxToks = lexer.tokenize();
    ctxErrors = lexer.getErrors();
    return ctxToks;
}

static void check(bool cond, const std::string& name) {
    if (cond) passed++; else failed++;
    std::string errs;
    for (const std::string& e : ctxErrors) errs += (errs.empty() ? "" : "<br>") + e;
    records.push_back({currentSection, name, ctxInput, describe(ctxToks), errs, cond});
    std::cout << (cond ? "  [OK]    " : "  [FALLO] ") << name << "\n"
              << "          entrada: \"" << escape(ctxInput) << "\"\n"
              << "          tokens:  " << describe(ctxToks) << "\n";
}

// Verifica que los tipos de token coincidan exactamente (incluye END_OF_FILE al final)
static void expectTypes(const std::string& name, const std::string& input,
                        const std::vector<TokenType>& expected) {
    Lexer lexer(input);
    std::vector<Token> toks = run(lexer, input);
    bool ok = toks.size() == expected.size();
    for (size_t i = 0; ok && i < expected.size(); i++) ok = toks[i].type == expected[i];
    check(ok, name);
}

// Verifica el lexema de un token concreto
static void expectLexeme(const std::string& name, const std::string& input,
                         size_t index, const std::string& lexeme) {
    Lexer lexer(input);
    std::vector<Token> toks = run(lexer, input);
    check(index < toks.size() && toks[index].lexeme == lexeme, name);
}

// Verifica posición (línea, columna) de un token concreto
static void expectPos(const std::string& name, const std::string& input,
                      size_t index, int line, int col) {
    Lexer lexer(input);
    std::vector<Token> toks = run(lexer, input);
    check(index < toks.size() && toks[index].line == line && toks[index].column == col,
          name);
}

// Celda de tabla Markdown: escapa | y saltos de linea
static std::string cell(const std::string& s) {
    std::string r;
    for (char c : s) r += (c == '|') ? std::string("\\|") : std::string(1, c);
    return r;
}

static void writeReport(const std::string& path) {
    std::ofstream out(path);
    out << "# Reporte de pruebas del analizador lexico\n\n"
        << "Generado automaticamente por `make test`.\n\n"
        << "**Resultado:** " << passed << " pruebas OK, " << failed << " fallidas\n\n"
        << "Formato de token: `TIPO(lexema)@linea:columna`\n";
    std::string last;
    for (const Record& r : records) {
        if (r.section != last) {
            last = r.section;
            out << "\n## " << last << "\n\n"
                << "| Prueba | Consulta | Tokens encontrados | Errores reportados | Resultado |\n"
                << "|---|---|---|---|---|\n";
        }
        out << "| " << cell(r.name) << " | `" << cell(escape(r.input)) << "` | `"
            << cell(r.tokens) << "` | " << cell(r.errors) << " | "
            << (r.ok ? "OK" : "**FALLO**") << " |\n";
    }
}

using T = TokenType;

int main(int argc, char* argv[]) {
    // 1. Sentencia válida
    section("1. Sentencia valida");
    expectTypes("select completo",
        "SELECT a, b FROM t WHERE x >= 10 AND y = 'hi' OR z < 2.5;",
        {T::SELECT_KW, T::IDENTIFIER, T::COMMA, T::IDENTIFIER, T::FROM_KW, T::IDENTIFIER,
         T::WHERE_KW, T::IDENTIFIER, T::GREATER_EQUAL, T::INTEGER, T::AND_KW,
         T::IDENTIFIER, T::EQUAL, T::STRING, T::OR_KW, T::IDENTIFIER, T::LESS, T::DECIMAL,
         T::SEMICOLON, T::END_OF_FILE});
    expectTypes("select *", "SELECT * FROM t",
        {T::SELECT_KW, T::STAR, T::FROM_KW, T::IDENTIFIER, T::END_OF_FILE});

    // 2. Operadores compuestos
    section("2. Operadores compuestos");
    expectTypes(">=", "a>=b", {T::IDENTIFIER, T::GREATER_EQUAL, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("<=", "a<=b", {T::IDENTIFIER, T::LESS_EQUAL, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("<>", "a<>b", {T::IDENTIFIER, T::NOT_EQUAL, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("!=", "a!=b", {T::IDENTIFIER, T::NOT_EQUAL, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("simples", "= < >", {T::EQUAL, T::LESS, T::GREATER, T::END_OF_FILE});
    expectLexeme("lexema <>", "<>", 0, "<>");
    expectTypes("< = separados", "< =", {T::LESS, T::EQUAL, T::END_OF_FILE});
    expectTypes(">>", ">>", {T::GREATER, T::GREATER, T::END_OF_FILE});

    // 3. Literales
    section("3. Literales");
    expectTypes("entero", "42", {T::INTEGER, T::END_OF_FILE});
    expectTypes("decimal", "3.14", {T::DECIMAL, T::END_OF_FILE});
    expectLexeme("lexema decimal", "3.14", 0, "3.14");
    expectTypes("cadena", "'hola mundo'", {T::STRING, T::END_OF_FILE});
    expectLexeme("cadena incluye comillas", "'hola'", 0, "'hola'");
    expectTypes("cadena vacia", "''", {T::STRING, T::END_OF_FILE});
    expectTypes("cadena con simbolos", "'a<=b, c;'", {T::STRING, T::END_OF_FILE});

    // 4. Palabras reservadas vs identificadores
    section("4. Palabras reservadas e identificadores");
    expectTypes("keywords mayusculas", "SELECT FROM WHERE AND OR",
        {T::SELECT_KW, T::FROM_KW, T::WHERE_KW, T::AND_KW, T::OR_KW, T::END_OF_FILE});
    expectTypes("keywords minusculas/mixtas", "select From wHeRe and Or",
        {T::SELECT_KW, T::FROM_KW, T::WHERE_KW, T::AND_KW, T::OR_KW, T::END_OF_FILE});
    expectLexeme("lexema original conservado", "select", 0, "select");
    expectTypes("prefijo de keyword es identificador", "selected fromage android order",
        {T::IDENTIFIER, T::IDENTIFIER, T::IDENTIFIER, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("identificadores con _ y digitos", "_id col1 a_b2",
        {T::IDENTIFIER, T::IDENTIFIER, T::IDENTIFIER, T::END_OF_FILE});

    // 5. Espacios en blanco
    section("5. Espacios en blanco");
    expectTypes("espacios, tabs y saltos", "SELECT \t a\n\n FROM\r\n t",
        {T::SELECT_KW, T::IDENTIFIER, T::FROM_KW, T::IDENTIFIER, T::END_OF_FILE});
    expectTypes("sin espacios", "a,b;",
        {T::IDENTIFIER, T::COMMA, T::IDENTIFIER, T::SEMICOLON, T::END_OF_FILE});
    expectPos("posicion token 1", "SELECT a", 0, 1, 1);
    expectPos("posicion token 2", "SELECT a", 1, 1, 8);
    expectPos("posicion tras salto de linea", "SELECT\n  a", 1, 2, 3);
    expectPos("posicion tras tab", "\ta", 0, 1, 2);

    // 6. Errores léxicos
    section("6. Errores lexicos");
    {
        Lexer l("a @ b");
        std::vector<Token> toks = run(l, "a @ b");
        check(toks.size() == 4 && toks[1].type == T::ERROR && toks[1].lexeme == "@"
              && toks[1].line == 1 && toks[1].column == 3, "caracter invalido");
        check(l.getErrors().size() == 1 &&
              l.getErrors()[0].find("línea 1, columna 3") != std::string::npos,
              "mensaje de error con posicion");
    }
    {
        Lexer l("x = 'abc");
        std::vector<Token> toks = run(l, "x = 'abc");
        check(toks.size() == 4 && toks[2].type == T::ERROR && toks[2].column == 5,
              "cadena sin cerrar");
        check(l.getErrors().size() == 1, "cadena sin cerrar reporta 1 error");
    }
    {
        Lexer l("x = 'abc\ny");
        std::vector<Token> toks = run(l, "x = 'abc\ny");
        check(toks[2].type == T::ERROR && toks[3].type == T::IDENTIFIER && toks[3].line == 2,
              "cadena sin cerrar no cruza lineas");
    }
    expectTypes("! solo es error", "!", {T::ERROR, T::END_OF_FILE});
    expectTypes("decimal mal formado", "1.", {T::ERROR, T::END_OF_FILE});
    {
        Lexer l("# $ @");
        run(l, "# $ @");
        check(l.getErrors().size() == 3, "varios errores consecutivos");
    }
    {
        Lexer l("SELECT a FROM t");
        run(l, "SELECT a FROM t");
        check(l.getErrors().empty(), "entrada valida no genera errores");
    }

    // 7. Casos límite
    section("7. Casos limite");
    expectTypes("entrada vacia", "", {T::END_OF_FILE});
    expectTypes("solo espacios", "  \n\t ", {T::END_OF_FILE});
    expectTypes("sentencia incompleta", "SELECT", {T::SELECT_KW, T::END_OF_FILE});
    expectTypes("sentencia incompleta 2", "SELECT a FROM WHERE",
        {T::SELECT_KW, T::IDENTIFIER, T::FROM_KW, T::WHERE_KW, T::END_OF_FILE});
    expectTypes("token pegado al final", "a=1",
        {T::IDENTIFIER, T::EQUAL, T::INTEGER, T::END_OF_FILE});
    expectTypes("operador al final", "a<", {T::IDENTIFIER, T::LESS, T::END_OF_FILE});
    expectTypes("cadena al final", "x='a'",
        {T::IDENTIFIER, T::EQUAL, T::STRING, T::END_OF_FILE});
    expectPos("EOF posicion", "a\nb", 2, 2, 2);
    {
        Lexer l("SELECT a");
        auto first = run(l, "SELECT a");
        auto second = run(l, "SELECT a");
        check(first.size() == second.size(), "tokenize repetido no duplica tokens");
    }

    std::string reportPath = argc > 1 ? argv[1] : "reporte_pruebas.md";
    writeReport(reportPath);
    std::cout << "\nReporte generado en: " << reportPath << "\n";
    std::cout << "\nResultado: " << passed << " pruebas OK, " << failed << " fallidas\n";
    return failed == 0 ? 0 : 1;
}
