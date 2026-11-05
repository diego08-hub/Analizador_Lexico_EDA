#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

inline const std::string EPSILON = "ε";
inline const std::string END_MARKER = "$";

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
};

struct Grammar {
    std::string startSymbol;
    std::set<std::string> nonTerminals;
    std::set<std::string> terminals;
    std::vector<Production> productions;
};

using SymbolSet = std::set<std::string>;
using GrammarSets = std::map<std::string, SymbolSet>;

const Grammar& sqlGrammar();
GrammarSets calculateFirstSets(const Grammar& grammar);
GrammarSets calculateFollowSets(const Grammar& grammar, const GrammarSets& firstSets);
