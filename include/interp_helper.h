#ifndef KIWI_INTERPHELPER_H
#define KIWI_INTERPHELPER_H

#include <vector>
#include "math/visitor.h"
#include "objects/method.h"
#include "objects/sliceindex.h"
#include "parsing/keywords.h"
#include "parsing/lexer.h"
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "util/string.h"
#include "globals.h"
#include "stackframe.h"

struct InterpHelper {
  static bool isSliceAssignmentExpression(const k_stream& stream) {
    size_t pos = stream->position;
    const auto& tokens = stream->tokens;

    while (pos < tokens.size()) {
      if (tokens.at(pos).getType() == KTokenType::COLON ||
          tokens.at(pos).getType() == KTokenType::OPERATOR) {
        return true;
      }

      ++pos;
    }

    return false;
  }

  static bool isListExpression(const k_stream& stream) {
    size_t position = stream->position + 1;  // Skip the "["
    int bracketCount = 1;

    const auto& tokens = stream->tokens;
    const size_t tokensSize = tokens.size();

    while (position < tokensSize && bracketCount > 0) {
      const auto& token = tokens.at(position);

      switch (token.getType()) {
        case KTokenType::OPEN_BRACKET:
          ++bracketCount;
          break;

        case KTokenType::CLOSE_BRACKET:
          --bracketCount;
          break;

        case KTokenType::OPEN_BRACE: {
          int braceCount = 1;
          ++position;  // Skip the current brace
          while (position < tokensSize && braceCount > 0) {
            const auto& innerToken = tokens.at(position);
            if (innerToken.getType() == KTokenType::OPEN_BRACE) {
              ++braceCount;
            } else if (innerToken.getType() == KTokenType::CLOSE_BRACE) {
              --braceCount;
            }
            ++position;
          }
        }
          continue;

        case KTokenType::COLON:
        case KTokenType::RANGE:
          return false;

        default:
          break;
      }

      ++position;
    }

    return bracketCount == 0;
  }

  static bool isRangeExpression(const k_stream& stream) {
    size_t pos = stream->position + 1;  // Skip the "["
    const auto& tokens = stream->tokens;
    size_t size = tokens.size();
    int counter = 1;

    while (pos < size && counter > 0) {
      const auto type = tokens.at(pos++).getType();

      if (type == KTokenType::OPEN_BRACKET) {
        ++counter;
      } else if (type == KTokenType::CLOSE_BRACKET) {
        --counter;
        if (counter == 0) {
          break;
        }
      }

      if (type == KTokenType::RANGE) {
        return true;
      }
    }

    return false;
  }

  static bool hasReturnValue(k_stream stream) {
    const auto& nextToken = stream->peek();
    const auto& type = nextToken.getType();

    switch (type) {
      case KTokenType::LITERAL:
      case KTokenType::STRING:
      case KTokenType::IDENTIFIER:
      case KTokenType::OPEN_PAREN:
      case KTokenType::OPEN_BRACE:
      case KTokenType::OPEN_BRACKET:
        return true;

      case KTokenType::KEYWORD:
        return nextToken.getSubType() == KName::KW_This;

      case KTokenType::OPERATOR:
        return nextToken.getSubType() == KName::Ops_Not;

      default:
        return false;
    }
  }

  static bool shouldUpdateFrameVariables(
      const k_string& varName,
      const std::shared_ptr<CallStackFrame> nextFrame) {
    return nextFrame->hasVariable(varName);
  }

  static void updateVariablesInCallerFrame(
      const std::unordered_map<k_string, k_value>& variables,
      std::shared_ptr<CallStackFrame> callerFrame) {
    auto& frameVariables = callerFrame->variables;
    for (const auto& var : variables) {
      if (shouldUpdateFrameVariables(var.first, callerFrame)) {
        frameVariables[var.first] = var.second;
      }
    }
  }

  static k_string getTemporaryId() {
    return "temporary_" + RNG::getInstance().random16();
  }

  static std::vector<Token> collectBodyTokens(k_stream& stream) {
    std::vector<Token> tokens;
    int counter = 1;

    while (stream->canRead() && counter != 0) {
      const Token& currentToken = stream->current();
      const KName subType = currentToken.getSubType();

      if (Keywords.is_block_keyword(subType)) {
        ++counter;
      } else if (subType == KName::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();
          continue;
        }
      }

      tokens.emplace_back(std::move(currentToken));
      stream->next();
    }

    return tokens;
  }

  static void collectBodyTokens(std::vector<Token>& tokens, k_stream& stream) {
    int counter = 1;

    while (stream->canRead() && counter != 0) {
      const auto& currentToken = stream->current();
      const auto subType = currentToken.getSubType();

      if (Keywords.is_block_keyword(subType)) {
        ++counter;
      } else if (subType == KName::KW_End) {
        --counter;

        // Stop here.
        if (counter == 0) {
          stream->next();
          continue;
        }
      }

      tokens.emplace_back(std::move(currentToken));
      stream->next();
    }
  }

  static size_t findNextKeyword(k_stream stream) {
    int depth = 0;
    size_t startIndex = stream->position;

    while (stream->canRead()) {
      auto token = stream->current();
      auto type = token.getType();

      switch (type) {
        case KTokenType::OPEN_PAREN:
        case KTokenType::OPEN_BRACKET:
        case KTokenType::OPEN_BRACE:
          depth++;
          break;
        case KTokenType::CLOSE_PAREN:
        case KTokenType::CLOSE_BRACKET:
        case KTokenType::CLOSE_BRACE:
          depth--;
          if (depth == 0 && startIndex != stream->position) {
            stream->next();
            return stream->position;
          }
          break;
        case KTokenType::KEYWORD:
          if (depth == 0) {
            return stream->position;
          }
          break;
        default:
          break;
      }
      stream->next();
    }

    return stream->position;
  }

  static void updateListSlice(k_stream stream, bool insertOp,
                              k_list& targetList, const SliceIndex& slice,
                              const k_list& rhsValues) {
    if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
      throw IndexError(stream->current(), "Start index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
      throw IndexError(stream->current(), "Stop index must be an integer.");
    } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
      throw IndexError(stream->current(), "Step value must be an integer.");
    }

    int start = std::get<k_int>(slice.indexOrStart);
    int stop = std::get<k_int>(slice.stopIndex);
    int step = std::get<k_int>(slice.stepValue);

    // This is a single element assignment.
    if (!slice.isSlice && insertOp) {
      stop = start;
    }

    auto& elements = targetList->elements;
    auto& rhsElements = rhsValues->elements;

    // Convert negative indices and adjust ranges
    int listSize = static_cast<int>(elements.size());
    int rhsSize = static_cast<int>(rhsElements.size());

    start += start < 0 ? listSize : 0;
    stop += stop < 0 ? listSize : 0;
    start = start < 0 ? 0 : start;
    stop = stop > listSize ? listSize : stop;
    // Special case for reverse slicing
    stop = step < 0 && stop == listSize ? -1 : stop;

    if (step == 1) {
      // Simple case: step is 1
      if (start >= stop) {
        elements.erase(elements.begin() + start, elements.begin() + stop);
        elements.insert(elements.begin() + start, rhsElements.begin(),
                        rhsElements.end());
      } else {
        std::copy(rhsElements.begin(), rhsElements.end(),
                  elements.begin() + start);
      }
    } else {
      // Complex case: step != 1
      int rhsIndex = 0;
      for (int i = start; i != stop && rhsIndex < rhsSize; i += step) {
        if ((step > 0 && i < listSize) || (step < 0 && i >= 0)) {
          elements[i] = rhsElements.at(rhsIndex++);
        } else {
          break;  // Avoid going out of bounds
        }
      }
    }
  }

  static k_value interpretAssignOp(k_stream stream, const KName& op,
                                   const k_value& currentValue,
                                   const k_value& value) {
    switch (op) {
      case KName::Ops_AddAssign:
        return std::visit(AddVisitor(stream->current()), currentValue, value);

      case KName::Ops_SubtractAssign:
        return std::visit(SubtractVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_MultiplyAssign:
        return std::visit(MultiplyVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_DivideAssign:
        return std::visit(DivideVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_ExponentAssign:
        return std::visit(PowerVisitor(stream->current()), currentValue, value);

      case KName::Ops_ModuloAssign:
        return std::visit(ModuloVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseAndAssign:
        return std::visit(BitwiseAndVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseOrAssign:
        return std::visit(BitwiseOrVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseXorAssign:
        return std::visit(BitwiseXorVisitor(stream->current()), currentValue,
                          value);

      case KName::Ops_BitwiseLeftShiftAssign:
        return std::visit(BitwiseLeftShiftVisitor(stream->current()),
                          currentValue, value);

      case KName::Ops_BitwiseRightShiftAssign:
        return std::visit(BitwiseRightShiftVisitor(stream->current()),
                          currentValue, value);

      case KName::Ops_BitwiseNotAssign:
        return std::visit(BitwiseNotVisitor(stream->current()), value);

      default:
        break;
    }

    throw InvalidOperationError(stream->current(), "Invalid operator.");
  }

  static k_value stringSlice(k_stream stream, SliceIndex& slice,
                             const k_value& value) {
    auto string = std::get<k_string>(value);
    auto list = std::make_shared<List>();

    auto& elements = list->elements;
    for (const char& c : string) {
      elements.emplace_back(k_string(1, c));
    }

    auto sliced = listSlice(stream, slice, list);
    std::ostringstream sv;

    if (std::holds_alternative<k_list>(sliced)) {
      auto slicedlist = std::get<k_list>(sliced)->elements;
      for (auto it = slicedlist.begin(); it != slicedlist.end(); ++it) {
        sv << Serializer::serialize(*it);
      }
    } else {
      sv << Serializer::serialize(sliced);
    }

    return sv.str();
  }

  static k_value listSlice(k_stream stream, const SliceIndex& slice,
                           const k_value& value) {
    auto list = std::get<k_list>(value);
    auto& elements = list->elements;
    if (slice.isSlice) {
      if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
        throw IndexError(stream->current(), "Start index must be an integer.");
      } else if (!std::holds_alternative<k_int>(slice.stopIndex)) {
        throw IndexError(stream->current(), "Stop index must be an integer.");
      } else if (!std::holds_alternative<k_int>(slice.stepValue)) {
        throw IndexError(stream->current(), "Step value must be an integer.");
      }

      int start = static_cast<int>(std::get<k_int>(slice.indexOrStart)),
          stop = static_cast<int>(std::get<k_int>(slice.stopIndex)),
          step = static_cast<int>(std::get<k_int>(slice.stepValue));

      // Adjust negative indices
      int listSize = static_cast<int>(elements.size());
      if (start < 0) {
        start = start + listSize > 0 ? start + listSize : 0;
      }

      if (stop < 0) {
        stop += listSize;
      } else {
        stop = stop < listSize ? stop : listSize;
      }

      // Adjust stop for reverse slicing
      if (step < 0 && stop == listSize) {
        stop = -1;
      }

      auto slicedList = std::make_shared<List>();
      auto& slicedElements = slicedList->elements;

      if (step < 0) {
        for (int i = (start == 0 ? listSize - 1 : start); i >= stop;
             i += step) {
          // Prevent out-of-bounds access
          if (i < 0 || i >= listSize) {
            break;
          }

          slicedElements.emplace_back(elements.at(i));
        }
      } else {
        for (int i = start; i < stop; i += step) {
          // Prevent out-of-bounds access
          if (i >= listSize) {
            break;
          }

          slicedElements.emplace_back(elements.at(i));
        }
      }
      return slicedList;  // Return the sliced list as a k_value
    } else {
      // Single index access
      if (!std::holds_alternative<k_int>(slice.indexOrStart)) {
        throw IndexError(stream->current(), "Index value must be an integer.");
      }

      int index = std::get<k_int>(slice.indexOrStart);
      int listSize = elements.size();

      if (index < 0) {
        index += listSize;  // Adjust for negative index
      }

      if (index < 0 || index >= listSize) {
        throw RangeError(stream->current(), "List index out of range.");
      }

      return elements.at(index);
    }
  }

  static bool hasVariable(std::shared_ptr<CallStackFrame> frame,
                          const k_string& name) {
    if (frame->hasVariable(name)) {
      return true;  // Found in the current frame
    }

    if (frame->inObjectContext()) {
      if (frame->getObjectContext()->hasVariable(name)) {
        return true;
      }
    }

    // Check in outer frames
    std::stack<std::shared_ptr<CallStackFrame>> tempStack(
        callStack);  // Copy the call stack
    while (!tempStack.empty()) {
      const auto& outerFrame = tempStack.top();
      if (outerFrame->hasVariable(name)) {
        return true;  // Found in an outer frame
      }
      tempStack.pop();
    }

    return false;  // Not found in any scope
  }

  static k_value getVariable(k_stream stream,
                             std::shared_ptr<CallStackFrame> frame,
                             const k_string& name) {
    // Check in the current frame
    if (frame->hasVariable(name)) {
      return frame->variables[name];
    }

    if (frame->inObjectContext() &&
        frame->getObjectContext()->hasVariable(name)) {
      return frame->getObjectContext()->instanceVariables[name];
    }

    // Check in outer frames
    auto tempStack(callStack);
    while (!tempStack.empty()) {
      const auto& outerFrame = tempStack.top();
      if (outerFrame->hasVariable(name)) {
        return outerFrame->variables[name];
      }
      tempStack.pop();
    }

    throw VariableUndefinedError(stream->current(), name);
  }

  static std::unordered_map<k_string, k_value> trackVariables(
      k_stream stream, std::shared_ptr<CallStackFrame> frame,
      const std::string& firstValue, const std::string& secondValue) {
    std::unordered_map<k_string, k_value> restore;
    if (hasVariable(frame, firstValue)) {
      restore[firstValue] = getVariable(stream, frame, firstValue);
    }

    if (!secondValue.empty() && hasVariable(frame, secondValue)) {
      restore[secondValue] = getVariable(stream, frame, secondValue);
    }

    return restore;
  }

  static void listAppend(k_stream stream, std::shared_ptr<CallStackFrame> frame,
                         k_value& listValue, const k_string& listVariableName) {
    k_value variableValue;
    try {
      variableValue = getVariable(stream, frame, listVariableName);
    } catch (const VariableUndefinedError&) {
      throw VariableUndefinedError(stream->current(), listVariableName);
    }

    if (!std::holds_alternative<k_list>(variableValue)) {
      throw InvalidOperationError(stream->current(),
                                  "`" + listVariableName + "` is not a list.");
    }

    const auto& listPtr = std::get<k_list>(variableValue);
    listPtr->elements.emplace_back(listValue);
  }

  static void interpretParameterizedCatch(k_stream stream,
                                          std::shared_ptr<CallStackFrame> frame,
                                          k_string& errorTypeVariableName,
                                          k_string& errorVariableName,
                                          k_value& errorType,
                                          k_value& errorValue) {
    stream->next();  // Skip "("

    if (stream->current().getType() != KTokenType::IDENTIFIER) {
      throw SyntaxError(
          stream->current(),
          "Syntax error in catch variable declaration. Missing identifier.");
    }

    errorTypeVariableName = stream->current().getText();
    stream->next();  // Skip the identifier.

    if (stream->current().getType() == KTokenType::COMMA) {
      stream->next();

      if (stream->current().getType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(
            stream->current(),
            "Syntax error in catch variable declaration. Missing identifier.");
      }

      errorVariableName = stream->current().getText();
      stream->next();
    } else {
      errorVariableName = errorTypeVariableName;
      errorTypeVariableName = "";
    }

    if (stream->current().getType() != KTokenType::CLOSE_PAREN) {
      throw SyntaxError(stream->current(),
                        "Syntax error in catch variable declaration.");
    }
    stream->next();  // Skip ")"

    const auto& error = frame->getErrorState().error;
    errorType = error.getError();
    errorValue = error.getMessage();
  }

  static k_string interpretPackageHome(k_string& packagePath, k_stream stream) {
    if (stream->current().getType() != KTokenType::STRING ||
        !String::beginsWith(packagePath, "@")) {
      return "";
    }

    k_string packageHome;

    // Get everything between the @ and the /, that is the home.
    Lexer lexer("", packagePath);
    const auto& tokens = lexer.getAllTokens();
    auto lastToken = Token::createEmpty();
    size_t pos = 0;
    bool build = false;
    k_string packageName;

    while (pos < tokens.size()) {
      const auto& token = tokens.at(pos);

      // If the last token was "@"
      if (pos + 1 < tokens.size() && lastToken.getText() == "@") {
        if (tokens.at(pos + 1).getSubType() == KName::Ops_Divide) {
          packageHome = token.getText();
          pos += 2;  // Skip package home and "/"
          build = true;
          continue;
        }
      }

      if (build) {
        packageName += token.getText();
      } else {
        lastToken = token;
      }
      ++pos;
    }

    if (!packageName.empty()) {
      packagePath = packageName;
    }

    return packageHome;
  }

  static k_string interpretBaseClass(k_stream stream) {
    k_string baseClassName;
    if (stream->current().getType() == KTokenType::OPERATOR) {
      if (stream->current().getSubType() != KName::Ops_LessThan) {
        throw SyntaxError(
            stream->current(),
            "Expected inheritance operator, `<`, in class definition.");
      }
      stream->next();

      if (stream->current().getType() != KTokenType::IDENTIFIER) {
        throw SyntaxError(stream->current(), "Expected base class name.");
      }

      baseClassName = stream->current().getText();
      stream->next();  // Skip base class.
    }
    return baseClassName;
  }

  static k_string minify(const k_string& path, bool output = false) {
    auto content = File::readFile(path);
    if (content.empty()) {
      return content;
    }

    std::ostringstream builder;
    Lexer lexer(path, content);
    auto stream = Lexer(path, content).getTokenStream();
    bool addSpace = true;
    while (stream->canRead()) {
      auto token = stream->current();
      switch (token.getType()) {
        case KTokenType::COMMENT:
          stream->next();
          continue;
        case KTokenType::KEYWORD:
        case KTokenType::IDENTIFIER:
        case KTokenType::CONDITIONAL:
        case KTokenType::LITERAL:
          if (addSpace) {
            builder << ' ';
          }
          builder << token.getText();
          addSpace = true;
          break;
        case KTokenType::STRING:
          if (addSpace) {
            builder << ' ';
          }
          builder << '"' << token.getText() << '"';
          addSpace = true;
          break;
        default:
          addSpace = false;
          builder << token.getText();
          break;
      }

      stream->next();
    }

    if (output) {
      std::cout << String::trim(builder.str()) << std::endl;
    }

    return builder.str();
  }

  static void dumpState() {
    std::cout << kiwi_arg << " v" << kiwi_version << " state dump" << std::endl;

    std::cout << "streams: " << streamStack.size() << std::endl;
    int counter = 0;
    auto tempStreamStack(streamStack);
    while (!tempStreamStack.empty()) {
      std::cout << counter++
                << " stream size: " << tempStreamStack.top()->tokens.size()
                << std::endl;
      tempStreamStack.pop();
    }

    std::cout << "frames: " << callStack.size() << std::endl;
    counter = 0;
    auto tempCallStack(callStack);
    while (!tempCallStack.empty()) {
      const auto& frame = tempCallStack.top();
      const auto& frameVariables = frame->variables;
      const auto& frameLambdas = frame->lambdas;
      std::cout << counter << " frame variables: " << frameVariables.size()
                << std::endl;
      for (const auto& var : frameVariables) {
        std::cout << "  name: " << var.first << std::endl;
      }
      std::cout << counter++ << " frame lambdas: " << frameLambdas.size()
                << std::endl;
      for (const auto& lambda : frameLambdas) {
        std::cout << "  name: " << lambda.first
                  << ", size: " << lambda.second.getCode().size() << std::endl;
      }
      tempCallStack.pop();
    }

    std::cout << "packages: " << packages.size() << std::endl;
    counter = 0;
    for (const auto& mod : packages) {
      std::cout << counter++ << " name: " << mod.first
                << ", size: " << mod.second.getCode().size() << std::endl;
    }

    std::cout << "classes: " << classes.size() << std::endl;
    counter = 0;
    for (const auto& clazz : classes) {
      std::cout << counter << " name: " << clazz.first
                << ", size: " << clazz.second.getMethods().size() << std::endl;
      for (const auto& method : clazz.second.getMethods()) {
        std::cout << counter << " class method name: " << method.first
                  << ", size: " << method.second.getCode().size() << std::endl;
      }
      counter++;
    }

    std::cout << "methods: " << methods.size() << std::endl;
    counter = 0;
    for (const auto& method : methods) {
      std::cout << counter++ << " name: " << method.first
                << ", size: " << method.second.getCode().size() << std::endl;
    }

    std::cout << std::endl;
  }

  static void preloadCoreObjects(std::shared_ptr<CallStackFrame> mainFrame) {
    const k_string null_classname = "Null";
    Class null;
    null.setClassName(null_classname);
    Method ctor;
    ctor.setName(Keywords.Ctor);
    classes[null_classname] = null;

    auto nullRef = std::make_shared<Object>();
    nullRef->className = null_classname;
    mainFrame->variables[Keywords.Null] = nullRef;
  }
};

#endif