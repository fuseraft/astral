#ifndef ASTRAL_PARSING_LEXER_H
#define ASTRAL_PARSING_LEXER_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "parsing/builtins.h"
#include "parsing/keywords.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "system/fileregistry.h"

class Lexer {
 public:
  Lexer(const std::string& file, const std::string& source, bool skipWS = true)
      : source(source), pos(0), skipWS(skipWS), row(0), col(0) {
    fileId = FileRegistry::getInstance().registerFile(file);
  }

  k_stream getTokenStream() {
    return std::make_shared<TokenStream>(getAllTokens());
  }

  std::vector<Token> getAllTokens() {
    std::vector<Token> tokens;
    row = 0;
    col = 0;

    while (true) {
      auto token = _getNextToken();

      if (token.getType() == KTokenType::ENDOFFILE) {
        break;
      }

      tokens.emplace_back(token);
    }

    return tokens;
  }

 private:
  std::string source;
  size_t pos;
  bool skipWS;
  int fileId;
  int row;
  int col;

  char getCurrentChar() {
    char c = source[pos++];
    if (c == '\n') {
      row++;
      col = 0;
    } else {
      col++;
    }
    return c;
  }

  char peek() {
    if (pos + 1 < source.length()) {
      return source[pos + 1];
    }
    return '\0';
  }

  Token _getNextToken() {
    skipWhitespace();

    if (pos >= source.length()) {
      return createToken(KTokenType::ENDOFFILE, KName::Default, "");
    }

    char currentChar = getCurrentChar();

    if (isalpha(currentChar) || currentChar == '_') {
      return parseIdentifier(currentChar);
    } else if (isdigit(currentChar)) {
      if (currentChar == '0' && (pos < source.length() && source[pos] == 'x')) {
        return parseHexLiteral();
      } else if (currentChar == '0' &&
                 (pos < source.length() && source[pos] == 'b')) {
        return parseBinaryLiteral();
      } else if (currentChar == '0' &&
                 (pos < source.length() && source[pos] == 'o')) {
        return parseOctalLiteral();
      }
      return parseLiteral(currentChar);
    } else if (currentChar == '"') {
      return parseString();
    } else if (currentChar == '\'') {
      return parseRegex();
    } else if (currentChar == '#') {
      return parseComment();
    } else if (currentChar == '/' &&
               (pos < source.length() && source[pos] == '#')) {
      return parseBlockComment();
    } else if (currentChar == '@') {
      return createToken(KTokenType::KEYWORD, KName::KW_This, "@");
    } else if (currentChar == '$') {
      return createToken(KTokenType::OPERATOR, KName::Default, "$");
    } else if (currentChar == '\n') {
      return createToken(KTokenType::NEWLINE, KName::Default, "\n");
    } else if (currentChar == '(') {
      return createToken(KTokenType::OPEN_PAREN, KName::Default, "(");
    } else if (currentChar == ')') {
      return createToken(KTokenType::CLOSE_PAREN, KName::Default, ")");
    } else if (currentChar == '[') {
      return createToken(KTokenType::OPEN_BRACKET, KName::Default, "[");
    } else if (currentChar == ']') {
      return createToken(KTokenType::CLOSE_BRACKET, KName::Default, "]");
    } else if (currentChar == '{') {
      return createToken(KTokenType::OPEN_BRACE, KName::Default, "{");
    } else if (currentChar == '}') {
      return createToken(KTokenType::CLOSE_BRACE, KName::Default, "}");
    } else if (currentChar == ',') {
      return createToken(KTokenType::COMMA, KName::Default, ",");
    } else if (currentChar == '?') {
      return createToken(KTokenType::QUESTION, KName::Default, "?");
    } else if (currentChar == '.') {
      return parseDot(currentChar);
    } else if (currentChar == '\\') {
      return parseEscapeCharacter();
    } else if (currentChar == ':') {
      return parseColon(currentChar);
    } else {
      return parseUnspecified(currentChar);
    }
  }

  void skipWhitespace() {
    if (!skipWS) {
      return;
    }

    while (pos < source.length() && isspace(source[pos])) {
      getCurrentChar();
    }
  }

  Token parseRegex() {
    std::string regexPattern;
    bool escape = false;

    while (pos < source.length()) {
      char currentChar = getCurrentChar();

      if (escape) {
        switch (currentChar) {
          case 'n':
            regexPattern += '\n';
            break;
          case 't':
            regexPattern += '\t';
            break;
          case 'r':
            regexPattern += '\r';
            break;
          case '\\':
            regexPattern += '\\';
            break;
          case '/':
            regexPattern += '/';
            break;
          default:
            regexPattern += "\\" + std::string(1, currentChar);
        }
        escape = false;
      } else if (currentChar == '\\') {
        escape = true;
      } else if (currentChar == '\'') {
        if (!escape) {
          break;
        }
        regexPattern += currentChar;
      } else {
        regexPattern += currentChar;
      }
    }

    return createToken(KTokenType::STRING, KName::Regex, regexPattern);
  }

  Token parseConditionalKeyword(const std::string& keyword) {
    auto st = KName::Default;

    if (keyword == Keywords.If) {
      st = KName::KW_If;
    } else if (keyword == Keywords.ElseIf) {
      st = KName::KW_ElseIf;
    } else if (keyword == Keywords.Else) {
      st = KName::KW_Else;
    } else if (keyword == Keywords.End) {
      st = KName::KW_End;
    }

    return createToken(KTokenType::CONDITIONAL, st, keyword);
  }

  Token parseKeywordSpecific(const std::string& keyword) {
    auto st = KName::Default;

    if (keyword == Keywords.Abstract) {
      st = KName::KW_Abstract;
    } else if (keyword == Keywords.As) {
      st = KName::KW_As;
    } else if (keyword == Keywords.Async) {
      st = KName::KW_Async;
    } else if (keyword == Keywords.Await) {
      st = KName::KW_Await;
    } else if (keyword == Keywords.Break) {
      st = KName::KW_Break;
    } else if (keyword == Keywords.Catch) {
      st = KName::KW_Catch;
    } else if (keyword == Keywords.Class) {
      st = KName::KW_Class;
    } else if (keyword == Keywords.Delete) {
      st = KName::KW_Delete;
    } else if (keyword == Keywords.Do) {
      st = KName::KW_Do;
    } else if (keyword == Keywords.Exit) {
      st = KName::KW_Exit;
    } else if (keyword == Keywords.Export) {
      st = KName::KW_Export;
    } else if (keyword == Keywords.False) {
      st = KName::KW_False;
    } else if (keyword == Keywords.For) {
      st = KName::KW_For;
    } else if (keyword == Keywords.Import) {
      st = KName::KW_Import;
    } else if (keyword == Keywords.In) {
      st = KName::KW_In;
    } else if (keyword == Keywords.Method) {
      st = KName::KW_Method;
    } else if (keyword == Keywords.Module) {
      st = KName::KW_Module;
    } else if (keyword == Keywords.Next) {
      st = KName::KW_Next;
    } else if (keyword == Keywords.Override) {
      st = KName::KW_Override;
    } else if (keyword == Keywords.Parse) {
      st = KName::KW_Parse;
    } else if (keyword == Keywords.Pass) {
      st = KName::KW_Pass;
    } else if (keyword == Keywords.Print) {
      st = KName::KW_Print;
    } else if (keyword == Keywords.PrintLn) {
      st = KName::KW_PrintLn;
    } else if (keyword == Keywords.Private) {
      st = KName::KW_Private;
    } else if (keyword == Keywords.Return) {
      st = KName::KW_Return;
    } else if (keyword == Keywords.Static) {
      st = KName::KW_Static;
    } else if (keyword == Keywords.Then) {
      st = KName::KW_Then;
    } else if (keyword == Keywords.This) {
      st = KName::KW_This;
    } else if (keyword == Keywords.Throw) {
      st = KName::KW_Throw;
    } else if (keyword == Keywords.Try) {
      st = KName::KW_Try;
    } else if (keyword == Keywords.While) {
      st = KName::KW_While;
    }

    return createToken(KTokenType::KEYWORD, st, keyword);
  }

  Token parseKeyword(const std::string& keyword) {
    if (Keywords.is_conditional_keyword(keyword)) {
      return parseConditionalKeyword(keyword);
    } else if (keyword == Keywords.With) {
      return createToken(KTokenType::LAMBDA, KName::KW_Lambda, keyword);
    } else if (Keywords.is_boolean(keyword)) {
      return Token::createBoolean(fileId, keyword, row, col);
    }

    return parseKeywordSpecific(keyword);
  }

  Token parseOperator(const std::string& op) {
    auto st = KName::Default;

    if (op == Operators.Add) {
      st = KName::Ops_Add;
    } else if (op == Operators.AddAssign) {
      st = KName::Ops_AddAssign;
    } else if (op == Operators.And) {
      st = KName::Ops_And;
    } else if (op == Operators.AndAssign) {
      st = KName::Ops_AndAssign;
    } else if (op == Operators.Assign) {
      st = KName::Ops_Assign;
    } else if (op == Operators.BitwiseAnd) {
      st = KName::Ops_BitwiseAnd;
    } else if (op == Operators.BitwiseAndAssign) {
      st = KName::Ops_BitwiseAndAssign;
    } else if (op == Operators.BitwiseLeftShift) {
      st = KName::Ops_BitwiseLeftShift;
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      st = KName::Ops_BitwiseLeftShiftAssign;
    } else if (op == Operators.BitwiseNot) {
      st = KName::Ops_BitwiseNot;
    } else if (op == Operators.BitwiseNotAssign) {
      st = KName::Ops_BitwiseNotAssign;
    } else if (op == Operators.BitwiseOr) {
      st = KName::Ops_BitwiseOr;
    } else if (op == Operators.BitwiseOrAssign) {
      st = KName::Ops_BitwiseOrAssign;
    } else if (op == Operators.BitwiseRightShift) {
      st = KName::Ops_BitwiseRightShift;
    } else if (op == Operators.BitwiseRightShiftAssign) {
      st = KName::Ops_BitwiseRightShiftAssign;
    } else if (op == Operators.BitwiseXor) {
      st = KName::Ops_BitwiseXor;
    } else if (op == Operators.BitwiseXorAssign) {
      st = KName::Ops_BitwiseXorAssign;
    } else if (op == Operators.Divide) {
      st = KName::Ops_Divide;
    } else if (op == Operators.DivideAssign) {
      st = KName::Ops_DivideAssign;
    } else if (op == Operators.Equal) {
      st = KName::Ops_Equal;
    } else if (op == Operators.Exponent) {
      st = KName::Ops_Exponent;
    } else if (op == Operators.ExponentAssign) {
      st = KName::Ops_ExponentAssign;
    } else if (op == Operators.GreaterThan) {
      st = KName::Ops_GreaterThan;
    } else if (op == Operators.GreaterThanOrEqual) {
      st = KName::Ops_GreaterThanOrEqual;
    } else if (op == Operators.LessThan) {
      st = KName::Ops_LessThan;
    } else if (op == Operators.LessThanOrEqual) {
      st = KName::Ops_LessThanOrEqual;
    } else if (op == Operators.ModuloAssign) {
      st = KName::Ops_ModuloAssign;
    } else if (op == Operators.Modulus) {
      st = KName::Ops_Modulus;
    } else if (op == Operators.Multiply) {
      st = KName::Ops_Multiply;
    } else if (op == Operators.MultiplyAssign) {
      st = KName::Ops_MultiplyAssign;
    } else if (op == Operators.Not) {
      st = KName::Ops_Not;
    } else if (op == Operators.NotEqual) {
      st = KName::Ops_NotEqual;
    } else if (op == Operators.Or) {
      st = KName::Ops_Or;
    } else if (op == Operators.OrAssign) {
      st = KName::Ops_OrAssign;
    } else if (op == Operators.Subtract) {
      st = KName::Ops_Subtract;
    } else if (op == Operators.SubtractAssign) {
      st = KName::Ops_SubtractAssign;
    }

    return createToken(KTokenType::OPERATOR, st, op);
  }

  Token parseUnspecified(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      bool isArithmeticOpChar =
          Operators.is_arithmetic_operator_char(initialChar);
      bool isBooleanOpChar = Operators.is_boolean_operator_char(initialChar);
      bool isArithmeticOp =
          (nextChar == '=' && (isArithmeticOpChar || isBooleanOpChar)) ||
          (initialChar == '*' && nextChar == '*');
      bool isBooleanOp =
          (nextChar == '|' || nextChar == '&') && isBooleanOpChar;
      bool isBitwiseOp = (Operators.is_bitwise_operator_char(initialChar) &&
                          nextChar == '=') ||
                         (initialChar == '<' && nextChar == '<') ||
                         (initialChar == '>' && nextChar == '>');

      if (isArithmeticOp || isBooleanOp || isBitwiseOp) {
        s += nextChar;
        getCurrentChar();

        nextChar = source[pos];
        if (nextChar == '=' && Operators.is_large_operator(s)) {
          s += nextChar;
          getCurrentChar();
        }
      }
    }

    return parseOperator(s);
  }

  Token parseColon(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == ':') {
        s += nextChar;
        getCurrentChar();
        return createToken(KTokenType::QUALIFIER, KName::Default, s);
      }
    }

    return createToken(KTokenType::COLON, KName::Default, s);
  }

  Token parseEscapeCharacter() {
    if (pos < source.length()) {
      char nextChar = getCurrentChar();

      switch (nextChar) {
        case 'n':
          return createToken(KTokenType::ESCAPED, KName::Default, "\n");
        case 'r':
          return createToken(KTokenType::ESCAPED, KName::Default, "\r");
        case 't':
          return createToken(KTokenType::ESCAPED, KName::Default, "\t");
        case '"':
          return createToken(KTokenType::ESCAPED, KName::Default, "\"");
        case 'b':
          return createToken(KTokenType::ESCAPED, KName::Default, "\b");
        case 'f':
          return createToken(KTokenType::ESCAPED, KName::Default, "\f");
        case '\\':
          return createToken(KTokenType::ESCAPED, KName::Default, "\\");
      }
    }

    getCurrentChar();

    return createToken(KTokenType::ESCAPED, KName::Default, "\\");
  }

  Token parseDot(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == '.') {
        s += nextChar;
        getCurrentChar();
        return createToken(KTokenType::RANGE, KName::Default, s);
      }
    }

    return createToken(KTokenType::DOT, KName::Default, ".");
  }

  Token parseTypeName(const std::string& typeName) {
    auto st = KName::Default;

    if (typeName == TypeNames.Integer) {
      st = KName::Types_Integer;
    } else if (typeName == TypeNames.Boolean) {
      st = KName::Types_Boolean;
    } else if (typeName == TypeNames.Double) {
      st = KName::Types_Double;
    } else if (typeName == TypeNames.Hash) {
      st = KName::Types_Hash;
    } else if (typeName == TypeNames.With) {
      st = KName::Types_Lambda;
    } else if (typeName == TypeNames.List) {
      st = KName::Types_List;
    } else if (typeName == TypeNames.Object) {
      st = KName::Types_Object;
    } else if (typeName == TypeNames.String) {
      st = KName::Types_String;
    } else if (typeName == TypeNames.None) {
      st = KName::Types_None;
    }

    return createToken(KTokenType::TYPENAME, st, typeName);
  }

  Token parseArgvBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ArgvBuiltins.GetArgv) {
      st = KName::Builtin_Argv_GetArgv;
    } else if (builtin == ArgvBuiltins.GetXarg) {
      st = KName::Builtin_Argv_GetXarg;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseConsoleBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ConsoleBuiltins.Input) {
      st = KName::Builtin_Console_Input;
    } else if (builtin == ConsoleBuiltins.Silent) {
      st = KName::Builtin_Console_Silent;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseEncoderBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == EncoderBuiltins.Base64Decode) {
      st = KName::Builtin_Encoder_Base64Decode;
    } else if (builtin == EncoderBuiltins.Base64Encode) {
      st = KName::Builtin_Encoder_Base64Encode;
    } else if (builtin == EncoderBuiltins.UrlDecode) {
      st = KName::Builtin_Encoder_UrlDecode;
    } else if (builtin == EncoderBuiltins.UrlEncode) {
      st = KName::Builtin_Encoder_UrlEncode;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseSerializerBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == SerializerBuiltins.Deserialize) {
      st = KName::Builtin_Serializer_Deserialize;
    } else if (builtin == SerializerBuiltins.Serialize) {
      st = KName::Builtin_Serializer_Serialize;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseEnvBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == EnvBuiltins.GetEnvironmentVariable) {
      st = KName::Builtin_Env_GetEnvironmentVariable;
    } else if (builtin == EnvBuiltins.SetEnvironmentVariable) {
      st = KName::Builtin_Env_SetEnvironmentVariable;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseFileIOBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == FileIOBuiltIns.AppendText) {
      st = KName::Builtin_FileIO_AppendText;
    } else if (builtin == FileIOBuiltIns.ChangeDirectory) {
      st = KName::Builtin_FileIO_ChangeDirectory;
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      st = KName::Builtin_FileIO_CopyFile;
    } else if (builtin == FileIOBuiltIns.CopyR) {
      st = KName::Builtin_FileIO_CopyR;
    } else if (builtin == FileIOBuiltIns.Combine) {
      st = KName::Builtin_FileIO_Combine;
    } else if (builtin == FileIOBuiltIns.CreateFile) {
      st = KName::Builtin_FileIO_CreateFile;
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      st = KName::Builtin_FileIO_DeleteFile;
    } else if (builtin == FileIOBuiltIns.FileExists) {
      st = KName::Builtin_FileIO_FileExists;
    } else if (builtin == FileIOBuiltIns.FileName) {
      st = KName::Builtin_FileIO_FileName;
    } else if (builtin == FileIOBuiltIns.FileSize) {
      st = KName::Builtin_FileIO_FileSize;
    } else if (builtin == FileIOBuiltIns.GetCurrentDirectory) {
      st = KName::Builtin_FileIO_GetCurrentDirectory;
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      st = KName::Builtin_FileIO_GetFileAbsolutePath;
    } else if (builtin == FileIOBuiltIns.GetFileAttributes) {
      st = KName::Builtin_FileIO_GetFileAttributes;
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      st = KName::Builtin_FileIO_GetFileExtension;
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      st = KName::Builtin_FileIO_GetFilePath;
    } else if (builtin == FileIOBuiltIns.Glob) {
      st = KName::Builtin_FileIO_Glob;
    } else if (builtin == FileIOBuiltIns.IsDirectory) {
      st = KName::Builtin_FileIO_IsDirectory;
    } else if (builtin == FileIOBuiltIns.ListDirectory) {
      st = KName::Builtin_FileIO_ListDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectory) {
      st = KName::Builtin_FileIO_MakeDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectoryP) {
      st = KName::Builtin_FileIO_MakeDirectoryP;
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      st = KName::Builtin_FileIO_MoveFile;
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      st = KName::Builtin_FileIO_ReadFile;
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      st = KName::Builtin_FileIO_ReadLines;
    } else if (builtin == FileIOBuiltIns.ReadBytes) {
      st = KName::Builtin_FileIO_ReadBytes;
    } else if (builtin == FileIOBuiltIns.RemoveDirectory) {
      st = KName::Builtin_FileIO_RemoveDirectory;
    } else if (builtin == FileIOBuiltIns.RemoveDirectoryF) {
      st = KName::Builtin_FileIO_RemoveDirectoryF;
    } else if (builtin == FileIOBuiltIns.TempDir) {
      st = KName::Builtin_FileIO_TempDir;
    } else if (builtin == FileIOBuiltIns.WriteBytes) {
      st = KName::Builtin_FileIO_WriteBytes;
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      st = KName::Builtin_FileIO_WriteLine;
    } else if (builtin == FileIOBuiltIns.WriteText) {
      st = KName::Builtin_FileIO_WriteText;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseListBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ListBuiltins.Map) {
      st = KName::Builtin_List_Map;
    } else if (builtin == ListBuiltins.None) {
      st = KName::Builtin_List_None;
    } else if (builtin == ListBuiltins.Reduce) {
      st = KName::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.Select) {
      st = KName::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = KName::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.ToH) {
      st = KName::Builtin_List_ToH;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseMathBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == MathBuiltins.Abs) {
      st = KName::Builtin_Math_Abs;
    } else if (builtin == MathBuiltins.Acos) {
      st = KName::Builtin_Math_Acos;
    } else if (builtin == MathBuiltins.Asin) {
      st = KName::Builtin_Math_Asin;
    } else if (builtin == MathBuiltins.Atan) {
      st = KName::Builtin_Math_Atan;
    } else if (builtin == MathBuiltins.Atan2) {
      st = KName::Builtin_Math_Atan2;
    } else if (builtin == MathBuiltins.Cbrt) {
      st = KName::Builtin_Math_Cbrt;
    } else if (builtin == MathBuiltins.Ceil) {
      st = KName::Builtin_Math_Ceil;
    } else if (builtin == MathBuiltins.CopySign) {
      st = KName::Builtin_Math_CopySign;
    } else if (builtin == MathBuiltins.Cos) {
      st = KName::Builtin_Math_Cos;
    } else if (builtin == MathBuiltins.Cosh) {
      st = KName::Builtin_Math_Cosh;
    } else if (builtin == MathBuiltins.Divisors) {
      st = KName::Builtin_Math_Divisors;
    } else if (builtin == MathBuiltins.Epsilon) {
      st = KName::Builtin_Math_Epsilon;
    } else if (builtin == MathBuiltins.Erf) {
      st = KName::Builtin_Math_Erf;
    } else if (builtin == MathBuiltins.ErfC) {
      st = KName::Builtin_Math_ErfC;
    } else if (builtin == MathBuiltins.Exp) {
      st = KName::Builtin_Math_Exp;
    } else if (builtin == MathBuiltins.ExpM1) {
      st = KName::Builtin_Math_ExpM1;
    } else if (builtin == MathBuiltins.FDim) {
      st = KName::Builtin_Math_FDim;
    } else if (builtin == MathBuiltins.Floor) {
      st = KName::Builtin_Math_Floor;
    } else if (builtin == MathBuiltins.FMax) {
      st = KName::Builtin_Math_FMax;
    } else if (builtin == MathBuiltins.FMin) {
      st = KName::Builtin_Math_FMin;
    } else if (builtin == MathBuiltins.Fmod) {
      st = KName::Builtin_Math_Fmod;
    } else if (builtin == MathBuiltins.Hypot) {
      st = KName::Builtin_Math_Hypot;
    } else if (builtin == MathBuiltins.IsFinite) {
      st = KName::Builtin_Math_IsFinite;
    } else if (builtin == MathBuiltins.IsInf) {
      st = KName::Builtin_Math_IsInf;
    } else if (builtin == MathBuiltins.IsNaN) {
      st = KName::Builtin_Math_IsNaN;
    } else if (builtin == MathBuiltins.IsNormal) {
      st = KName::Builtin_Math_IsNormal;
    } else if (builtin == MathBuiltins.LGamma) {
      st = KName::Builtin_Math_LGamma;
    } else if (builtin == MathBuiltins.Log) {
      st = KName::Builtin_Math_Log;
    } else if (builtin == MathBuiltins.Log10) {
      st = KName::Builtin_Math_Log10;
    } else if (builtin == MathBuiltins.Log1P) {
      st = KName::Builtin_Math_Log1P;
    } else if (builtin == MathBuiltins.Log2) {
      st = KName::Builtin_Math_Log2;
    } else if (builtin == MathBuiltins.NextAfter) {
      st = KName::Builtin_Math_NextAfter;
    } else if (builtin == MathBuiltins.Pow) {
      st = KName::Builtin_Math_Pow;
    } else if (builtin == MathBuiltins.Random) {
      st = KName::Builtin_Math_Random;
    } else if (builtin == MathBuiltins.Remainder) {
      st = KName::Builtin_Math_Remainder;
    } else if (builtin == MathBuiltins.Round) {
      st = KName::Builtin_Math_Round;
    } else if (builtin == MathBuiltins.RotateLeft) {
      st = KName::Builtin_Math_RotateLeft;
    } else if (builtin == MathBuiltins.RotateRight) {
      st = KName::Builtin_Math_RotateRight;
    } else if (builtin == MathBuiltins.Sin) {
      st = KName::Builtin_Math_Sin;
    } else if (builtin == MathBuiltins.Sinh) {
      st = KName::Builtin_Math_Sinh;
    } else if (builtin == MathBuiltins.Sqrt) {
      st = KName::Builtin_Math_Sqrt;
    } else if (builtin == MathBuiltins.Tan) {
      st = KName::Builtin_Math_Tan;
    } else if (builtin == MathBuiltins.Tanh) {
      st = KName::Builtin_Math_Tanh;
    } else if (builtin == MathBuiltins.TGamma) {
      st = KName::Builtin_Math_TGamma;
    } else if (builtin == MathBuiltins.Trunc) {
      st = KName::Builtin_Math_Trunc;
    } else if (builtin == MathBuiltins.ListPrimes) {
      st = KName::Builtin_Math_ListPrimes;
    } else if (builtin == MathBuiltins.NthPrime) {
      st = KName::Builtin_Math_NthPrime;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseModuleBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ModuleBuiltins.Home) {
      st = KName::Builtin_Module_Home;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseSysBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == SysBuiltins.EffectiveUserId) {
      st = KName::Builtin_Sys_EffectiveUserId;
    } else if (builtin == SysBuiltins.Exec) {
      st = KName::Builtin_Sys_Exec;
    } else if (builtin == SysBuiltins.ExecOut) {
      st = KName::Builtin_Sys_ExecOut;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseWebClientBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == HttpBuiltins.Get) {
      st = KName::Builtin_WebClient_Get;
    } else if (builtin == HttpBuiltins.Post) {
      st = KName::Builtin_WebClient_Post;
    } else if (builtin == HttpBuiltins.Put) {
      st = KName::Builtin_WebClient_Put;
    } else if (builtin == HttpBuiltins.Delete) {
      st = KName::Builtin_WebClient_Delete;
    } else if (builtin == HttpBuiltins.Head) {
      st = KName::Builtin_WebClient_Head;
    } else if (builtin == HttpBuiltins.Options) {
      st = KName::Builtin_WebClient_Options;
    } else if (builtin == HttpBuiltins.Patch) {
      st = KName::Builtin_WebClient_Patch;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseWebServerBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == WebServerBuiltins.Get) {
      st = KName::Builtin_WebServer_Get;
    } else if (builtin == WebServerBuiltins.Post) {
      st = KName::Builtin_WebServer_Post;
    } else if (builtin == WebServerBuiltins.Listen) {
      st = KName::Builtin_WebServer_Listen;
    } else if (builtin == WebServerBuiltins.Host) {
      st = KName::Builtin_WebServer_Host;
    } else if (builtin == WebServerBuiltins.Port) {
      st = KName::Builtin_WebServer_Port;
    } else if (builtin == WebServerBuiltins.Public) {
      st = KName::Builtin_WebServer_Public;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseLoggingBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == LoggingBuiltins.Debug) {
      st = KName::Builtin_Logging_Debug;
    } else if (builtin == LoggingBuiltins.Error) {
      st = KName::Builtin_Logging_Error;
    } else if (builtin == LoggingBuiltins.FilePath) {
      st = KName::Builtin_Logging_FilePath;
    } else if (builtin == LoggingBuiltins.Info) {
      st = KName::Builtin_Logging_Info;
    } else if (builtin == LoggingBuiltins.Level) {
      st = KName::Builtin_Logging_Level;
    } else if (builtin == LoggingBuiltins.Mode) {
      st = KName::Builtin_Logging_Mode;
    } else if (builtin == LoggingBuiltins.EntryFormat) {
      st = KName::Builtin_Logging_EntryFormat;
    } else if (builtin == LoggingBuiltins.TimestampFormat) {
      st = KName::Builtin_Logging_TimestampFormat;
    } else if (builtin == LoggingBuiltins.Warn) {
      st = KName::Builtin_Logging_Warn;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseTimeBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == TimeBuiltins.AMPM) {
      st = KName::Builtin_Time_AMPM;
    } else if (builtin == TimeBuiltins.Delay) {
      st = KName::Builtin_Time_Delay;
    } else if (builtin == TimeBuiltins.EpochMilliseconds) {
      st = KName::Builtin_Time_EpochMilliseconds;
    } else if (builtin == TimeBuiltins.Hour) {
      st = KName::Builtin_Time_Hour;
    } else if (builtin == TimeBuiltins.IsDST) {
      st = KName::Builtin_Time_IsDST;
    } else if (builtin == TimeBuiltins.Minute) {
      st = KName::Builtin_Time_Minute;
    } else if (builtin == TimeBuiltins.Month) {
      st = KName::Builtin_Time_Month;
    } else if (builtin == TimeBuiltins.MonthDay) {
      st = KName::Builtin_Time_MonthDay;
    } else if (builtin == TimeBuiltins.Second) {
      st = KName::Builtin_Time_Second;
    } else if (builtin == TimeBuiltins.Ticks) {
      st = KName::Builtin_Time_Ticks;
    } else if (builtin == TimeBuiltins.TicksToMilliseconds) {
      st = KName::Builtin_Time_TicksToMilliseconds;
    } else if (builtin == TimeBuiltins.Timestamp) {
      st = KName::Builtin_Time_Timestamp;
    } else if (builtin == TimeBuiltins.FormatDateTime) {
      st = KName::Builtin_Time_FormatDateTime;
    } else if (builtin == TimeBuiltins.WeekDay) {
      st = KName::Builtin_Time_WeekDay;
    } else if (builtin == TimeBuiltins.Year) {
      st = KName::Builtin_Time_Year;
    } else if (builtin == TimeBuiltins.YearDay) {
      st = KName::Builtin_Time_YearDay;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseBuiltinMethod(const std::string& builtin) {
    if (ArgvBuiltins.is_builtin(builtin)) {
      return parseArgvBuiltin(builtin);
    } else if (ConsoleBuiltins.is_builtin(builtin)) {
      return parseConsoleBuiltin(builtin);
    } else if (EnvBuiltins.is_builtin(builtin)) {
      return parseEnvBuiltin(builtin);
    } else if (FileIOBuiltIns.is_builtin(builtin)) {
      return parseFileIOBuiltin(builtin);
    } else if (LoggingBuiltins.is_builtin(builtin)) {
      return parseLoggingBuiltin(builtin);
    } else if (ListBuiltins.is_builtin(builtin)) {
      return parseListBuiltin(builtin);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return parseMathBuiltin(builtin);
    } else if (ModuleBuiltins.is_builtin(builtin)) {
      return parseModuleBuiltin(builtin);
    } else if (SysBuiltins.is_builtin(builtin)) {
      return parseSysBuiltin(builtin);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return parseTimeBuiltin(builtin);
    } else if (WebServerBuiltins.is_builtin(builtin)) {
      return parseWebServerBuiltin(builtin);
    } else if (HttpBuiltins.is_builtin(builtin)) {
      return parseWebClientBuiltin(builtin);
    } else if (EncoderBuiltins.is_builtin(builtin)) {
      return parseEncoderBuiltin(builtin);
    } else if (SerializerBuiltins.is_builtin(builtin)) {
      return parseSerializerBuiltin(builtin);
    }

    return createToken(KTokenType::IDENTIFIER, KName::Default, builtin);
  }

  Token parseAstralBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == AstralBuiltins.BeginsWith) {
      st = KName::Builtin_Astral_BeginsWith;
    } else if (builtin == AstralBuiltins.Chars) {
      st = KName::Builtin_Astral_Chars;
    } else if (builtin == AstralBuiltins.Contains) {
      st = KName::Builtin_Astral_Contains;
    } else if (builtin == AstralBuiltins.Downcase) {
      st = KName::Builtin_Astral_Downcase;
    } else if (builtin == AstralBuiltins.Empty) {
      st = KName::Builtin_Astral_Empty;
    } else if (builtin == AstralBuiltins.EndsWith) {
      st = KName::Builtin_Astral_EndsWith;
    } else if (builtin == AstralBuiltins.HasKey) {
      st = KName::Builtin_Astral_HasKey;
    } else if (builtin == AstralBuiltins.IndexOf) {
      st = KName::Builtin_Astral_IndexOf;
    } else if (builtin == AstralBuiltins.IsA) {
      st = KName::Builtin_Astral_IsA;
    } else if (builtin == AstralBuiltins.Join) {
      st = KName::Builtin_Astral_Join;
    } else if (builtin == AstralBuiltins.Keys) {
      st = KName::Builtin_Astral_Keys;
    } else if (builtin == AstralBuiltins.Merge) {
      st = KName::Builtin_Astral_Merge;
    } else if (builtin == AstralBuiltins.Values) {
      st = KName::Builtin_Astral_Values;
    } else if (builtin == AstralBuiltins.LastIndexOf) {
      st = KName::Builtin_Astral_LastIndexOf;
    } else if (builtin == AstralBuiltins.LeftTrim) {
      st = KName::Builtin_Astral_LeftTrim;
    } else if (builtin == AstralBuiltins.Members) {
      st = KName::Builtin_Astral_Members;
    } else if (builtin == AstralBuiltins.Replace) {
      st = KName::Builtin_Astral_Replace;
    } else if (builtin == AstralBuiltins.Reverse) {
      st = KName::Builtin_Astral_Reverse;
    } else if (builtin == AstralBuiltins.RightTrim) {
      st = KName::Builtin_Astral_RightTrim;
    } else if (builtin == AstralBuiltins.Size) {
      st = KName::Builtin_Astral_Size;
    } else if (builtin == AstralBuiltins.Split) {
      st = KName::Builtin_Astral_Split;
    } else if (builtin == AstralBuiltins.Substring) {
      st = KName::Builtin_Astral_Substring;
    } else if (builtin == AstralBuiltins.ToD) {
      st = KName::Builtin_Astral_ToD;
    } else if (builtin == AstralBuiltins.ToI) {
      st = KName::Builtin_Astral_ToI;
    } else if (builtin == AstralBuiltins.ToS) {
      st = KName::Builtin_Astral_ToS;
    } else if (builtin == AstralBuiltins.ToBytes) {
      st = KName::Builtin_Astral_ToBytes;
    } else if (builtin == AstralBuiltins.ToHex) {
      st = KName::Builtin_Astral_ToHex;
    } else if (builtin == AstralBuiltins.Trim) {
      st = KName::Builtin_Astral_Trim;
    } else if (builtin == AstralBuiltins.Type) {
      st = KName::Builtin_Astral_Type;
    } else if (builtin == AstralBuiltins.Upcase) {
      st = KName::Builtin_Astral_Upcase;
    } else if (builtin == AstralBuiltins.Push) {
      st = KName::Builtin_Astral_Push;
    } else if (builtin == AstralBuiltins.Pop) {
      st = KName::Builtin_Astral_Pop;
    } else if (builtin == AstralBuiltins.Dequeue) {
      st = KName::Builtin_Astral_Dequeue;
    } else if (builtin == AstralBuiltins.Enqueue) {
      st = KName::Builtin_Astral_Enqueue;
    } else if (builtin == AstralBuiltins.Shift) {
      st = KName::Builtin_Astral_Shift;
    } else if (builtin == AstralBuiltins.Unshift) {
      st = KName::Builtin_Astral_Unshift;
    } else if (builtin == AstralBuiltins.Remove) {
      st = KName::Builtin_Astral_Remove;
    } else if (builtin == AstralBuiltins.RemoveAt) {
      st = KName::Builtin_Astral_RemoveAt;
    } else if (builtin == AstralBuiltins.Rotate) {
      st = KName::Builtin_Astral_Rotate;
    } else if (builtin == AstralBuiltins.Insert) {
      st = KName::Builtin_Astral_Insert;
    } else if (builtin == AstralBuiltins.Slice) {
      st = KName::Builtin_Astral_Slice;
    } else if (builtin == AstralBuiltins.Concat) {
      st = KName::Builtin_Astral_Concat;
    } else if (builtin == AstralBuiltins.Unique) {
      st = KName::Builtin_Astral_Unique;
    } else if (builtin == AstralBuiltins.Count) {
      st = KName::Builtin_Astral_Count;
    } else if (builtin == AstralBuiltins.Flatten) {
      st = KName::Builtin_Astral_Flatten;
    } else if (builtin == AstralBuiltins.Zip) {
      st = KName::Builtin_Astral_Zip;
    } else if (builtin == AstralBuiltins.Clear) {
      st = KName::Builtin_Astral_Clear;
    } else if (builtin == AstralBuiltins.Clone) {
      st = KName::Builtin_Astral_Clone;
    } else if (builtin == AstralBuiltins.Pretty) {
      st = KName::Builtin_Astral_Pretty;
    } else if (builtin == AstralBuiltins.Find) {
      st = KName::Builtin_Astral_Find;
    } else if (builtin == AstralBuiltins.Match) {
      st = KName::Builtin_Astral_Match;
    } else if (builtin == AstralBuiltins.Matches) {
      st = KName::Builtin_Astral_Matches;
    } else if (builtin == AstralBuiltins.MatchesAll) {
      st = KName::Builtin_Astral_MatchesAll;
    } else if (builtin == AstralBuiltins.Scan) {
      st = KName::Builtin_Astral_Scan;
    } else if (builtin == ListBuiltins.Map) {
      st = KName::Builtin_List_Map;
    } else if (builtin == ListBuiltins.Select) {
      st = KName::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = KName::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.Reduce) {
      st = KName::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.None) {
      st = KName::Builtin_List_None;
    } else if (builtin == ListBuiltins.Sum) {
      st = KName::Builtin_List_Sum;
    } else if (builtin == ListBuiltins.Min) {
      st = KName::Builtin_List_Min;
    } else if (builtin == ListBuiltins.Max) {
      st = KName::Builtin_List_Max;
    } else if (builtin == ListBuiltins.ToH) {
      st = KName::Builtin_List_ToH;
    }

    return createToken(KTokenType::IDENTIFIER, st, builtin);
  }

  Token parseIdentifier(const std::string& identifier) {
    auto st = KName::Default;
    return createToken(KTokenType::IDENTIFIER, st, identifier);
  }

  Token parseIdentifier(char initialChar) {
    std::string identifier(1, initialChar);
    char lastChar = '\0';

    if (pos > 2) {
      lastChar = source[pos - 2];
    }

    bool isCall = lastChar == '.';

    while (pos < source.length() &&
           (isalnum(source[pos]) || source[pos] == '_')) {
      identifier += getCurrentChar();
    }

    if (Keywords.is_keyword(identifier)) {
      return parseKeyword(identifier);
    } else if (TypeNames.is_typename(identifier)) {
      return parseTypeName(identifier);
    } else if (AstralBuiltins.is_builtin_method(identifier)) {
      return parseBuiltinMethod(identifier);
    } else if (isCall && AstralBuiltins.is_builtin(identifier)) {
      return parseAstralBuiltin(identifier);
    }

    return parseIdentifier(identifier);
  }

  Token parseLiteral(char initialChar) {
    std::string literal(1, initialChar);
    char lastChar = initialChar;

    while (pos < source.length() &&
           (isdigit(source[pos]) || source[pos] == '.')) {
      if (source[pos] == '.' && lastChar == '.') {
        literal.pop_back();
        --pos;
        break;
      }

      lastChar = source[pos];
      literal += getCurrentChar();
    }

    if (literal.find('.') != std::string::npos) {
      return createToken(KTokenType::LITERAL, KName::Default, literal,
                         std::stod(literal));
    } else {
      std::istringstream ss(literal);
      k_int value;
      ss >> value;
      return createToken(KTokenType::LITERAL, KName::Default, literal, value);
    }
  }

  Token parseHexLiteral() {
    std::string hexLiteral;
    getCurrentChar();  // Move past 'x'

    while (pos < source.length() && isxdigit(source[pos])) {
      hexLiteral += getCurrentChar();
    }

    std::istringstream ss(hexLiteral);
    k_int value;
    ss >> std::hex >> value;

    return createToken(KTokenType::LITERAL, KName::Default, hexLiteral, value);
  }

  Token parseBinaryLiteral() {
    std::string binaryLiteral;
    getCurrentChar();  // Move past 'b'

    while (pos < source.length() &&
           (source[pos] == '0' || source[pos] == '1')) {
      binaryLiteral += getCurrentChar();
    }

    k_int value = std::stoi(binaryLiteral, nullptr, 2);

    return createToken(KTokenType::LITERAL, KName::Default, binaryLiteral,
                       value);
  }

  Token parseOctalLiteral() {
    std::string octalLiteral;
    getCurrentChar();  // Move past 'o'

    while (pos < source.length() && source[pos] >= '0' && source[pos] <= '7') {
      octalLiteral += getCurrentChar();
    }

    k_int value = std::stoi(octalLiteral, nullptr, 8);

    return createToken(KTokenType::LITERAL, KName::Default, octalLiteral,
                       value);
  }

  Token parseString() {
    std::string str;
    bool escape = false;

    while (pos < source.length()) {
      char currentChar = source[pos];

      if (escape) {
        switch (currentChar) {
          case 'n':
            str += '\n';
            break;
          case 'r':
            str += '\r';
            break;
          case 't':
            str += '\t';
            break;
          case '\\':
            str += '\\';
            str += '\\';
            break;
          case 'b':
            str += '\b';
            break;
          case 'f':
            str += '\f';
            break;
          case '"':
            str += '"';
            break;
          default:
            str += currentChar;
        }
        escape = false;
      } else if (currentChar == '\\') {
        escape = true;
      } else if (currentChar == '"') {
        getCurrentChar();  // Move past the closing quote
        break;             // End of string
      } else if (currentChar == '$' && peek() == '{') {
        getCurrentChar();  // Skip '$'
        getCurrentChar();  // Skip '{'
        std::string interpolationExpression = parseInterpolatedExpression();
        str += interpolationExpression;
        continue;
      } else {
        str += currentChar;
      }

      getCurrentChar();
    }

    if (escape) {
      str += '\\';
    }

    return createToken(KTokenType::STRING, KName::Default, str);
  }

  Token parseBlockComment() {
    std::string comment;
    pos++;  // Skip the "/#"

    while (pos + 1 < source.length()) {
      char currentChar = getCurrentChar();
      if (currentChar == '#' && source[pos] == '/') {
        pos++;  // Skip the "#/"
        break;
      } else {
        comment += currentChar;
      }
    }

    return createToken(KTokenType::COMMENT, KName::Default, comment);
  }

  Token parseComment() {
    std::string comment;

    while (pos < source.length() && source[pos] != '\n') {
      comment += getCurrentChar();
    }

    return createToken(KTokenType::COMMENT, KName::Default, comment);
  }

  std::string parseInterpolatedExpression() {
    std::string expression;
    int braceCount = 1;

    while (pos < source.length() && braceCount > 0) {
      char currentChar = getCurrentChar();

      if (currentChar == '}' && braceCount == 1) {
        --braceCount;
        break;
      } else if (currentChar == '{') {
        ++braceCount;
      } else if (currentChar == '}') {
        --braceCount;
      }

      if (braceCount > 0) {
        expression += currentChar;
      }
    }

    return "${" + expression + "}";
  }

  Token createToken(KTokenType type, KName name, const std::string& text) {
    return Token::create(type, name, fileId, text, row, col);
  }

  Token createToken(KTokenType type, KName name, const std::string& text,
                    const k_value& value) {
    return Token::create(type, name, fileId, text, value, row, col);
  }
};

#endif
