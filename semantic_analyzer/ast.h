#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "semantic_types.h"

namespace semantic {

// Clase base para todos los nodos del Árbol de Sintaxis Abstracta (AST)
class ASTNode {
public:
    int line = 0;
    int column = 0;
    DataType inferredType = DataType::UNKNOWN;

    ASTNode(int l = 0, int c = 0, DataType t = DataType::UNKNOWN)
        : line(l), column(c), inferredType(t) {}

    virtual ~ASTNode() = default;

    virtual std::string nodeName() const = 0;
    virtual void print(std::ostream& os, int indent = 0) const = 0;
};

// Nodo que representa una columna proyectada o referenciada en WHERE
class ColumnNode : public ASTNode {
public:
    std::string name;
    int ordinalPosition = -1;
    bool expandedFromStar = false;

    ColumnNode(std::string colName, int l = 0, int c = 0, bool fromStar = false)
        : ASTNode(l, c, DataType::UNKNOWN),
          name(std::move(colName)),
          expandedFromStar(fromStar) {}

    std::string nodeName() const override { return "ColumnNode"; }
    void print(std::ostream& os, int indent = 0) const override;
};

// Nodo que representa un literal (INTEGER, DECIMAL, STRING, BOOLEAN)
class LiteralNode : public ASTNode {
public:
    std::string rawValue;
    DataType literalType;

    LiteralNode(std::string value, DataType type, int l = 0, int c = 0)
        : ASTNode(l, c, type),
          rawValue(std::move(value)),
          literalType(type) {}

    std::string nodeName() const override { return "LiteralNode"; }
    void print(std::ostream& os, int indent = 0) const override;
};

// Nodo que representa una expresión binaria (lógica, relacional o aritmética)
class BinaryExprNode : public ASTNode {
public:
    enum class OpCategory {
        LOGICAL,     // AND, OR
        RELATIONAL,  // =, <>, !=, <, >, <=, >=
        ARITHMETIC   // +, -, *, /
    };

    OpCategory category;
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    bool hasImplicitCoercion = false; // true cuando ocurre promoción INT -> FLOAT

    BinaryExprNode(OpCategory cat,
                   std::string opStr,
                   std::unique_ptr<ASTNode> lhs,
                   std::unique_ptr<ASTNode> rhs,
                   int l = 0,
                   int c = 0)
        : ASTNode(l, c, DataType::UNKNOWN),
          category(cat),
          op(std::move(opStr)),
          left(std::move(lhs)),
          right(std::move(rhs)) {}

    std::string nodeName() const override { return "BinaryExprNode"; }
    void print(std::ostream& os, int indent = 0) const override;
};

// Nodo raíz de una consulta SELECT ... FROM ... [WHERE ...]
class SelectStmtNode : public ASTNode {
public:
    bool isSelectStar = false;
    int starLine = 0;
    int starColumn = 0;

    std::vector<std::unique_ptr<ColumnNode>> projections;

    std::string tableName;
    int tableLine = 0;
    int tableColumn = 0;

    std::unique_ptr<ASTNode> whereClause; // nullptr si no hay cláusula WHERE

    SelectStmtNode(int l = 0, int c = 0)
        : ASTNode(l, c, DataType::UNKNOWN) {}

    std::string nodeName() const override { return "SelectStmtNode"; }
    void print(std::ostream& os, int indent = 0) const override;
};

} // namespace semantic
