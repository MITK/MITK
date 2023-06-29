/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include "mitkVector.h"
#include "mitkMaximumCurveDescriptionParameter.h"
#include "mitkMeanResidenceTimeDescriptionParameter.h"
#include "mitkAreaUnderTheCurveDescriptionParameter.h"
#include "mitkAreaUnderFirstMomentDescriptionParameter.h"
#include "mitkTimeToPeakCurveDescriptionParameter.h"

class mitkCurveDescriptiveParametersTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCurveDescriptiveParametersTestSuite);
  MITK_TEST(MaximumCurveDescriptionParameterTest);
  MITK_TEST(AreaUnderTheCurveDescriptionParameterTest);
  MITK_TEST(AreaUnderFirstMomentDescriptionParameterTest);
  MITK_TEST(MeanResidenceTimeDescriptionParameterTest);
  MITK_TEST(TimeToPeakDescriptionParameterTest);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType m_maxParameter;
  mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType m_aucParameter;
  mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType m_aumcParameter;
  mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType m_mrtParameter;
  mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType m_ttpParameter;

public:
  void setUp() override
  {
    mitk::ModelBase::TimeGridType m_grid(22);
    for (int i = 0; i < 22; ++i)
    {
      // time grid in seconds, 14s between frames
      m_grid[i] = (double)14 * i;
    }

    static const double t = 100.0;
    static const double b1 = 0.0;
    static const double a1 = 5.0;
    static const double a2 = -3.0;
    const auto b2 = (a1-a2) * t + b1;


    mitk::ModelBase::ModelResultType signal(m_grid.GetSize());
    mitk::ModelBase::TimeGridType::const_iterator timeGridEnd = m_grid.end();
    mitk::ModelBase::ModelResultType::iterator signalPos = signal.begin();

    for (mitk::ModelBase::ModelResultType::const_iterator gridPos = m_grid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
    {
      if ((*gridPos) < t)
      {
        *signalPos = a1 * (*gridPos) + b1;
      }
      else
      {
        *signalPos = a2 * (*gridPos) + b2;
      }
    }

    mitk::CurveDescriptionParameterBase::Pointer maxParameterFunction = mitk::MaximumCurveDescriptionParameter::New().GetPointer();
    m_maxParameter = maxParameterFunction->GetCurveDescriptionParameter(signal, m_grid);

    mitk::CurveDescriptionParameterBase::Pointer aucParameterFunction = mitk::AreaUnderTheCurveDescriptionParameter::New().GetPointer();
    m_aucParameter = aucParameterFunction->GetCurveDescriptionParameter(signal, m_grid);

    mitk::CurveDescriptionParameterBase::Pointer aumcParameterFunction = mitk::AreaUnderFirstMomentDescriptionParameter::New().GetPointer();
    m_aumcParameter = aumcParameterFunction->GetCurveDescriptionParameter(signal, m_grid);

    mitk::CurveDescriptionParameterBase::Pointer mrtParameterFunction = mitk::MeanResidenceTimeDescriptionParameter::New().GetPointer();
    m_mrtParameter = mrtParameterFunction->GetCurveDescriptionParameter(signal, m_grid);

    mitk::CurveDescriptionParameterBase::Pointer ttpParameterFunction = mitk::TimeToPeakCurveDescriptionParameter::New().GetPointer();
    m_ttpParameter = ttpParameterFunction->GetCurveDescriptionParameter(signal, m_grid);
  }


  void tearDown() override
  {

  }

  void MaximumCurveDescriptionParameterTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking maximum value.", mitk::Equal(490.0, m_maxParameter[0], 1e-6, true) == true);
  }

  void AreaUnderTheCurveDescriptionParameterTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking auc value.", mitk::Equal(67690.0, m_aucParameter[0], 1e-6, true) == true);
  }

  void AreaUnderFirstMomentDescriptionParameterTest()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking aumc value.", mitk::Equal(8459948.0, m_aumcParameter[0], 1e-6, true) == true);
  }

   void MeanResidenceTimeDescriptionParameterTest()
  {
     CPPUNIT_ASSERT_MESSAGE("Checking mrt value.", mitk::Equal(124.980765253, m_mrtParameter[0], 1e-6, true) == true);
  }

   void TimeToPeakDescriptionParameterTest()
   {
     CPPUNIT_ASSERT_MESSAGE("Checking ttp value.", mitk::Equal(98.0, m_ttpParameter[0], 1e-6, true) == true);
   }
};
MITK_TEST_SUITE_REGISTRATION(mitkCurveDescriptiveParameters)


