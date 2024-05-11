#ifndef KIWI_PARSING_KEYWORDS_H
#define KIWI_PARSING_KEYWORDS_H

#include <string>
#include <unordered_set>
#include "parsing/tokentype.h"
#include "typing/value.h"

struct {
  const k_string Qualifier = "::";
} Symbols;

struct {
  const k_string Add = "+";
  const k_string AddAssign = "+=";
  const k_string Assign = "=";
  const k_string Divide = "/";
  const k_string DivideAssign = "/=";
  const k_string Equal = "==";
  const k_string Exponent = "**";
  const k_string ExponentAssign = "**=";
  const k_string GreaterThan = ">";
  const k_string GreaterThanOrEqual = ">=";
  const k_string LessThan = "<";
  const k_string LessThanOrEqual = "<=";
  const k_string ModuloAssign = "%=";
  const k_string Modulus = "%";
  const k_string Multiply = "*";
  const k_string MultiplyAssign = "*=";
  const k_string NotEqual = "!=";
  const k_string Subtract = "-";
  const k_string SubtractAssign = "-=";
  const k_string Or = "||";
  const k_string OrAssign = "||=";
  const k_string And = "&&";
  const k_string AndAssign = "&&=";
  const k_string Not = "!";
  const k_string BitwiseOr = "|";
  const k_string BitwiseOrAssign = "|=";
  const k_string BitwiseAnd = "&";
  const k_string BitwiseAndAssign = "&=";
  const k_string BitwiseXor = "^";
  const k_string BitwiseXorAssign = "^=";
  const k_string BitwiseNot = "~";
  const k_string BitwiseNotAssign = "~=";
  const k_string BitwiseLeftShift = "<<";
  const k_string BitwiseLeftShiftAssign = "<<=";
  const k_string BitwiseRightShift = ">>";
  const k_string BitwiseRightShiftAssign = ">>=";

  std::unordered_set<k_string> large_operators = {
      Exponent, Or, And, BitwiseLeftShift, BitwiseRightShift};

  std::unordered_set<char> arithmetic_operator_chars = {'+', '-', '/', '*',
                                                        '%'};

  std::unordered_set<char> boolean_operator_chars = {'=', '!', '<',
                                                     '>', '|', '&'};

  std::unordered_set<char> bitwise_operator_chars = {'^', '~', '&', '|'};

  std::unordered_set<KName> assignment_operators = {
      KName::Ops_Assign,
      KName::Ops_AddAssign,
      KName::Ops_SubtractAssign,
      KName::Ops_MultiplyAssign,
      KName::Ops_DivideAssign,
      KName::Ops_ExponentAssign,
      KName::Ops_OrAssign,
      KName::Ops_AndAssign,
      KName::Ops_ModuloAssign,
      KName::Ops_BitwiseOrAssign,
      KName::Ops_BitwiseAndAssign,
      KName::Ops_BitwiseXorAssign,
      KName::Ops_BitwiseNotAssign,
      KName::Ops_BitwiseLeftShiftAssign,
      KName::Ops_BitwiseRightShiftAssign};

  std::unordered_set<KName> equality_operators = {KName::Ops_Equal,
                                                  KName::Ops_NotEqual};

  std::unordered_set<KName> additive_operators = {KName::Ops_Add,
                                                  KName::Ops_Subtract};

  std::unordered_set<KName> multiplicative_operators = {
      KName::Ops_Multiply, KName::Ops_Divide, KName::Ops_Modulus,
      KName::Ops_Exponent};

  std::unordered_set<KName> comparison_operators = {
      KName::Ops_GreaterThan, KName::Ops_GreaterThanOrEqual,
      KName::Ops_LessThan, KName::Ops_LessThanOrEqual};

  std::unordered_set<KName> bitwise_operators = {KName::Ops_BitwiseLeftShift,
                                                 KName::Ops_BitwiseRightShift};

  std::unordered_set<KName> unary_operators = {
      KName::Ops_Not, KName::Ops_Subtract, KName::Ops_BitwiseNot};

  bool is_equality_op(const KName& arg) {
    return equality_operators.find(arg) != equality_operators.end();
  }

  bool is_comparison_op(const KName& arg) {
    return comparison_operators.find(arg) != comparison_operators.end();
  }

  bool is_bitwise_op(const KName& arg) {
    return bitwise_operators.find(arg) != bitwise_operators.end();
  }

  bool is_additive_op(const KName& arg) {
    return additive_operators.find(arg) != additive_operators.end();
  }

  bool is_multiplicative_op(const KName& arg) {
    return multiplicative_operators.find(arg) != multiplicative_operators.end();
  }

  bool is_unary_op(const KName& arg) {
    return unary_operators.find(arg) != unary_operators.end();
  }

  bool is_large_operator(const k_string& arg) {
    return large_operators.find(arg) != large_operators.end();
  }

  bool is_assignment_operator(const KName& arg) {
    return assignment_operators.find(arg) != assignment_operators.end();
  }

  bool is_arithmetic_operator_char(const char& arg) {
    return arithmetic_operator_chars.find(arg) !=
           arithmetic_operator_chars.end();
  }

  bool is_bitwise_operator_char(const char& arg) {
    return bitwise_operator_chars.find(arg) != bitwise_operator_chars.end();
  }

  bool is_boolean_operator_char(const char& arg) {
    return boolean_operator_chars.find(arg) != boolean_operator_chars.end();
  }
} Operators;

struct {
  const k_string Abstract = "abstract";
  const k_string As = "as";
  const k_string Async = "async";
  const k_string Await = "await";
  const k_string Break = "break";
  const k_string Catch = "catch";
  const k_string Class = "class";
  const k_string Ctor = "initialize";
  const k_string Delete = "delete";
  const k_string Do = "do";
  const k_string Each = "each";
  const k_string Else = "else";
  const k_string ElseIf = "elsif";
  const k_string End = "end";
  const k_string Exit = "exit";
  const k_string Export = "export";
  const k_string False = "false";
  const k_string For = "for";
  const k_string Function = "fn";
  const k_string Go = "go";
  const k_string If = "if";
  const k_string Import = "import";
  const k_string In = "in";
  const k_string Method = "def";
  const k_string Package = "package";
  const k_string New = "new";
  const k_string Next = "next";
  const k_string Null = "null";
  const k_string Override = "override";
  const k_string Parse = "parse";
  const k_string Pass = "pass";
  const k_string Print = "print";
  const k_string PrintLn = "println";
  const k_string Private = "private";
  const k_string Return = "return";
  const k_string Static = "static";
  const k_string Then = "then";
  const k_string This = "@";
  const k_string Throw = "throw";
  const k_string True = "true";
  const k_string Try = "try";
  const k_string With = "with";
  const k_string When = "when";
  const k_string While = "while";

  std::unordered_set<k_string> keywords = {
      Abstract, As,    Async,  Await, Break,   Catch,   Class,   Delete,
      Do,       Else,  ElseIf, End,   Exit,    Export,  False,   For,
      Function, If,    Import, In,    With,    Method,  Package, Next,
      Override, Parse, Pass,   Print, PrintLn, Private, Return,  Static,
      Then,     This,  Throw,  True,  Try,     When,    While};

  std::unordered_set<k_string> conditional_keywords = {If, Else, ElseIf, End};

  std::unordered_set<KName> block_keywords = {
      KName::KW_While,   KName::KW_For, KName::KW_Method, KName::KW_If,
      KName::KW_Package, KName::KW_Try, KName::KW_Class,  KName::KW_Lambda};

  bool is_keyword(const k_string& arg) {
    return keywords.find(arg) != keywords.end();
  }

  bool is_boolean(const k_string& arg) { return arg == True || arg == False; }

  bool is_conditional_keyword(const k_string& arg) {
    return conditional_keywords.find(arg) != conditional_keywords.end();
  }

  bool is_loop_keyword(const KName& arg) {
    return arg == KName::KW_While || arg == KName::KW_For;
  }

  bool is_block_keyword(const KName& arg) {
    return block_keywords.find(arg) != block_keywords.end();
  }
} Keywords;

struct {
  const k_string Integer = "Integer";
  const k_string Double = "Double";
  const k_string Boolean = "Boolean";
  const k_string String = "String";
  const k_string List = "List";
  const k_string Hash = "Hash";
  const k_string Object = "Object";
  const k_string With = "Lambda";
  const k_string None = "None";

  std::unordered_set<k_string> typenames = {
      Integer, Double, Boolean, String, List, Hash, Object, With, None};

  bool is_typename(const k_string& arg) {
    return typenames.find(arg) != typenames.end();
  }
} TypeNames;

#endif