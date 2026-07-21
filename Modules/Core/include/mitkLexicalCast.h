/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkLexicalCast.h
 * \brief Locale-independent conversion between strings and numeric types.
 *
 * These wrappers are intentionally backed by \c boost::lexical_cast rather than
 * \c std::from_chars / \c std::to_chars. The standard route looks like the
 * obvious replacement, but its floating-point support is not portable yet:
 * libc++ (Apple/macOS) ships the floating-point \c <charconv> overloads only in
 * very recent releases. \c boost::lexical_cast handles integers, normal floats,
 * "inf"/"-inf"/"nan" (any case) correctly and locale-independently on every
 * platform, and is header-only, so it adds no compiled Boost library or linkage
 * cost.
 *
 * One caveat: boost::lexical_cast, like libc++'s stream \c num_get, rejects the
 * smallest subnormals on Apple/libc++ (e.g. \c denorm_min()), reporting the
 * underflow as an error. LexicalCast therefore falls back to a classic-locale
 * \c std::istringstream that tolerates the underflow. Revisit once
 * std::from_chars for floating-point types is universally available.
 *
 * \ingroup Core
 */

#ifndef mitkLexicalCast_h
#define mitkLexicalCast_h

#include <mitkExceptionMacro.h>

#include <boost/lexical_cast.hpp>

#include <locale>
#include <sstream>
#include <string>

namespace mitk
{
  /**
   * \brief Thrown when a string cannot be interpreted as the requested numeric type.
   */
  class BadLexicalCast : public Exception
  {
  public:
    mitkExceptionClassMacro(BadLexicalCast, Exception);
  };

  /**
   * \brief Convert a string to a numeric target type (locale-independent).
   *
   * "inf"/"infinity" and "nan" are recognized case-independently, matching the
   * output of \ref ToString.
   *
   * \tparam Target The numeric type to convert to (e.g. \c float, \c double, \c int).
   * \param arg The string to convert.
   * \return The converted value of type \a Target.
   * \throw BadLexicalCast If the string is not a valid representation of \a Target.
   */
  template <typename Target>
  inline Target LexicalCast(const std::string &arg)
  {
    try
    {
      return boost::lexical_cast<Target>(arg);
    }
    catch (const boost::bad_lexical_cast &)
    {
      // boost::lexical_cast fails for the smallest subnormals on Apple/libc++
      // (it reports the underflow as an error). A classic-locale istringstream
      // still yields the value; the underflow raises failbit, which we tolerate,
      // treating only badbit as a genuine stream error.
      Target result{};
      std::istringstream stream{arg};
      stream.imbue(std::locale::classic());
      stream.exceptions(std::ios::badbit);
      stream.unsetf(std::ios::skipws);

      try
      {
        stream >> result;
      }
      catch (const std::ios_base::failure &)
      {
        mitkThrowException(BadLexicalCast) << "Cannot interpret \"" << arg << "\" as a number";
      }

      return result;
    }
  }

  /**
   * \brief Convert a numeric value to its string representation (locale-independent).
   *
   * Infinity and NaN are written as "inf", "-inf" and "nan". The result reads
   * back to the original value via \ref LexicalCast.
   *
   * \tparam Source The numeric type to convert from.
   * \param value The value to convert.
   * \return The string representation of \a value.
   */
  template <typename Source>
  inline std::string ToString(const Source &value)
  {
    return boost::lexical_cast<std::string>(value);
  }
}

#endif
