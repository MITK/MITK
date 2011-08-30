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

#include <algorithm>

#include "mitkServiceProperties.h"
#include "mitkServiceUtils.h"

const std::string& mitk::ServiceConstants::OBJECTCLASS()
{
  static const std::string s("objectclass");
  return s;
}

const std::string& mitk::ServiceConstants::SERVICE_ID()
{
  static const std::string s("service.id");
  return s;
}

const std::string& mitk::ServiceConstants::SERVICE_RANKING()
{
  static const std::string s("service.ranking");
  return s;
}

namespace mitk {

std::size_t hash_ci_string::operator()(const ci_string& s) const
{
  ci_string ls(s);
  std::transform(ls.begin(), ls.end(), ls.begin(), tolower);

#ifdef MITK_HAS_UNORDERED_MAP_H
  using namespace std;
#else
  using namespace itk;
#endif
  return hash<std::string>()(ls);
}

}

