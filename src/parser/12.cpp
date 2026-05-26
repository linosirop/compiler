#include "ast.h"

#include <sstream>
#include <typeinfo>

namespace {
void indent(std::ostream& out, int level) {
    for (int i = 0; i < level; ++i) out << "  ";
}

std::string exprToString(const ExprPtr& expr) {
    if (!expr) return "null";
    if (auto literal = std::dynamic_pointer_cast<LiteralExprNode>(expr)) return literal->value;
    if (auto id = std::dynamic_pointer_cast<IdentifierExprNode>(expr)) return id->name;
    if (auto unary = std::dynamic_pointer_cast<UnaryExprNode>(expr)) return "(" + unary->op + exprToString(unary->operand) + ")";
    if (auto binary = std::dynamic_pointer_cast<BinaryExprNode>(expr)) return "(" + exprToString(binary->left) + " " + binary->op + " " + exprToString(binary->right) + ")";
    if (auto assign = std::dynamic_pointer_cast<AssignmentExprNode>(expr)) return "(" + exprToString(assign->target) + " " + assign->op + " " + exprToString(assign->value) + ")";
    if (auto call = std::dynamic_pointer_cast<CallExprNode>(expr)) {
        std::string result = call->callee + "(";
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += exprToString(call->arguments[i]);
        }
        result += ")";
        return result;
    }
    return "<expr>";
}

void printStmt(const StmtPtr& stmt, std::ostream& out, int level) {
    if (!stmt) return;
    if (auto block = std::dynamic_pointer_cast<BlockStmtNode>(stmt)) {
        indent(out, level); out << "Block [line " << block->line << "]:\n";
        for (const auto& child : block->statements) printStmt(child, out, level + 1);
    } else if (auto var = std::dynamic_pointer_cast<VarDeclStmtNode>(stmt)) {
        indent(out, level); out << "VarDecl: " << var->type << " " << var->name;
        if (var->initializer) out << " = " << exprToString(var->initializer);
        out << " [line " << var->line << "]\n";
    } else if (auto expr = std::dynamic_pointer_cast<ExprStmtNode>(stmt)) {
        indent(out, level); out << "ExprStmt: " << exprToString(expr->expression) << " [line " << expr->line << "]\n";
    } else if (auto empty = std::dynamic_pointer_cast<EmptyStmtNode>(stmt)) {
        indent(out, level); out << "EmptyStmt [line " << empty->line << "]\n";
    } else if (auto ifs = std::dynamic_pointer_cast<IfStmtNode>(stmt)) {
        indent(out, level); out << "IfStmt [line " << ifs->line << "]:\n";
        indent(out, level + 1); out << "Condition: " << exprToString(ifs->condition) << "\n";
        indent(out, level + 1); out << "Then:\n";
        printStmt(ifs->thenBranch, out, level + 2);
        if (ifs->elseBranch) {
            indent(out, level + 1); out << "Else:\n";
            printStmt(ifs->elseBranch, out, level + 2);
        }
    } else if (auto wh = std::dynamic_pointer_cast<WhileStmtNode>(stmt)) {
        indent(out, level); out << "WhileStmt [line " << wh->line << "]:\n";
        indent(out, level + 1); out << "Condition: " << exprToString(wh->condition) << "\n";
        printStmt(wh->body, out, level + 1);
    } else if (auto fs = std::dynamic_pointer_cast<ForStmtNode>(stmt)) {
        indent(out, level); out << "ForStmt [line " << fs->line << "]:\n";
        indent(out, level + 1); out << "Init:\n";
        printStmt(fs->init, out, level + 2);
        indent(out, level + 1); out << "Condition: " << exprToString(fs->condition) << "\n";
        indent(out, level + 1); out << "Update: " << exprToString(fs->update) << "\n";
        printStmt(fs->body, out, level + 1);
    } else if (auto ret = std::dynamic_pointer_cast<ReturnStmtNode>(stmt)) {
        indent(out, level); out << "Return: " << exprToString(ret->value) << " [line " << ret->line << "]\n";
    }
}

void printDecl(const DeclPtr& decl, std::ostream& out, int level) {
    if (auto fn = std::dynamic_pointer_cast<FunctionDeclNode>(decl)) {
        indent(out, level); out << "FunctionDecl: " << fn->name << " -> " << fn->returnType << " [line " << fn->line << "]:\n";
        indent(out, level + 1); out << "Parameters: [";
        for (size_t i = 0; i < fn->parameters.size(); ++i) {
            if (i > 0) out << ", ";
            out << fn->parameters[i].type << " " << fn->parameters[i].name;
        }
        out << "]\n";
        indent(out, level + 1); out << "Body:\n";
        printStmt(fn->body, out, level + 2);
    } else if (auto st = std::dynamic_pointer_cast<StructDeclNode>(decl)) {
        indent(out, level); out << "StructDecl: " << st->name << " [line " << st->line << "]:\n";
        for (const auto& field : st->fields) printStmt(field, out, level + 1);
    } else if (auto gv = std::dynamic_pointer_cast<GlobalVarDeclNode>(decl)) {
        printStmt(gv->declaration, out, level);
    }
}

struct DotWriter {
    std::ostream& out;
    int nextId = 0;

    int node(const std::string& label, const std::string& color) {
        int id = nextId++;
        out << "  n" << id << " [label=\"" << label << "\", style=filled, fillcolor=\"" << color << "\"];\n";
        return id;
    }

    void edge(int from, int to) { out << "  n" << from << " -> n" << to << ";\n"; }

    int expr(const ExprPtr& e) {
        if (!e) return node("null", "white");
        if (auto lit = std::dynamic_pointer_cast<LiteralExprNode>(e)) return node("Literal\\n" + lit->value, "lightyellow");
        if (auto id = std::dynamic_pointer_cast<IdentifierExprNode>(e)) return node("Identifier\\n" + id->name, "lightyellow");
        if (auto un = std::dynamic_pointer_cast<UnaryExprNode>(e)) {
            int n = node("Unary\\n" + un->op, "lightyellow"); edge(n, expr(un->operand)); return n;
        }
        if (auto bin = std::dynamic_pointer_cast<BinaryExprNode>(e)) {
            int n = node("Binary\\n" + bin->op, "lightyellow"); edge(n, expr(bin->left)); edge(n, expr(bin->right)); return n;
        }
        if (auto as = std::dynamic_pointer_cast<AssignmentExprNode>(e)) {
            int n = node("Assignment\\n" + as->op, "lightyellow"); edge(n, expr(as->target)); edge(n, expr(as->value)); return n;
        }
        if (auto call = std::dynamic_pointer_cast<CallExprNode>(e)) {
            int n = node("Call\\n" + call->callee, "lightyellow");
            for (auto& a : call->arguments) edge(n, expr(a));
            return n;
        }
        return node("Expr", "lightyellow");
    }

    int stmt(const StmtPtr& s) {
        if (!s) return node("null", "white");
        if (auto block = std::dynamic_pointer_cast<BlockStmtNode>(s)) {
            int n = node("Block", "lightblue"); for (auto& c : block->statements) edge(n, stmt(c)); return n;
        }
        if (auto var = std::dynamic_pointer_cast<VarDeclStmtNode>(s)) {
            int n = node("VarDecl\\n" + var->type + " " + var->name, "lightblue"); if (var->initializer) edge(n, expr(var->initializer)); return n;
        }
        if (auto es = std::dynamic_pointer_cast<ExprStmtNode>(s)) { int n = node("ExprStmt", "lightblue"); edge(n, expr(es->expression)); return n; }
        if (std::dynamic_pointer_cast<EmptyStmtNode>(s)) return node("EmptyStmt", "lightblue");
        if (auto ifs = std::dynamic_pointer_cast<IfStmtNode>(s)) { int n = node("IfStmt", "lightblue"); edge(n, expr(ifs->condition)); edge(n, stmt(ifs->thenBranch)); if (ifs->elseBranch) edge(n, stmt(ifs->elseBranch)); return n; }
        if (auto wh = std::dynamic_pointer_cast<WhileStmtNode>(s)) { int n = node("WhileStmt", "lightblue"); edge(n, expr(wh->condition)); edge(n, stmt(wh->body)); return n; }
        if (auto fs = std::dynamic_pointer_cast<ForStmtNode>(s)) { int n = node("ForStmt", "lightblue"); edge(n, stmt(fs->init)); edge(n, expr(fs->condition)); edge(n, expr(fs->update)); edge(n, stmt(fs->body)); return n; }
        if (auto ret = std::dynamic_pointer_cast<ReturnStmtNode>(s)) { int n = node("Return", "lightblue"); if (ret->value) edge(n, expr(ret->value)); return n; }
        return node("Stmt", "lightblue");
    }

    int decl(const DeclPtr& d) {
        if (auto fn = std::dynamic_pointer_cast<FunctionDeclNode>(d)) {
            int n = node("FunctionDecl\\n" + fn->name + " -> " + fn->returnType, "palegreen");
            for (const auto& p : fn->parameters) edge(n, node("Param\\n" + p.type + " " + p.name, "palegreen"));
            edge(n, stmt(fn->body)); return n;
        }
        if (auto st = std::dynamic_pointer_cast<StructDeclNode>(d)) { int n = node("StructDecl\\n" + st->name, "palegreen"); for (auto& f : st->fields) edge(n, stmt(f)); return n; }
        if (auto gv = std::dynamic_pointer_cast<GlobalVarDeclNode>(d)) return stmt(gv->declaration);
        return node("Decl", "palegreen");
    }
};
}

void printAstText(const ProgramNode& program, std::ostream& out) {
    out << "Program [line " << program.line << "]:\n";
    for (const auto& decl : program.declarations) printDecl(decl, out, 1);
}

void printAstDot(const ProgramNode& program, std::ostream& out) {
    out << "digraph AST {\n  node [shape=box, fontname=\"Consolas\"];\n";
    DotWriter writer{out};
    int root = writer.node("Program", "lightgray");
    for (const auto& decl : program.declarations) writer.edge(root, writer.decl(decl));
    out << "}\n";
}
