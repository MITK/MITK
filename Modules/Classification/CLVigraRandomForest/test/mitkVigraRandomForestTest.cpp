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

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include "mitkIOUtil.h"
#include <mitkVigraRandomForestClassifier.h>
#include <itkLabelSampler.h>
#include <itkAddImageFilter.h>
#include <mitkImageCast.h>

class mitkVigraRandomForestTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkVigraRandomForestTestSuite  );

  MITK_TEST(TrainThreadedDecisionForest);
  //  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::VigraRandomForestClassifier::Pointer classifier;

public:

  void setUp(void)
  {


  }

  void TrainThreadedDecisionForest()
  {


  }

  void TestThreadedDecisionForest()
  {

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkVigraRandomForest)


