/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include "mitkTestingMacros.h"

#include "mitkSimpleBarrierConstraintChecker.h"
#include "mitkMVConstrainedCostFunctionDecorator.h"
#include "mitkLinearModel.h"

class TestCostFunction : public mitk::MVModelFitCostFunction
{
public:

    typedef TestCostFunction Self;
    typedef mitk::MVModelFitCostFunction Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkNewMacro(Self);

    typedef Superclass::SignalType SignalType;

    mutable unsigned int m_calls;

protected:

    MeasureType CalcMeasure(const ParametersType &parameters, const SignalType& signal) const override
    {
      MeasureType result = signal;
      result[0] = parameters[0];
      result[1] = parameters[1];
      ++m_calls;
      return result;
    };

    TestCostFunction()
    {
      m_calls = 0;
    }

    ~TestCostFunction() override{}
};

int mitkMVConstrainedCostFunctionDecoratorTest(int  /*argc*/, char*[] /*argv[]*/)
{
	MITK_TEST_BEGIN("mitkMVConstrainedCostFunctionDecoratorTest")

  mitk::SimpleBarrierConstraintChecker::Pointer checker = mitk::SimpleBarrierConstraintChecker::New();
  mitk::MVConstrainedCostFunctionDecorator::Pointer decorator = mitk::MVConstrainedCostFunctionDecorator::New();
  TestCostFunction::Pointer innerCF = TestCostFunction::New();
  mitk::LinearModel::Pointer model = mitk::LinearModel::New();
  decorator->SetModel(model);
  innerCF->SetModel(model);

  mitk::MVModelFitCostFunction::SignalType signal(5);
  signal.Fill(0.0);
  decorator->SetSample(signal);
  innerCF->SetSample(signal);

  mitk::LinearModel::TimeGridType grid(5);
  grid[0] = 0;
  grid[1] = 1;
  grid[2] = 2;
  grid[3] = 3;
  grid[4] = 4;

  model->SetTimeGrid(grid);

  mitk::SimpleBarrierConstraintChecker::ParametersType p1(2);
  p1[0] = 0;
  p1[1] = 50;
  mitk::SimpleBarrierConstraintChecker::ParametersType p2(2);
  p2[0] = 10;
  p2[1] = 50;
  mitk::SimpleBarrierConstraintChecker::ParametersType p3(2);
  p3[0] = 100;
  p3[1] = 50;
  mitk::SimpleBarrierConstraintChecker::ParametersType p4(2);
  p4[0] = 2;
  p4[1] = 50;

  checker->SetLowerBarrier(0,1,4);
  checker->SetUpperBarrier(0,100);

  double defaultMaxPenalty = 1e15;

  ///////////////////////
  //Tests

  //check freshly created checker;
  MITK_TEST_CONDITION_REQUIRED(decorator->GetWrappedCostFunction() == nullptr,
								 "Testing GetWrappedCostFunction for new decorator.");
  MITK_TEST_CONDITION_REQUIRED(decorator->GetConstraintChecker() == nullptr,
								 "Testing GetWrappedCostFunction for new decorator.");
  MITK_TEST_CONDITION_REQUIRED(decorator->GetFailureThreshold() == defaultMaxPenalty,
								 "Testing GetWrappedCostFunction for new decorator.");

  MITK_TEST_FOR_EXCEPTION(mitk::Exception, decorator->GetValue(p1));
  decorator->SetWrappedCostFunction(innerCF);
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, decorator->GetValue(p1));
  decorator->SetWrappedCostFunction(nullptr);
  decorator->SetConstraintChecker(checker);
  MITK_TEST_FOR_EXCEPTION(mitk::Exception, decorator->GetValue(p1));
  decorator->SetWrappedCostFunction(innerCF);


  mitk::MVModelFitCostFunction::MeasureType measure = decorator->GetValue(p1);
  MITK_TEST_CONDITION_REQUIRED(measure[0] == defaultMaxPenalty, "Testing measure 1 with parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(measure[1] == defaultMaxPenalty, "Testing measure 2 with parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(measure[2] == defaultMaxPenalty, "Testing measure 3 with parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(measure[3] == defaultMaxPenalty, "Testing measure 3 with parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(measure[4] == defaultMaxPenalty, "Testing measure 3 with parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(innerCF->m_calls == 0, "Checking calls with parameters p1.");

  measure = decorator->GetValue(p2);
  MITK_TEST_CONDITION_REQUIRED(measure[0] == 10, "Testing measure 1 with parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(measure[1] == 50, "Testing measure 2 with parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(measure[2] == 70, "Testing measure 3 with parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(innerCF->m_calls == 1, "Checking calls with parameters p2.");

  measure = decorator->GetValue(p3);
  MITK_TEST_CONDITION_REQUIRED(measure[0] == defaultMaxPenalty, "Testing measure 1 with parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(measure[1] == defaultMaxPenalty, "Testing measure 2 with parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(measure[2] == defaultMaxPenalty, "Testing measure 3 with parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(innerCF->m_calls == 1, "Checking calls with parameters p3.");

  decorator->SetActivateFailureThreshold(false);
  measure = decorator->GetValue(p3);
  MITK_TEST_CONDITION_REQUIRED(measure[0] == 100+defaultMaxPenalty, "Testing measure 1 with parameters p3 (deactiveated threshold).");
  MITK_TEST_CONDITION_REQUIRED(measure[1] == 50+defaultMaxPenalty, "Testing measure 2 with parameters p3 (deactiveated threshold).");
  MITK_TEST_CONDITION_REQUIRED(measure[2] == 250+defaultMaxPenalty, "Testing measure 3 with parameters p3 (deactiveated threshold).");
  MITK_TEST_CONDITION_REQUIRED(innerCF->m_calls ==2, "Checking calls with parameters p3 (deactiveated threshold).");
  decorator->SetActivateFailureThreshold(true);

  measure = decorator->GetValue(p4);
  MITK_TEST_CONDITION_REQUIRED(measure[0] == 2+(-1*log(1/4.)), "Testing measure 1 with parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(measure[1] == 50+(-1*log(1/4.)), "Testing measure 2 with parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(measure[2] == 54+(-1*log(1/4.)), "Testing measure 3 with parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(innerCF->m_calls == 3, "Checking calls with parameters p4.");


  MITK_TEST_END()
}
