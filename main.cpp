#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stdexcept>

using namespace std;


enum TokenType {
    T_END, T_NUM, T_ID,
    T_PLUS, T_MINUS, T_MUL, T_DIV,
    T_LP, T_RP,
    T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
    T_AND, T_OR
};

struct Token {
    TokenType type;
    string text;
    int pos;
};

string input;
int pos = 0;
Token cur;

void skipSpaces() {
    while (pos < input.size() && isspace(input[pos]))
        pos++;
}

Token getToken() {
    skipSpaces();

    if (pos >= input.size())
        return {T_END, "", pos};

    int start = pos;

    if (isdigit(input[pos]) || input[pos] == '.') {
        while (pos < input.size() &&
              (isdigit(input[pos]) || input[pos] == '.'))
            pos++;
        return {T_NUM, input.substr(start, pos - start), start};
    }

    if (isalpha(input[pos])) {
        while (pos < input.size() && isalnum(input[pos]))
            pos++;
        return {T_ID, input.substr(start, pos - start), start};
    }

    string two = input.substr(pos, 2);
    if (two == "&&") { pos += 2; return {T_AND, "&&", start}; }
    if (two == "||") { pos += 2; return {T_OR,  "||", start}; }
    if (two == "==") { pos += 2; return {T_EQ,  "==", start}; }
    if (two == "!=") { pos += 2; return {T_NE,  "!=", start}; }
    if (two == "<=") { pos += 2; return {T_LE,  "<=", start}; }
    if (two == ">=") { pos += 2; return {T_GE,  ">=", start}; }

    char c = input[pos++];
    switch (c) {
        case '+': return {T_PLUS, "+", start};
        case '-': return {T_MINUS, "-", start};
        case '*': return {T_MUL, "*", start};
        case '/': return {T_DIV, "/", start};
        case '(': return {T_LP, "(", start};
        case ')': return {T_RP, ")", start};
        case '<': return {T_LT, "<", start};
        case '>': return {T_GT, ">", start};
    }

    throw runtime_error("Недопустимый символ в позиции " + to_string(start));
}

void nextToken() {
    cur = getToken();
}

struct Node {
    string op;      
    double value;   
    Node* left;
    Node* right;
};

Node* expression();
Node* and_expr();
Node* comparison();
Node* add();
Node* mul();
Node* unary();
Node* primary();


Node* makeNode(string op, Node* l = nullptr, Node* r = nullptr) {
    Node* n = new Node;
    n->op = op;
    n->left = l;
    n->right = r;
    return n;
}

Node* primary() {
    if (cur.type == T_NUM) {
        Node* n = makeNode("num");
        n->value = stod(cur.text);
        nextToken();
        return n;
    }

    if (cur.type == T_ID) {
        Node* n = makeNode(cur.text);
        nextToken();
        return n;
    }

    if (cur.type == T_LP) {
        nextToken();
        Node* n = expression();
        if (cur.type != T_RP)
            throw runtime_error("Ожидалась ')'");
        nextToken();
        return n;
    }

    throw runtime_error("Ожидался операнд");
}

Node* unary() {
    if (cur.type == T_MINUS) {
        nextToken();
        return makeNode("-", makeNode("num", nullptr, nullptr), unary());
    }
    return primary();
}

Node* mul() {
    Node* n = unary();
    while (cur.type == T_MUL || cur.type == T_DIV) {
        string op = cur.text;
        nextToken();
        n = makeNode(op, n, unary());
    }
    return n;
}

Node* add() {
    Node* n = mul();
    while (cur.type == T_PLUS || cur.type == T_MINUS) {
        string op = cur.text;
        nextToken();
        n = makeNode(op, n, mul());
    }
    return n;
}

Node* comparison() {
    Node* n = add();
    if (cur.type == T_EQ || cur.type == T_NE ||
        cur.type == T_LT || cur.type == T_GT ||
        cur.type == T_LE || cur.type == T_GE) {
        string op = cur.text;
        nextToken();
        n = makeNode(op, n, add());
    }
    return n;
}

Node* and_expr() {
    Node* n = comparison();
    while (cur.type == T_AND) {
        nextToken();
        n = makeNode("&&", n, comparison());
    }
    return n;
}


Node* expression() {
    Node* n = and_expr();
    while (cur.type == T_OR) {
        nextToken();
        n = makeNode("||", n, and_expr());
    }
    return n;
}


void printRPN(Node* n) {
    if (!n) return;
    printRPN(n->left);
    printRPN(n->right);

    if (n->op == "num")
        cout << n->value << " ";
    else
        cout << n->op << " ";
}

double eval(Node* n, map<string,double>& vars) {
    if (n->op == "num")
        return n->value;

    if (!n->left && !n->right) {
        if (!vars.count(n->op))
            throw runtime_error("Не задана переменная: " + n->op);
        return vars[n->op];
    }

    double a = eval(n->left, vars);
    double b = eval(n->right, vars);

    if (n->op == "+") return a + b;
    if (n->op == "-") return a - b;
    if (n->op == "*") return a * b;
    if (n->op == "/") {
        if (b == 0) throw runtime_error("Деление на ноль");
        return a / b;
    }
    if (n->op == "==") return a == b;
    if (n->op == "!=") return a != b;
    if (n->op == "<")  return a < b;
    if (n->op == ">")  return a > b;
    if (n->op == "<=") return a <= b;
    if (n->op == ">=") return a >= b;
    if (n->op == "&&") return a && b;
    if (n->op == "||") return a || b;

    throw runtime_error("Неизвестный оператор");
}

int main() {
    try {
        cout << "Введите выражение:\n";
        getline(cin, input);

        pos = 0;
        nextToken();
        Node* tree = expression();

        if (cur.type != T_END)
            throw runtime_error("Лишние символы в конце");

        cout << "\nОПЗ:\n";
        printRPN(tree);
        cout << "\n";

        map<string,double> vars;
        cout << "\nВведите переменные (имя значение), конец — end:\n";
        while (true) {
            string k;
            cin >> k;
            if (k == "end") break;
            cin >> vars[k];
        }

        cout << "\nРезультат:\n";
        cout << eval(tree, vars) << endl;
    }
    catch (exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
    }
}
