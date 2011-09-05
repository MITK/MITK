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

#include "mitkAny.h"

namespace mitk {

template<typename Iterator>
std::string container_to_string(Iterator i1, Iterator i2)
{
  std::stringstream ss;
  ss << "(";
  const Iterator begin = i1;
  for ( ; i1 != i2; ++i1)
  {
    if (i1 == begin) ss << *i1;
    else ss << "," << *i1;
  }
  return ss.str();
}

std::string any_value_to_string(const std::vector<std::string>& val)
{
  return container_to_string(val.begin(), val.end());
}

std::string any_value_to_string(const std::list<std::string>& val)
{
  return container_to_string(val.begin(), val.end());
}

std::string any_value_to_string(const std::vector<Any>& val)
{
  return container_to_string(val.begin(), val.end());
}

}
