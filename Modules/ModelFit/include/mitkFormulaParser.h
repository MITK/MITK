/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFormulaParser_h
#define mitkFormulaParser_h

#include <map>
#include <memory>
#include <string>

#include <mitkExceptionMacro.h>

#include <MitkModelFitExports.h>

namespace mitk
{
  class CompiledFormula;

  /**
   * \brief Exception class for all exceptions generated in the FormulaParser module.
   */
  class MITKMODELFIT_EXPORT FormulaParserException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(FormulaParserException, mitk::Exception);
  };


  /**
   * \class FormulaParser
   * \brief Evaluates simple mathematical formula strings (e.g. "3.5 + 4 * x * sin(x) - 1 / 2").
   *
   * Function strings (e.g. "sin", "tan" or "abs") are translated to
   * actual function calls and variables (e.g. "x", "myVariable", "amount_")
   * are replaced by their currently assigned value via a look-up table.
   *
   * The parser is able to recognize:
   * - numeric literals with optional decimal point and exponent (1.5, .5, 1e-3)
   * - sums, differences, products and divisions (a + b, 4 - 3, 2 * x, 9 / 3)
   * - algebraic signs (+5, -5)
   * - exponentiation (2 ^ 4)
   * - parentheses (3 * (4 + 2))
   * - variables (x, myVar, test2_var_)
   * - the following unary functions: abs, exp, sin, cos, tan,
   *   sind (sine in degrees), cosd (cosine in degrees), tand (tangent in
   *   degrees), fresnelS and fresnelC (Fresnel integrals)
   *
   * Identifiers must be contiguous; whitespace is only skipped between tokens.
   *
   * In order to use the FormulaParser you just have to initialize it with a map of
   * variables (i.e. a look-up table where "x" is assigned to 5 for example and
   * "y" is assigned to 13 and so on) and then call the
   * FormulaParser::Parse function with the string that should be evaluated. Be
   * sure to update the look-up table every time a variable's value changes since that
   * is not done automatically.
   *
   * The formula is compiled on first evaluation and the compiled form is
   * cached, so calling Parse() repeatedly with the same string only
   * re-evaluates it against the current variable values. Variables are looked
   * up by name on every evaluation, so the map may be modified between calls.
   * Instances are not thread-safe; use one instance per thread.
   *
   * \sa GenericParamModel
   */
  class MITKMODELFIT_EXPORT FormulaParser
  {
  public:
    using ValueType = double;
    using VariableMapType = std::map<std::string, ValueType>;

    /**
     * \brief Constructs the FormulaParser and initializes the variables.
     * \param[in] variables A map of variables with values. The map is delivered as a
     * pointer so you can still change it from outside this class.
     */
    FormulaParser(const VariableMapType* variables);

    /**
     * \brief Evaluates the input string and returns the resulting value.
     * \param[in] input The string to be evaluated.
     * \return The number that results from the evaluated string.
     * \throw FormulaParserException If the parser encounters an unexpected character,
     * a variable cannot be found in the look-up table, or the grammar cannot be applied.
     */
    ValueType Parse(const std::string& input);

    /**
     * \brief Looks up the associated value of the given variable name.
     * \param[in] var The name of the variable whose value is to be returned.
     * \return The associated value of the given variable name.
     * \throw FormulaParserException If the variable map is empty or the variable name
     * cannot be found.
     */
    ValueType LookupVariable(const std::string var);

  private:
    /** \brief Map that holds the values that will replace the variables during evaluation. */
    const VariableMapType* m_Variables;

    /** \brief Input string the currently cached compiled formula was created from. */
    std::string m_CachedInput;

    /** \brief Compiled form of m_CachedInput; reused as long as the input string stays the same. */
    std::shared_ptr<const CompiledFormula> m_CachedFormula;
  };
}

#endif
