/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFormulaParser_h
#define mitkFormulaParser_h

#include <string>
#include <map>

#include "mitkExceptionMacro.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /*!
  *	@brief	Exception class for all exceptions that are generated in the FormulaParser module.
  */
  class MITKMODELFIT_EXPORT FormulaParserException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(FormulaParserException, mitk::Exception);
  };


  /*!
   *	@brief		This class offers the functionality to evaluate simple mathematical formula
   *				strings (e.g. <code>"3.5 + 4 * x * sin(x) - 1 / 2"</code>).
   *	@details	Internally it utilizes the @c boost::spirit framework with the Grammar
   *				structure to parse the input string into a valid result number.
   *				Function strings (e.g. @c "sin", @c "tan" or @c "abs") are translated to
   *				actual function calls and variables (e.g. @c "x", @c "myVariable", "amount_")
   *				are replaced by their currently assigned value via a look-up table.
   *
   *				The parser is able to recognize:
   *				@li sums, differences, products and divisions (<code>a + b</code>,
   *					<code>4 - 3</code>, <code>2 * x</code>, <code>9 / 3</code>)
   *				@li algebraic signs (@c +5, @c -5)
   *				@li exponentiation (<code>2 ^ 4</code>)
   *				@li parentheses (<code>3 * (4 + 2)</code>)
   *				@li variables (@c x, @c myVar, @c test2_var_)
   *				@li the following unary functions: @c abs, @c exp, @c sin, @c cos, @c tan,
   *					@c sind (sine in degrees), @c cosd (cosine in degrees), @c tand (tangent in
   *					degrees)
   *
   *				In order to use the FormulaParser you just have to initialize it with a map of
   *				variables (i.e. a look-up table where @c "x" is assigned to @c 5 for example and
   *				@c "y" is assigned to @c 13 and so on) and then call the
   *				@ref FormulaParser::parse function with the string that should be evaluated. Be
   *				sure to update the look-up table everytime a variable's value changes since that
   *				is not done automatically.
   *
   *	@author		Sascha Diatschuk
   */
  class MITKMODELFIT_EXPORT FormulaParser
  {
  public:
    using ValueType = double;
    using VariableMapType = std::map<std::string, ValueType>;
    /*!
     *	@brief					Construct the FormulaParser and initialized the variables with
     *							@b variables.
     *	@param[in] variables	A map of variables with values that will be assigned to the
     *							corresponding member variable. The map is delivered as a
     *							pointer so you can still change it from outside this class.
     */
    FormulaParser(const VariableMapType* variables);

    /*!
     *	@brief				Evaluates the @b input string and returns the resulting
     *						value.
     *	@param[in] input	The string to be evaluated.
     *	@return				The number that results from the evaluated string.
     *	@throw FormulaParserException	If
     *						@li the parser comes across an unexpected character,
     *						@li a variable in the input string could not be found in the look-up
     *						table or
     *						@li the parser cannot apply the grammar to the string at all.
     */
    ValueType parse(const std::string& input);

    /*!
     *	@brief			Looks up the associated value of the given string @b var in the
     *					variables map.
     *	@param[in] var	The name of the variable whose value is to be returned.
     *	@return			The associated value of the given variable name.
     *	@throw FormulaParserException	If the variable map is empty or the given variable name
     *					cannot be found.
     */
    ValueType lookupVariable(const std::string var);

  private:
    /*! @brief Map that holds the values that will replace the variables during evaluation. */
    const VariableMapType* m_Variables;
  };
}

#endif
