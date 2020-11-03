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
#include "mitkModelFitConstants.h"
#include "mitkModelFitException.h"

mitk::SimpleBarrierConstraintChecker::ParametersType GenerateParameters(double p1, double p2, double p3)
{
  mitk::SimpleBarrierConstraintChecker::ParametersType result(3);
  result[0] = p1;
  result[1] = p2;
  result[2] = p3;
  return result;
}

int mitkSimpleBarrierConstraintCheckerTest(int  /*argc*/, char*[] /*argv[]*/)
{
	MITK_TEST_BEGIN("mitkSimpleBarrierConstraintCheckerTest")

  mitk::SimpleBarrierConstraintChecker::Pointer checker = mitk::SimpleBarrierConstraintChecker::New();

  double defaultMaxPenalty = 1e15;

  //check freshly created checker;
  MITK_TEST_CONDITION_REQUIRED(checker->GetNumberOfConstraints() == 0, "Testing GetNumberOfConstraints for new checker.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetFailedConstraintValue() == defaultMaxPenalty, "Testing GetNumberOfConstraints for new checker.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetMaxConstraintPenalty() == defaultMaxPenalty, "Testing GetNumberOfConstraints for new checker.");

  //configure checker (test setter)
  checker->SetLowerBarrier(0,5);
  checker->SetLowerBarrier(1,10,3);
  checker->SetUpperBarrier(1,100,10);
  checker->SetUpperBarrier(2,50);

  typedef mitk::SimpleBarrierConstraintChecker::ParametersType ParametersType;
  typedef mitk::SimpleBarrierConstraintChecker::PenaltyArrayType PenaltiesType;

  ParametersType p1 = GenerateParameters(10,30,40);
  ParametersType p2 = GenerateParameters(5.0001,30,40);
  ParametersType p3 = GenerateParameters(5,30,40);
  ParametersType p4 = GenerateParameters(4.5,12,40);
  ParametersType p5 = GenerateParameters(10,10,40);
  ParametersType p6 = GenerateParameters(10,91,40);
  ParametersType p7 = GenerateParameters(10,100,40);
  ParametersType p8 = GenerateParameters(1000,30,50);
  ParametersType p9 = GenerateParameters(10,30,49.999);

  PenaltiesType penalties = checker->GetPenalties(p1);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p1) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p1.");

  penalties = checker->GetPenalties(p2);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p2) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p2.");

  penalties = checker->GetPenalties(p3);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == defaultMaxPenalty, "Testing penalty 1 of test parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p3.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p3) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p3.");

  penalties = checker->GetPenalties(p4);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == defaultMaxPenalty, "Testing penalty 1 of test parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == -1*log(2/3.), "Testing penalty 2 of test parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p4.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p4) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p4.");

  penalties = checker->GetPenalties(p5);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p5.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == defaultMaxPenalty, "Testing penalty 2 of test parameters p5.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p5.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p5.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p5) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p5.");

  penalties = checker->GetPenalties(p6);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p6.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p6.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == -1*log(9/10.), "Testing penalty 3 of test parameters p6.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p6) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p6.");

  penalties = checker->GetPenalties(p7);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p7.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p7.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == defaultMaxPenalty, "Testing penalty 3 of test parameters p7.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p7.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p7) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p7.");

  penalties = checker->GetPenalties(p8);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p8.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p8.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p8.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == defaultMaxPenalty, "Testing penalty 4 of test parameters p8.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p8) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p8.");

  penalties = checker->GetPenalties(p9);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(checker->GetPenaltySum(p9) == penalties[0] + penalties[1] + penalties[2] + penalties[3], "Testing penalty sum of test parameters p9.");

  checker->SetMaxConstraintPenalty(2222);
  penalties = checker->GetPenalties(p8);
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 2222, "Testing penalty 3 of test parameters p8 with changed max penalty.");

  mitk::SimpleBarrierConstraintChecker::ParameterIndexVectorType index;
  index.push_back(0);
  index.push_back(1);
  checker->SetLowerSumBarrier(index,37);

  index[0]=1;
  index[1]=2;
  checker->SetUpperSumBarrier(index,75);

  penalties = checker->GetPenalties(p1);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[4] == 0.0, "Testing penalty 5 of test parameters p1.");
  MITK_TEST_CONDITION_REQUIRED(penalties[5] == 0.0, "Testing penalty 6 of test parameters p1.");

  penalties = checker->GetPenalties(p2);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[4] == 2222, "Testing penalty 5 of test parameters p2.");
  MITK_TEST_CONDITION_REQUIRED(penalties[5] == 0.0, "Testing penalty 6 of test parameters p2.");

  penalties = checker->GetPenalties(p9);
  MITK_TEST_CONDITION_REQUIRED(penalties[0] == 0.0, "Testing penalty 1 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[1] == 0.0, "Testing penalty 2 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[2] == 0.0, "Testing penalty 3 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[3] == 0.0, "Testing penalty 4 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[4] == 0.0, "Testing penalty 5 of test parameters p9.");
  MITK_TEST_CONDITION_REQUIRED(penalties[5] == 2222, "Testing penalty 6 of test parameters p9.");

  ParametersType invalidP;

  MITK_TEST_FOR_EXCEPTION(mitk::Exception, checker->GetPenalties(invalidP));

  MITK_TEST_END()
}
