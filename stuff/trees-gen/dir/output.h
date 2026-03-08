template<typename T> struct Visitor;
struct Expr {
  template<typename T> T accept(Visitor<T> visitor);
};

struct Binary: public Expr {
  Binary(Expr* left, Token* _operator, Expr* right) {
    this->left = left;
    this->_operator = _operator;
    this->right = right;
  }
  Expr* left;
  Token* _operator;
  Expr* right;

  template<typename T> T accept(Visitor<T> visitor) {
    return visitor.visitBinaryExpr(*this);
  }
};

struct Grouping: public Expr {
  Grouping(Expr* expression) {
    this->expression = expression;
  }
  Expr* expression;

  template<typename T> T accept(Visitor<T> visitor) {
    return visitor.visitGroupingExpr(*this);
  }
};

struct Literal: public Expr {
  Literal(void* literal) {
    this->literal = literal;
  }
  void* literal;

  template<typename T> T accept(Visitor<T> visitor) {
    return visitor.visitLiteralExpr(*this);
  }
};

struct Unary: public Expr {
  Unary(Token* _operator, Expr* right) {
    this->_operator = _operator;
    this->right = right;
  }
  Token* _operator;
  Expr* right;

  template<typename T> T accept(Visitor<T> visitor) {
    return visitor.visitUnaryExpr(*this);
  }
};
template<typename T>
struct Visitor {
  T visitBinaryExpr(Binary expr);
  T visitGroupingExpr(Grouping expr);
  T visitLiteralExpr(Literal expr);
  T visitUnaryExpr(Unary expr);
};
