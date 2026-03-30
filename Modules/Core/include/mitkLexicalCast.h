/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkLexicalCast.h
 * \brief Provides a robust lexical cast from strings to numeric types with a fallback for edge-case values.
 *
 * This header wraps \c boost::lexical_cast and adds a fallback using \c std::istringstream
 * for certain compiler/platform combinations (e.g. Apple LLVM) that fail to convert
 * very small floating-point numbers such as \c 0.2225e-307. It also provides template
 * specializations that redirect \c boost::lexical_cast\<float\>, \c boost::lexical_cast\<double\>,
 * and \c boost::lexical_cast\<long\ double\> through the MITK implementation.
 *
 * \ingroup Core
 */

#ifndef mitkLexicalCast_h
#define mitkLexicalCast_h

#include <boost/lexical_cast.hpp>

namespace mitk
{
  /**
   * \brief Convert a string to a numeric target type with an \c std::istringstream fallback.
   *
   * First attempts conversion via \c boost::conversion::detail::try_lexical_convert.
   * If that fails (e.g. for very small floating-point values on certain compilers),
   * falls back to parsing via \c std::istringstream.
   *
   * \tparam Target The numeric type to convert to (e.g. \c float, \c double).
   * \param arg The string to convert.
   * \return The converted value of type \a Target.
   * \throw boost::bad_lexical_cast If neither conversion path succeeds.
   */
  template <typename Target>
  inline Target lexical_cast(const std::string &arg)
  {
    Target result = Target();

    // Let Boost try to do the lexical cast, which will most probably succeed!
    if (!boost::conversion::detail::try_lexical_convert(arg, result))
    {
      // Fallback to our own conversion using std::istringstream. This happens with
      // Apple LLVM version 9.1.0 (clang-902.0.39.1) on darwin17.5.0 and very small
      // floating point numbers like 0.2225e-307.
      std::istringstream stream(arg);
      stream.exceptions(std::ios::badbit);

      try
      {
        stream.unsetf(std::ios::skipws);
        stream.precision(boost::detail::lcast_precision<Target>::value);
        stream >> result;
      }
      catch (const std::ios_base::failure &)
      {
        boost::conversion::detail::throw_bad_cast<std::string, Target>();
      }
    }

    return result;
  }
}

namespace boost
{
  /**
   * \brief Specialization of boost::lexical_cast for string-to-float conversion.
   *
   * Delegates to mitk::lexical_cast\<float\> to benefit from the istringstream fallback.
   */
  template <>
  inline float lexical_cast<float, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<float>(arg);
  }

  /**
   * \brief Specialization of boost::lexical_cast for string-to-double conversion.
   *
   * Delegates to mitk::lexical_cast\<double\> to benefit from the istringstream fallback.
   */
  template <>
  inline double lexical_cast<double, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<double>(arg);
  }

  /**
   * \brief Specialization of boost::lexical_cast for string-to-long-double conversion.
   *
   * Delegates to mitk::lexical_cast\<long double\> to benefit from the istringstream fallback.
   */
  template <>
  inline long double lexical_cast<long double, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<long double>(arg);
  }
}

#endif
