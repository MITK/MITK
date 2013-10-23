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

#ifndef MITKPARAMETERIZEDTESTFIXTURE_H
#define MITKPARAMETERIZEDTESTFIXTURE_H

#include <cppunit/TestFixture.h>

#include <vector>
#include <string>

namespace mitk {

class ParameterizedTestFixture : public CppUnit::TestFixture
{

public:

  virtual void setUpParameter(const std::vector<std::string>& parameter) {}

};

}

#endif // MITKPARAMETERIZEDTESTFIXTURE_H
