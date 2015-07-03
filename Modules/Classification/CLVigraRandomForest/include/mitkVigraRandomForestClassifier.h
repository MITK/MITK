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

#ifndef mitkVigraRandomForestClassifier_h
#define mitkVigraRandomForestClassifier_h

#include <MitkCLVigraRandomForestExports.h>
#include <mitkAbstractClassifier.h>

//#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>

#include <mitkBaseData.h>

namespace mitk
{
class MITKCLVIGRARANDOMFOREST_EXPORT VigraRandomForestClassifier : public AbstractClassifier
{
public:

  mitkClassMacro(VigraRandomForestClassifier,AbstractClassifier)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  VigraRandomForestClassifier();
  VigraRandomForestClassifier(const VigraRandomForestClassifier & other)
  {
    this->m_RandomForest = other.m_RandomForest;
  }

  ~VigraRandomForestClassifier();

  void Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y);
  void OnlineTrain(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y);
  Eigen::MatrixXi Predict(const Eigen::MatrixXd &X);

  bool SupportsPointWiseWeight();
  bool SupportsPointWiseProbability();
  void ConvertParameter();

  void SetRandomForest(const vigra::RandomForest<int> & rf)
  {
    m_RandomForest = rf;
  }

  const vigra::RandomForest<int> & GetRandomForest() const
  {
    return m_RandomForest;
  }

  void UsePointWiseWeight(bool);
  void SetMaximumTreeDepth(int);
  void SetMinimumSplitNodeSize(int);
  void SetPrecision(double);
  void SetSamplesPerTree(double);
  void UseSampleWithReplacement(bool);
  void SetTreeCount(int);
  void SetWeightLambda(double);

  void SetNthItems(const char *val, unsigned int idx);
  std::string GetNthItem(unsigned int idx);

  void SetItemList(std::vector<std::string>);
  std::vector<std::string> GetItemList();

  void PrintParameter(std::ostream &str = std::cout);


private:
  // *-------------------
  // * THREADING
  // *-------------------

  static ITK_THREAD_RETURN_TYPE TrainTreesCallback(void *);
  static ITK_THREAD_RETURN_TYPE PredictCallback(void *);

  struct TrainingData;
  struct PredictionData;
  struct EigenToVigraTransform;
  struct Parameter;

  Parameter * m_Parameter;
  vigra::RandomForest<int> m_RandomForest;

};
}

#endif //mitkVigraRandomForestClassifier_h
