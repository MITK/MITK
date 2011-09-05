/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITK_SERVICE_PROPERTIES_H
#define MITK_SERVICE_PROPERTIES_H

#include <cctype>

#include <mitkConfig.h>

#ifdef MITK_HAS_UNORDERED_MAP_H
#include <unordered_map>
#else
#include "mitkItkHashMap.h"
#endif

#include <mitkCommon.h>
#include "mitkAny.h"

namespace mitk {

struct ci_char_traits : public std::char_traits<char>
    // just inherit all the other functions
    //  that we don't need to override
{
    
  static bool eq(char c1, char c2)
  {
    return std::toupper(c1) == std::toupper(c2);
  }

  static bool ne(char c1, char c2)
  {
    return std::toupper(c1) != std::toupper(c2);
  }

  static bool lt(char c1, char c2)
  {
    return std::toupper(c1) < std::toupper(c2);
  }

  static bool gt(char c1, char c2)
  {
    return std::toupper(c1) > std::toupper(c2);
  }

  static int compare(const char* s1, const char* s2, std::size_t n)
  {
    while (n-- > 0)
    {
      if (lt(*s1, *s2)) return -1;
      if (gt(*s1, *s2)) return 1;
      ++s1; ++s2;
    }
    return 0;
  }

  static const char* find(const char* s, int n, char a)
  {
    while (n-- > 0 && std::toupper(*s) != std::toupper(a))
    {
      ++s;
    }
    return s;
  }

};

class ci_string : public std::basic_string<char, ci_char_traits>
{
private:

  typedef std::basic_string<char, ci_char_traits> Super;

public:

  inline ci_string() : Super() {}
  inline ci_string(const ci_string& cistr) : Super(cistr) {}
  inline ci_string(const ci_string& cistr, size_t pos, size_t n) : Super(cistr, pos, n) {}
  inline ci_string(const char* s, size_t n) : Super(s, n) {}
  inline ci_string(const char* s) : Super(s) {}
  inline ci_string(size_t n, char c) : Super(n, c) {}

  inline ci_string(const std::string& str) : Super(str.begin(), str.end()) {}

  template<class InputIterator> ci_string(InputIterator begin, InputIterator end)
    : Super(begin, end)
  {}

  inline operator std::string () const
  {
    return std::string(begin(), end());
  }
};

struct MITK_CORE_EXPORT hash_ci_string
{
  std::size_t operator()(const ci_string& s) const;
};

/**
 * \ingroup MicroServices
 *
 * A hash table based map class with case-insensitive keys. This class
 * uses ci_string as key type and Any as values. Due
 * to the conversion capabilities of ci_string, std::string objects
 * can be used transparantly to insert or retrieve key-value pairs.
 *
 * <p>
 * Note that the case of the keys will be preserved.
 */
#ifdef MITK_HAS_UNORDERED_MAP_H
typedef std::unordered_map<ci_string, Any, hash_ci_string> ServiceProperties;
#else
typedef itk::hash_map<ci_string, Any, hash_ci_string> ServiceProperties;
#endif

/**
 * \ingroup MicroServices
 */
namespace ServiceConstants {

/**
 * Service property identifying all of the class names under which a service
 * was registered in the framework. The value of this property must be of
 * type <code>std::list&lt;std::string&gt;</code>.
 *
 * <p>
 * This property is set by the framework when a service is registered.
 */
MITK_CORE_EXPORT const std::string& OBJECTCLASS(); // = "objectclass"

/**
 * Service property identifying a service's registration number. The value
 * of this property must be of type <code>long int</code>.
 *
 * <p>
 * The value of this property is assigned by the framework when a service is
 * registered. The framework assigns a unique value that is larger than all
 * previously assigned values since the framework was started. These values
 * are NOT persistent across restarts of the framework.
 */
MITK_CORE_EXPORT const std::string& SERVICE_ID(); // = "service.id"

/**
 * Service property identifying a service's ranking number.
 *
 * <p>
 * This property may be supplied in the
 * <code>ServiceProperties</code> object passed to the
 * <code>ModuleContext::RegisterService</code> method. The value of this
 * property must be of type <code>int</code>.
 *
 * <p>
 * The service ranking is used by the framework to determine the <i>natural
 * order</i> of services, see ServiceReference::operator<(const ServiceReference&),
 * and the <i>default</i> service to be returned from a call to the
 * {@link ModuleContext::GetServiceReference} method.
 *
 * <p>
 * The default ranking is zero (0). A service with a ranking of
 * <code>std::numeric_limits<int>::max()</code> is very likely to be returned as the
 * default service, whereas a service with a ranking of
 * <code>std::numeric_limits<int>::min()</code> is very unlikely to be returned.
 *
 * <p>
 * If the supplied property value is not of type <code>int</code>, it is
 * deemed to have a ranking value of zero.
 */
MITK_CORE_EXPORT const std::string& SERVICE_RANKING(); // = "service.ranking"

}

}

#endif // MITK_SERVICE_PROPERTIES_H
