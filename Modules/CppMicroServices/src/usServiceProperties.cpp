/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usServiceProperties.h>

#include <algorithm>

namespace us {

const std::string& ServiceConstants::OBJECTCLASS()
{
  static const std::string s("objectclass");
  return s;
}

const std::string& ServiceConstants::SERVICE_ID()
{
  static const std::string s("service.id");
  return s;
}

const std::string& ServiceConstants::SERVICE_RANKING()
{
  static const std::string s("service.ranking");
  return s;
}

const std::string& ServiceConstants::SERVICE_SCOPE()
{
  static const std::string s("service.scope");
  return s;
}

const std::string& ServiceConstants::SCOPE_SINGLETON()
{
  static const std::string s("singleton");
  return s;
}

const std::string& ServiceConstants::SCOPE_MODULE()
{
  static const std::string s("module");
  return s;
}

const std::string& ServiceConstants::SCOPE_PROTOTYPE()
{
  static const std::string s("prototype");
  return s;
}

}

using namespace us;

// make sure all static locals get constructed, so that they
// can be used in destructors of global statics.
std::string tmp1 = ServiceConstants::OBJECTCLASS();
std::string tmp2 = ServiceConstants::SERVICE_ID();
std::string tmp3 = ServiceConstants::SERVICE_RANKING();
std::string tmp4 = ServiceConstants::SERVICE_SCOPE();
std::string tmp5 = ServiceConstants::SCOPE_SINGLETON();
std::string tmp6 = ServiceConstants::SCOPE_MODULE();
std::string tmp7 = ServiceConstants::SCOPE_PROTOTYPE();
