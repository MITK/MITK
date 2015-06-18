/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryCallHistory.h"

#include <algorithm>

namespace berry
{

CallHistory::CallHistory(/*Object target*/)
{
  // classType = target.getClass();
}

void CallHistory::Add(const std::string& methodName)
{
  TestMethodName(methodName);
  methodList.push_back(methodName);
}

void CallHistory::Clear()
{
  methodList.clear();
}

bool CallHistory::VerifyOrder(const std::vector<std::string>& testNames) const
throw(Poco::InvalidArgumentException)
{
  std::size_t testIndex = 0;
  std::size_t testLength = testNames.size();
  if (testLength == 0)
  return true;
  for (std::size_t nX = 0; nX < methodList.size(); nX++)
  {
    const std::string& methodName = methodList[nX];
    const std::string& testName = testNames[testIndex];
    TestMethodName(testName);
    if (testName == methodName)
    ++testIndex;
    if (testIndex >= testLength)
    return true;
  }
  return false;
}

bool CallHistory::Contains(const std::string& methodName) const
{
  TestMethodName(methodName);
  return std::find(methodList.begin(), methodList.end(), methodName) != methodList.end();
}

bool CallHistory::Contains(const std::vector<std::string>& methodNames) const
{
  for (std::size_t i = 0; i < methodNames.size(); i++)
  {
    TestMethodName(methodNames[i]);
    if (std::find(methodList.begin(), methodList.end(), methodNames[i]) == methodList.end())
      return false;
  }
  return true;
}

bool CallHistory::IsEmpty() const
{
  return methodList.empty();
}

void CallHistory::PrintTo(std::ostream& out) const
{
  for (std::size_t i = 0; i < methodList.size(); i++)
    out << methodList[i] << std::endl;
}

void CallHistory::TestMethodName(const std::string&) const
throw(Poco::InvalidArgumentException)
{
//  Method[] methods = classType.getMethods();
//  for (int i = 0; i < methods.length; i++)
//    if (methods[i].getName().equals(methodName))
//      return;
//  throw new IllegalArgumentException("Target class (" + classType.getName()
//      + ") does not contain method: " + methodName);
}

}
