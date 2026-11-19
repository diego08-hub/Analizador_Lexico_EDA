#include "ast.h"

namespace semantic {

namespace {

std::string makeIndent(int indent) {
    return std::string(static_cast<size_t>(indent * 2), ' ');
}

std::string opCategoryToString(BinaryExprNode::OpCategory cat) {
    switch (cat) {
        case BinaryExprNode::OpCategory::LOGICAL:    return "LOGICAL";
        case BinaryExprNode::OpCategory::RELATIONAL: return "RELATIONAL";
        case BinaryExprNode::OpCategory::ARITHMETIC: return "ARITHMETIC";
    }
    return "UNKNOWN";
}

} // namespace

// NOTA (Persona 1): Puedes personalizar o enriquecer este formateo del AST decorado
// en tu rama feat/catalog-ast manteniendo las firmas declaradas en ast.h.

void ColumnNode::print(std::ostream& os, int indent) const {
    os << makeIndent(indent) << "ColumnNode('" << name << "')"
       << " [type=" << dataTypeToString(inferredType);
    if (ordinalPosition > 0) {
        os << ", ordinal=" << ordinalPosition;
    }
    if (expandedFromStar) {
        os << ", from=*";
    }
    os << "] @(" << line << ":" << column << ")\n";
}

void LiteralNode::print(std::ostream& os, int indent) const {
    os << makeIndent(indent) << "LiteralNode(" << rawValue << ")"
       << " [type=" << dataTypeToString(inferredType) << "]"
       << " @(" << line << ":" << column << ")\n";
}

void BinaryExprNode::print(std::ostream& os, int indent) const {
    os << makeIndent(indent) << "BinaryExprNode('" << op << "', "
       << opCategoryToString(category) << ")"
       << " [type=" << dataTypeToString(inferredType);
    if (hasImplicitCoercion) {
        os << ", coercion=INT->FLOAT";
    }
    os << "] @(" << line << ":" << column << ")\n";

    if (left) {
        left->print(os, indent + 1);
    }
    if (right) {
        right->print(os, indent + 1);
    }
}

void SelectStmtNode::print(std::ostream& os, int indent) const {
    os << makeIndent(indent) << "SelectStmtNode @(" << line << ":" << column << ")\n";

    os << makeIndent(indent + 1) << "FROM Table('" << tableName << "')"
       << " @(" << tableLine << ":" << tableColumn << ")\n";

    os << makeIndent(indent + 1) << "SELECT Projections"
       << (isSelectStar ? " (expanded from '*')" : "") << ":\n";
    for (const auto& col : projections) {
        if (col) {
            col->print(os, indent + 2);
        }
    }

    if (whereClause) {
        os << makeIndent(indent + 1) << "WHERE [type="
           << dataTypeToString(whereClause->inferredType) << "]:\n";
        whereClause->print(os, indent + 2);
    } else {
        os << makeIndent(indent + 1) << "WHERE: <none>\n";
    }
}

} // namespace semantic
