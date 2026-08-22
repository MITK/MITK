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
 * libc++ (Apple/macOS) ships the floating-point \c \<charconv\> overloads only
 * in very recent releases. \c boost::lexical_cast handles integers, normal
 * floats, "inf"/"-inf"/"nan" (any case) correctly on every platform, and is
 * header-only, so it adds no compiled Boost library or linkage cost.
 *
 * Two caveats, both handled here. First, boost::lexical_cast converts through
 * streams that consult the GLOBAL C++ locale: under a locale whose numpunct
 * groups thousands with '.' (de_DE style), "1.234" would silently parse as
 * 1234. The wrappers therefore use boost::lexical_cast only while the global
 * locale is the classic one (the normal case) and otherwise convert through
 * explicitly classic-imbued streams, with the special values "inf"/"infinity"
 * and "nan" handled by hand because stream extraction does not parse them
 * portably. Second, boost::lexical_cast, like libc++'s stream \c num_get,
 * rejects the smallest subnormals on Apple/libc++ (e.g. \c denorm_min()),
 * reporting the underflow as an error; the same classic-locale stream
 * conversion rescues those. Revisit once std::from_chars for floating-point
 * types is universally available.
 *
 * \ingroup Core
 */

#ifndef mitkLexicalCast_h
#define mitkLexicalCast_h

#include <mitkExceptionMacro.h>

#include <boost/lexical_cast.hpp>

#include <limits>
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
    if (std::locale() == std::locale::classic())
    {
      try
      {
        return boost::lexical_cast<Target>(arg);
      }
      catch (const boost::bad_lexical_cast &)
      {
        // Fall through to the stream conversion below, which rescues the
        // smallest subnormals that boost::lexical_cast rejects on
        // Apple/libc++ (reported as an underflow error).
      }
    }
    else if constexpr (std::numeric_limits<Target>::has_infinity)
    {
      // boost::lexical_cast is bypassed completely: it converts through the
      // non-classic global locale. It would have handled the special values,
      // which stream extraction does not parse portably, so they are handled
      // by hand (ASCII-only case folding on purpose).
      std::string token = arg;
      for (auto &c : token)
      {
        if (c >= 'A' && c <= 'Z')
          c = static_cast<char>(c - 'A' + 'a');
      }

      Target sign = static_cast<Target>(1);
      if (!token.empty() && ('+' == token.front() || '-' == token.front()))
      {
        if ('-' == token.front())
          sign = static_cast<Target>(-1);

        token.erase(0, 1);
      }

      if ("inf" == token || "infinity" == token)
        return sign * std::numeric_limits<Target>::infinity();

      if ("nan" == token)
        return std::numeric_limits<Target>::quiet_NaN();
    }

    // A classic-locale istringstream yields the value even for underflowing
    // subnormals: the underflow sets failbit, but the whole string is
    // consumed, so the stream reaches eof. An ordinary parse failure (garbage,
    // trailing characters, empty input) also sets failbit but stops short of
    // eof, so require full consumption instead of trusting failbit.
    Target result{};
    std::istringstream stream{arg};
    stream.imbue(std::locale::classic());
    stream.unsetf(std::ios::skipws);
    stream >> result;

    if (arg.empty() || !stream.eof())
      mitkThrowException(BadLexicalCast) << "Cannot interpret \"" << arg << "\" as a number";

    return result;
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
    if (std::locale() == std::locale::classic())
      return boost::lexical_cast<std::string>(value);

    // boost::lexical_cast writes through the non-classic global locale, which
    // would apply digit grouping and its decimal separator. The precision
    // matches the round-trip precision boost::lexical_cast uses.
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream.precision(std::numeric_limits<Source>::max_digits10);
    stream << value;
    return stream.str();
  }
}

#endif
