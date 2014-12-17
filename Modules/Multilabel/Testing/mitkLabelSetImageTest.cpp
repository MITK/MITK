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

#include <mitkLabelSetImage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkLabelSetImageTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSetImageTestSuite);
  MITK_TEST(TestInitialize);
  MITK_TEST(TestInitializeByLabeledImage);
  MITK_TEST(TestConcatenate);
  MITK_TEST(TestClearBuffer);
  MITK_TEST(TestMergeLabels);
  MITK_TEST(TestMergeLabel);
  MITK_TEST(TestUpdateCenterOfMass);
  MITK_TEST(TestRemoveLabels);
  MITK_TEST(TestEraseLabel);
  MITK_TEST(TestEraseLabels);
  MITK_TEST(TestExistsLabel);
  MITK_TEST(TestExistsLabelSet);
  MITK_TEST(TestGetActiveLabel);
  MITK_TEST(TestGetLabel);
  MITK_TEST(TestGetActiveLabelSet);
  MITK_TEST(TestGetLabelSet);
  MITK_TEST(TestGetActiveLayer);
  MITK_TEST(TestGetNumberOfLabels);
  MITK_TEST(TestGetTotalNumberOfLabels);
  MITK_TEST(TestSetActiveLayer);
  MITK_TEST(TestGetNumberOfLayers);
  MITK_TEST(TestGetVectorImage);
  MITK_TEST(TestSetVectorImage);
  MITK_TEST(TestAddLayer);
  MITK_TEST(TestRemoveLayer);
  MITK_TEST(TestGetLayerImage);
  MITK_TEST(TestSetExteriorLabel);
  MITK_TEST(TestGetExteriorLabel);
  // TODO Mask, Stamp, etc. aus der Datenstructur raus
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSetImage::Pointer m_LabelSetImage;

public:

  void setUp()
  {
    // Create a new labelset image with 1 layer
  }

  void tearDown()
  {
    // Delete LabelSetImage
  }

  // Reduce contours with nth point
  void TestInitialize()
  {

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSetImage)
