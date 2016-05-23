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

#include "mitkDICOMTagHelper.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

class mitkDICOMTagHelperTestSuite : public mitk::TestFixture
{
    CPPUNIT_TEST_SUITE(mitkDICOMTagHelperTestSuite);
    // Test the append method
    MITK_TEST(GeneratPropertyNameForDICOMTag);
    MITK_TEST(GetDefaultDICOMTagsOfInterest);

    CPPUNIT_TEST_SUITE_END();

private:

public:

    void setUp() override
    {
    }

    void tearDown() override
    {
    }

    void GeneratPropertyNameForDICOMTag()
    {
        std::string result = mitk::GeneratPropertyNameForDICOMTag(mitk::DICOMTag(0x0018, 0x0080));
        MITK_TEST_CONDITION_REQUIRED(result == "DICOM.0018.0080", "Testing GeneratPropertyNameForDICOMTag(mitk::DICOMTag(0x0018, 0x0080)");

        result = mitk::GeneratPropertyNameForDICOMTag(mitk::DICOMTag(0x0008, 0x001a));
        MITK_TEST_CONDITION_REQUIRED(result == "DICOM.0008.001a", "Testing GeneratPropertyNameForDICOMTag(mitk::DICOMTag(0x0008, 0x001a)");
    }

    void GetDefaultDICOMTagsOfInterest()
    {
    }

};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMTagHelper)
