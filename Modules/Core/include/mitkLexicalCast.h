/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkLexicalCast_h
#define mitkLexicalCast_h

#include <boost/lexical_cast.hpp>

namespace mitk
{
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
        stream.precision(boost::detail::lcast_get_precision<Target>());
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
  template <>
  inline float lexical_cast<float, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<float>(arg);
  }

  template <>
  inline double lexical_cast<double, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<double>(arg);
  }

  template <>
  inline long double lexical_cast<long double, std::string>(const std::string &arg)
  {
    return mitk::lexical_cast<long double>(arg);
  }
}

#endif
