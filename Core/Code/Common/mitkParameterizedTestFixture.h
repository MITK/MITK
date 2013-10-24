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
#include <mitkTestingConfig.h>

#include <itksys/SystemTools.hxx>

#include <vector>
#include <string>

namespace mitk {

class ParameterizedTestFixture : public CppUnit::TestFixture
{

public:

  virtual void setUpParameter(const std::vector<std::string>& parameter) {}

protected:

  static std::string getTestDataFilePath(const std::string& testData)
  {
    if (itksys::SystemTools::FileIsFullPath(testData.c_str())) return testData;
    return std::string(MITK_DATA_DIR) + "/" + testData;
  }
};

}

#endif // MITKPARAMETERIZEDTESTFIXTURE_H
