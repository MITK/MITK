/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <numbers>
#include <string_view>
#include <vector>

#include <mitkFormulaParser.h>
#include <mitkFresnel.h>
#include <mitkLexicalCast.h>

namespace
{
  /* Classification is done by hand to keep tokenization locale-independent:
     formulas round-trip through scene files and must parse identically under
     any global locale a host application installs, while std::isalpha and
     friends classify per the global C locale (and are undefined for negative
     char values on top). */
  bool IsSpace(char c)
  {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
  }

  bool IsDigit(char c)
  {
    return c >= '0' && c <= '9';
  }

  bool IsAlpha(char c)
  {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
  }

  bool IsIdentifierChar(char c)
  {
    return IsAlpha(c) || IsDigit(c) || c == '_';
  }

  double Deg2Rad(double deg)
  {
    return deg * std::numbers::pi / 180.0;
  }

  double SinDeg(double t)
  {
    return std::sin(Deg2Rad(t));
  }

  double CosDeg(double t)
  {
    return std::cos(Deg2Rad(t));
  }

  double TanDeg(double t)
  {
    return std::tan(Deg2Rad(t));
  }

  /* mitk::fresnel_s/fresnel_c compute the pi/2-normalized Fresnel integrals;
     rescaling argument and result yields the unnormalized integrals
     int_0^t sin(u^2) du and int_0^t cos(u^2) du. */
  double FresnelS(double t)
  {
    const double x = t / std::sqrt(std::numbers::pi / 2.0);
    return mitk::fresnel_s(x) / std::sqrt(2.0 / std::numbers::pi);
  }

  double FresnelC(double t)
  {
    const double x = t / std::sqrt(std::numbers::pi / 2.0);
    return mitk::fresnel_c(x) / std::sqrt(2.0 / std::numbers::pi);
  }

  using UnaryFunction = double (*)(double);

  UnaryFunction ResolveFunction(std::string_view name)
  {
    static const std::map<std::string_view, UnaryFunction> functions = {
      {"abs", +[](double v) { return std::abs(v); }},
      {"exp", +[](double v) { return std::exp(v); }},
      {"sin", +[](double v) { return std::sin(v); }},
      {"cos", +[](double v) { return std::cos(v); }},
      {"tan", +[](double v) { return std::tan(v); }},
      {"sind", &SinDeg},
      {"cosd", &CosDeg},
      {"tand", &TanDeg},
      {"fresnelS", &FresnelS},
      {"fresnelC", &FresnelC}};

    const auto it = functions.find(name);
    return it != functions.end() ? it->second : nullptr;
  }
}

namespace mitk
{
  /* Compiled postfix form of a formula string. Immutable after construction
     and therefore safely shareable between FormulaParser copies. Variables
     are resolved by name on every evaluation so that changes to the variable
     map between evaluations are picked up. */
  class CompiledFormula
  {
  public:
    enum class OpCode
    {
      PushNumber,
      PushVariable,
      Add,
      Subtract,
      Multiply,
      Divide,
      Power,
      Negate,
      Call
    };

    struct Instruction
    {
      OpCode op = OpCode::PushNumber;
      double value = 0.0;
      std::string name;
      double (*function)(double) = nullptr;
    };

    CompiledFormula(std::vector<Instruction>&& instructions, std::size_t maxStackDepth)
      : m_Instructions(std::move(instructions)),
        m_MaxStackDepth(maxStackDepth)
    {
    }

    FormulaParser::ValueType Evaluate(FormulaParser& parser) const
    {
      std::vector<FormulaParser::ValueType> stack;
      stack.reserve(m_MaxStackDepth);

      const auto pop = [&stack]() {
        const auto value = stack.back();
        stack.pop_back();
        return value;
      };

      for (const auto& instruction : m_Instructions)
      {
        switch (instruction.op)
        {
          case OpCode::PushNumber:
            stack.push_back(instruction.value);
            break;

          case OpCode::PushVariable:
            stack.push_back(parser.LookupVariable(instruction.name));
            break;

          case OpCode::Add:
          {
            const auto rhs = pop();
            stack.back() += rhs;
            break;
          }

          case OpCode::Subtract:
          {
            const auto rhs = pop();
            stack.back() -= rhs;
            break;
          }

          case OpCode::Multiply:
          {
            const auto rhs = pop();
            stack.back() *= rhs;
            break;
          }

          case OpCode::Divide:
          {
            const auto rhs = pop();
            stack.back() /= rhs;
            break;
          }

          case OpCode::Power:
          {
            const auto rhs = pop();
            stack.back() = std::pow(stack.back(), rhs);
            break;
          }

          case OpCode::Negate:
            stack.back() = -stack.back();
            break;

          case OpCode::Call:
            stack.back() = instruction.function(stack.back());
            break;
        }
      }

      return stack.back();
    }

  private:
    std::vector<Instruction> m_Instructions;
    std::size_t m_MaxStackDepth;
  };
}

namespace
{
  enum class TokenKind
  {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    LParen,
    RParen,
    End
  };

  struct Token
  {
    TokenKind kind = TokenKind::End;
    std::size_t position = 0;
    std::string_view text;
    double value = 0.0;
  };

  [[noreturn]] void ThrowSyntaxError(const std::string& input, std::size_t position)
  {
    if (position < input.size())
    {
      mitkThrowException(mitk::FormulaParserException)
        << "Error while parsing '" << input << "': Unexpected character '" << input[position]
        << "' after '" << input.substr(0, position) << "'";
    }

    mitkThrowException(mitk::FormulaParserException)
      << "Error while parsing '" << input << "': Unexpected end of input";
  }

  /* Lexes one numeric literal starting at pos (a digit, or a dot directly
     followed by a digit). The exponent is only consumed when at least one
     digit follows it, so "1e" lexes as the number 1 followed by the
     identifier "e", matching the original Boost.Spirit tokenization. */
  double LexNumber(const std::string& input, std::size_t& pos)
  {
    const std::size_t start = pos;

    while (pos < input.size() && IsDigit(input[pos]))
      ++pos;

    if (pos < input.size() && input[pos] == '.')
    {
      ++pos;

      while (pos < input.size() && IsDigit(input[pos]))
        ++pos;
    }

    if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E'))
    {
      auto exponentPos = pos + 1;

      if (exponentPos < input.size() && (input[exponentPos] == '+' || input[exponentPos] == '-'))
        ++exponentPos;

      if (exponentPos < input.size() && IsDigit(input[exponentPos]))
      {
        pos = exponentPos + 1;

        while (pos < input.size() && IsDigit(input[pos]))
          ++pos;
      }
    }

    /* The lexer defines the token extent, so conversion failures are limited
       to exotic cases; out-of-range literals yield a huge/zero value inside
       LexicalCast, which is fine. */
    try
    {
      return mitk::LexicalCast<double>(input.substr(start, pos - start));
    }
    catch (const mitk::BadLexicalCast&)
    {
      ThrowSyntaxError(input, start);
    }
  }

  std::vector<Token> Tokenize(const std::string& input)
  {
    std::vector<Token> tokens;
    std::size_t pos = 0;

    while (pos < input.size())
    {
      const char c = input[pos];

      if (IsSpace(c))
      {
        ++pos;
      }
      else if (IsDigit(c) || (c == '.' && pos + 1 < input.size() && IsDigit(input[pos + 1])))
      {
        Token token;
        token.kind = TokenKind::Number;
        token.position = pos;
        token.value = LexNumber(input, pos);
        tokens.push_back(token);
      }
      else if (IsAlpha(c))
      {
        const std::size_t start = pos;

        do
        {
          ++pos;
        } while (pos < input.size() && IsIdentifierChar(input[pos]));

        Token token;
        token.kind = TokenKind::Identifier;
        token.position = start;
        token.text = std::string_view(input).substr(start, pos - start);
        tokens.push_back(token);
      }
      else
      {
        auto kind = TokenKind::End;

        switch (c)
        {
          case '+': kind = TokenKind::Plus; break;
          case '-': kind = TokenKind::Minus; break;
          case '*': kind = TokenKind::Star; break;
          case '/': kind = TokenKind::Slash; break;
          case '^': kind = TokenKind::Caret; break;
          case '(': kind = TokenKind::LParen; break;
          case ')': kind = TokenKind::RParen; break;
          default: ThrowSyntaxError(input, pos);
        }

        Token token;
        token.kind = kind;
        token.position = pos;
        tokens.push_back(token);
        ++pos;
      }
    }

    Token endToken;
    endToken.kind = TokenKind::End;
    endToken.position = input.size();
    tokens.push_back(endToken);

    return tokens;
  }

  /* Formulas come from user input, CLI arguments, and scene files; the
     nesting depth is capped instead of risking a stack overflow. */
  constexpr std::size_t maxNestingDepth = 500;

  /* Recursive-descent parser that emits the postfix instructions directly.
     Grammar (like the original Boost.Spirit grammar):

       expression = term (('+' | '-') term)*
       term       = factor (('*' | '/') factor)*
       factor     = primary ('^' primary)*
       primary    = number | '(' expression ')' | '-' primary | '+' primary
                  | function '(' expression ')' | variable */
  class Parser
  {
  public:
    explicit Parser(const std::string& input)
      : m_Input(input),
        m_Tokens(Tokenize(input))
    {
    }

    mitk::CompiledFormula Run()
    {
      this->ParseExpression();
      this->Expect(TokenKind::End);

      return mitk::CompiledFormula(std::move(m_Instructions), m_MaxStackDepth);
    }

  private:
    using OpCode = mitk::CompiledFormula::OpCode;

    const Token& Current() const
    {
      return m_Tokens[m_Position];
    }

    const Token& Next() const
    {
      return m_Tokens[m_Position + 1];
    }

    void Advance()
    {
      ++m_Position;
    }

    void Expect(TokenKind kind)
    {
      if (this->Current().kind != kind)
        ThrowSyntaxError(m_Input, this->Current().position);

      this->Advance();
    }

    void Emit(mitk::CompiledFormula::Instruction&& instruction)
    {
      const auto op = instruction.op;
      m_Instructions.push_back(std::move(instruction));

      if (op == OpCode::PushNumber || op == OpCode::PushVariable)
      {
        ++m_StackDepth;
        m_MaxStackDepth = std::max(m_MaxStackDepth, m_StackDepth);
      }
      else if (op != OpCode::Negate && op != OpCode::Call)
      {
        --m_StackDepth; // binary operators pop two operands and push one
      }
    }

    void Emit(OpCode op)
    {
      mitk::CompiledFormula::Instruction instruction;
      instruction.op = op;
      this->Emit(std::move(instruction));
    }

    void EmitNumber(double value)
    {
      mitk::CompiledFormula::Instruction instruction;
      instruction.op = OpCode::PushNumber;
      instruction.value = value;
      this->Emit(std::move(instruction));
    }

    void EmitVariable(std::string name)
    {
      mitk::CompiledFormula::Instruction instruction;
      instruction.op = OpCode::PushVariable;
      instruction.name = std::move(name);
      this->Emit(std::move(instruction));
    }

    void EmitCall(UnaryFunction function)
    {
      mitk::CompiledFormula::Instruction instruction;
      instruction.op = OpCode::Call;
      instruction.function = function;
      this->Emit(std::move(instruction));
    }

    void ParseExpression()
    {
      this->ParseTerm();

      while (this->Current().kind == TokenKind::Plus || this->Current().kind == TokenKind::Minus)
      {
        const auto op = this->Current().kind == TokenKind::Plus ? OpCode::Add : OpCode::Subtract;
        this->Advance();
        this->ParseTerm();
        this->Emit(op);
      }
    }

    void ParseTerm()
    {
      this->ParseFactor();

      while (this->Current().kind == TokenKind::Star || this->Current().kind == TokenKind::Slash)
      {
        const auto op = this->Current().kind == TokenKind::Star ? OpCode::Multiply : OpCode::Divide;
        this->Advance();
        this->ParseFactor();
        this->Emit(op);
      }
    }

    void ParseFactor()
    {
      this->ParsePrimary();

      // Deliberately folds left-associatively (2^3^2 == 64) like the original
      // grammar; formulas stored in scene files rely on this.
      while (this->Current().kind == TokenKind::Caret)
      {
        this->Advance();
        this->ParsePrimary();
        this->Emit(OpCode::Power);
      }
    }

    void ParsePrimary()
    {
      if (m_Depth >= maxNestingDepth)
      {
        mitkThrowException(mitk::FormulaParserException)
          << "Error while parsing formula: Expression is nested too deeply";
      }

      ++m_Depth;

      const Token& token = this->Current();

      switch (token.kind)
      {
        case TokenKind::Number:
          this->EmitNumber(token.value);
          this->Advance();
          break;

        case TokenKind::LParen:
          this->Advance();
          this->ParseExpression();
          this->Expect(TokenKind::RParen);
          break;

        case TokenKind::Minus:
          this->Advance();
          this->ParsePrimary();
          this->Emit(OpCode::Negate);
          break;

        case TokenKind::Plus:
          this->Advance();
          this->ParsePrimary();
          break;

        case TokenKind::Identifier:
          this->ParseFunctionOrVariable(token);
          break;

        default:
          ThrowSyntaxError(m_Input, token.position);
      }

      --m_Depth;
    }

    void ParseFunctionOrVariable(const Token& token)
    {
      // A known function name is only a function call when followed by '(';
      // otherwise it is an ordinary variable, as in the original grammar.
      const auto function =
        this->Next().kind == TokenKind::LParen ? ResolveFunction(token.text) : nullptr;

      if (function != nullptr)
      {
        this->Advance(); // function name
        this->Advance(); // '('
        this->ParseExpression();
        this->Expect(TokenKind::RParen);
        this->EmitCall(function);
      }
      else
      {
        this->EmitVariable(std::string(token.text));
        this->Advance();
      }
    }

    const std::string& m_Input;
    std::vector<Token> m_Tokens;
    std::vector<mitk::CompiledFormula::Instruction> m_Instructions;
    std::size_t m_Position = 0;
    std::size_t m_Depth = 0;
    std::size_t m_StackDepth = 0;
    std::size_t m_MaxStackDepth = 0;
  };
}

namespace mitk
{
  FormulaParser::FormulaParser(const VariableMapType* variables) : m_Variables(variables)
  {
  }

  FormulaParser::ValueType FormulaParser::Parse(const std::string& input)
  {
    if (!m_CachedFormula || input != m_CachedInput)
    {
      // Compile before touching the cache so that it stays consistent when
      // compilation throws.
      auto compiledFormula = std::make_shared<const CompiledFormula>(Parser(input).Run());
      m_CachedInput = input;
      m_CachedFormula = std::move(compiledFormula);
    }

    return m_CachedFormula->Evaluate(*this);
  }

  FormulaParser::ValueType FormulaParser::LookupVariable(const std::string var)
  {
    if (m_Variables == nullptr)
    {
      mitkThrowException(FormulaParserException) << "Map of variables is empty";
    }

    try
    {
      return m_Variables->at(var);
    }
    catch (std::out_of_range&)
    {
      mitkThrowException(FormulaParserException) << "No variable '" << var << "' defined in lookup";
    }
  }
}
