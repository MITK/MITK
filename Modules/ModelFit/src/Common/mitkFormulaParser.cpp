/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <boost/math/constants/constants.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/version.hpp>

#include "mitkFormulaParser.h"
#include "mitkFresnel.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;

typedef std::string::const_iterator Iter;
typedef ascii::space_type Skipper;

namespace qi = boost::spirit::qi;

namespace mitk
{
  /*!
   *	@brief			Transforms the given number from degrees to radians and returns it.
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] deg	A scalar value in degrees.
   *	@return			The given value in radians.
   */
  template<typename T>
  inline T deg2rad(const T deg)
  {
    return deg * boost::math::constants::pi<T>() / static_cast<T>(180);
  }

  /*!
   *	@brief			Returns the cosine of the given degree scalar.
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] t	A scalar value in degrees whose cosine should be returned.
   *	@return			The cosine of the given degree scalar.
   */
  template<typename T>
  inline T cosd(const T t)
  {
    return std::cos(deg2rad(t));
  }

  /*!
   *	@brief			Returns the sine of the given degree scalar.
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] t	A scalar value in degrees whose sine should be returned.
   *	@return			The sine of the given degree scalar.
   */
  template<typename T>
  inline T sind(const T t)
  {
    return std::sin(deg2rad(t));
  }

  /*!
   *	@brief			Returns the tangent of the given degree scalar.
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] t	A scalar value in degrees whose tangent should be returned.
   *	@return			The tangent of the given degree scalar.
   */
  template<typename T>
  inline T tand(const T t)
  {
    return std::tan(deg2rad(t));
  }

  /*!
   *	@brief			Returns the fresnel integral sine at the given x-coordinate.
   *	@details		Code for "fresnel_s()" (fresnel.cpp and fresnel.h) taken as-is from the GNU
   *					Scientific Library (https://www.gnu.org/software/gsl/).
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] t	The x-coordinate at which the fresnel integral sine should be returned.
   *	@return			The fresnel integral sine at the given x-coordinate.
   */
  template<typename T>
  T fresnelS(const T t)
  {
    T x = t / boost::math::constants::root_half_pi<T>();
    return static_cast<T>(fresnel_s(x) / boost::math::constants::root_two_div_pi<T>());
  }

  /*!
   *	@brief			Returns the fresnel integral cosine at the given x-coordinate.
   *	@details		Code for "fresnel_c()" (fresnel.cpp and fresnel.h) taken as-is from the GNU
   *					Scientific Library (https://www.gnu.org/software/gsl/).
   *	@tparam T		The scalar type that represents a value (e.g. double).
   *	@param[in] t	The x-coordinate at which the fresnel integral cosine should be returned.
   *	@return			The fresnel integral cosine at the given x-coordinate.
   */
  template<typename T>
  T fresnelC(const T t)
  {
    T x = t / boost::math::constants::root_half_pi<T>();
    return static_cast<T>(fresnel_c(x) / boost::math::constants::root_two_div_pi<T>());
  }

  /*!
   *	@brief		The grammar that defines the language (i.e. what is allowed) for the parser.
   */
  class Grammar : public qi::grammar<Iter, FormulaParser::ValueType(), Skipper>
  {
    /*!
     *	@brief	Helper structure that makes it easier to dynamically call any
     *			one-parameter-function by overloading the @c () operator.
     */
    struct func1_
    {
      // Required for Phoenix 3+
      template<typename Sig>
      struct result;

      /*!
       *	@brief				Helper structure that is needed for compatibility with
       *						@c boost::phoenix.
       *	@tparam Functor		Type of the functor (this struct).
       *	@tparam Function	Type of the function that should be called.
       *	@tparam Arg1		Type of the argument the function should be called with.
       *
       */
      template<typename Functor, typename Function, typename Arg1>
      struct result<Functor(Function, Arg1&)>
      {
        /*! @brief The result structure always needs this typedef */
        typedef Arg1 type;
      };

      /*!
       *	@brief				Calls the function @b f with the argument @b a1 and returns the
       *						result.
       *						The result always has the same type as the argument.
       *	@tparam Function	Type of the function that should be called.
       *	@tparam Arg1			Type of the argument the function should be called with.
       *	@param[in] f		The function that should be called.
       *	@param[in] a1		The argument the function should be called with.
       *	@return				The result of the called function.
       */
      template<typename Function, typename Arg1>
      Arg1 operator()(const Function f, const Arg1 a1) const
      {
        return f(a1);
      }
    };

    /*!
     *	@brief	Helper structure that maps strings to function calls so that parsing e.g.
     *			@c "cos(0)" actually calls the @c std::cos function with parameter @c 1 so it
     *			returns @c 0.
     */
    class unaryFunction_ :
      public qi::symbols<typename std::iterator_traits<Iter>::value_type, FormulaParser::ValueType(*)(FormulaParser::ValueType)>
    {
    public:
      /*!
       *	@brief Constructs the structure, this is where the mapping takes place.
       */
      unaryFunction_()
      {
        this->add
        ("abs", static_cast<FormulaParser::ValueType(*)(FormulaParser::ValueType)>(&std::abs))
          ("exp", static_cast<FormulaParser::ValueType(*)(FormulaParser::ValueType)>(&std::exp)) // @TODO: exp ignores division by zero
          ("sin", static_cast<FormulaParser::ValueType(*)(FormulaParser::ValueType)>(&std::sin))
          ("cos", static_cast<FormulaParser::ValueType(*)(FormulaParser::ValueType)>(&std::cos))
          ("tan", static_cast<FormulaParser::ValueType(*)(FormulaParser::ValueType)>(&std::tan))
          ("sind", &sind)
          ("cosd", &cosd)
          ("tand", &tand)
          ("fresnelS", &fresnelS)
          ("fresnelC", &fresnelC);
      }
    } unaryFunction;

  public:
    /*!
     *	@brief							Constructs the grammar with the given formula parser.
     *	@param[in, out] formulaParser	The formula parser this grammar is for - so it can
     *									access its variable map.
     */
    Grammar(FormulaParser& formulaParser) : Grammar::base_type(start)
    {
      using qi::_val;
      using qi::_1;
      using qi::_2;
      using qi::char_;
      using qi::alpha;
      using qi::alnum;
      using qi::double_;
      using qi::as_string;

      phx::function<func1_> func1;

      start = expression > qi::eoi;

      expression = term[_val = _1]
        >> *(('+' >> term[_val += _1])
          | ('-' >> term[_val -= _1]));

      term = factor[_val = _1]
        >> *(('*' >> factor[_val *= _1])
          | ('/' >> factor[_val /= _1]));

      factor = primary[_val = _1];
      /*!	@TODO:	Repair exponentiation */
      //>> *('^' >> factor[phx::bind<FormulaParser::ValueType, FormulaParser::ValueType, FormulaParser::ValueType>(std::pow, _val, _1)]);

      variable = as_string[alpha >> *(alnum | char_('_'))]
        [_val = phx::bind(&FormulaParser::lookupVariable, &formulaParser, _1)];

      primary = double_[_val = _1]
        | '(' >> expression[_val = _1] >> ')'
        | ('-' >> primary[_val = -_1])
        | ('+' >> primary[_val = _1])
        | (unaryFunction >> '(' >> expression >> ')')[_val = func1(_1, _2)]
        | variable[_val = _1];
    }

    /*! the rules of the grammar. */
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> start;
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> expression;
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> term;
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> factor;
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> variable;
    qi::rule<Iter, FormulaParser::ValueType(), Skipper> primary;
  };


  FormulaParser::FormulaParser(const VariableMapType* variables) : m_Variables(variables)
  {}

  FormulaParser::ValueType FormulaParser::parse(const std::string& input)
  {
    std::string::const_iterator iter = input.begin();
    std::string::const_iterator end = input.end();
    FormulaParser::ValueType result = static_cast<FormulaParser::ValueType>(0);

    try
    {
      if (!qi::phrase_parse(iter, end, Grammar(*this), ascii::space, result))
      {
        mitkThrowException(FormulaParserException) << "Could not parse '" << input <<
          "': Grammar could not be applied to the input " << "at all.";
      }
    }
    catch (qi::expectation_failure<Iter>& e)
    {
      std::string parsed = "";

      for (Iter i = input.begin(); i != e.first; i++)
      {
        parsed += *i;
      }
      mitkThrowException(FormulaParserException) << "Error while parsing '" << input <<
        "': Unexpected character '" << *e.first << "' after '" << parsed << "'";
    }

    return result;
  };

  FormulaParser::ValueType FormulaParser::lookupVariable(const std::string var)
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
  };

}
