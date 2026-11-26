/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkEnvironmentInfo.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkVersion.h>

#include <QString>
#include <QSysInfo>

class mitkEnvironmentInfoTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkEnvironmentInfoTestSuite);
  MITK_TEST(CollectsRealValues);
  MITK_TEST(NeverContainsTheHostname);
  MITK_TEST(FieldsAreOrderedAndSkipEmptyFields);
  MITK_TEST(FieldDisplayLabelsArePinned);
  CPPUNIT_TEST_SUITE_END();

public:
  void CollectsRealValues()
  {
    const auto info = mitk::CollectEnvironmentInfo();

    CPPUNIT_ASSERT_EQUAL(std::string(MITK_VERSION_STRING), info.mitkVersion);
    CPPUNIT_ASSERT_EQUAL(QSysInfo::prettyProductName().toStdString(), info.os);
    CPPUNIT_ASSERT_EQUAL(QSysInfo::currentCpuArchitecture().toStdString(), info.cpuArchitecture);
    CPPUNIT_ASSERT(!info.buildType.empty());
    CPPUNIT_ASSERT(!info.qtVersion.empty());
    CPPUNIT_ASSERT(!info.vtkVersion.empty());
    CPPUNIT_ASSERT(!info.itkVersion.empty());
    CPPUNIT_ASSERT(!info.uiLocale.empty());
    CPPUNIT_ASSERT_MESSAGE("the UI layer fills in the GPU renderer", info.gpuRenderer.empty());
  }

  void NeverContainsTheHostname()
  {
    const auto hostname = QSysInfo::machineHostName().toStdString();

    if (hostname.empty())
      return;

    for (const auto& [field, value] : mitk::ToFields(mitk::CollectEnvironmentInfo()))
      CPPUNIT_ASSERT_MESSAGE(mitk::ToString(field), value.find(hostname) == std::string::npos);
  }

  void FieldsAreOrderedAndSkipEmptyFields()
  {
    auto info = mitk::CollectEnvironmentInfo();
    info.mitkRevisionDescription.clear();

    const auto fields = mitk::ToFields(info);
    CPPUNIT_ASSERT(fields == mitk::ToFields(info));
    CPPUNIT_ASSERT(mitk::EnvironmentField::MitkVersion == fields.front().first);

    for (const auto& field : fields)
    {
      CPPUNIT_ASSERT(field.first != mitk::EnvironmentField::RevisionDescription);
      CPPUNIT_ASSERT(field.first != mitk::EnvironmentField::GpuRenderer);
    }

    info.gpuRenderer = "Test GPU";
    const auto withGpu = mitk::ToFields(info);
    CPPUNIT_ASSERT_EQUAL(fields.size() + 1, withGpu.size());
    CPPUNIT_ASSERT(mitk::EnvironmentField::GpuRenderer == withGpu.back().first);
    CPPUNIT_ASSERT_EQUAL(std::string("Test GPU"), withGpu.back().second);
  }

  void FieldDisplayLabelsArePinned()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("MITK version"), mitk::ToString(mitk::EnvironmentField::MitkVersion));
    CPPUNIT_ASSERT_EQUAL(std::string("Revision description"), mitk::ToString(mitk::EnvironmentField::RevisionDescription));
    CPPUNIT_ASSERT_EQUAL(std::string("Operating system"), mitk::ToString(mitk::EnvironmentField::OperatingSystem));
    CPPUNIT_ASSERT_EQUAL(std::string("GPU renderer"), mitk::ToString(mitk::EnvironmentField::GpuRenderer));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkEnvironmentInfo)
