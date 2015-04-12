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

#include <MitkClassifierVigraRandomForestExports.h>
#include <mitkAbstractClassifier.h>

//#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>

#include <mitkBaseData.h>

namespace mitk
{
class MITKCLASSIFIERVIGRARANDOMFOREST_EXPORT VigraRandomForestClassifier : public AbstractClassifier
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

  void Train(const MatrixType &X, const MatrixType &Y);
  MatrixType Predict(const MatrixType &X);

  bool SupportsPointWiseWeight();
  bool SupportsPointWiseProbability();
  void ConvertParameter();

  vigra::MultiArrayView<2, double> GetPointWiseWeight();

  typedef AbstractClassifier::MatrixType MatrixType;

//  typedef  //VigraMatrix2dType;
//  typedef     VigraLabel2dType;

  void SetRandomForest(const vigra::RandomForest<double> & rf)
  {
    m_RandomForest = rf;
  }

  const vigra::RandomForest<double> & GetRandomForest() const
  {
    return m_RandomForest;
  }

  void UsePointBasedWeights(bool);
  void SetTreeDepth(int);
  void SetMinumumSplitNodeSize(int);
  void SetPrecision(double);
  void SetSamplesPerTree(double);
  void UseSampleWithReplacement(bool);
  void SetTreeCount(int);
  void SetWeightLambda(double);
//  void SetStratification(vigra::RF_OptionTag);

  void PrintParameter();

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
  vigra::RandomForest<double> m_RandomForest;


  ///
  /// \brief EigenAccessByVigra allows direct access to EigenMatrix data structure (no need to copy data)
  /// \param Already initialized eigen matrix
  /// \return vigra MultiArrayView structure
  ///
  vigra::MultiArrayView<2, double> EigenAccessByVigra(const MatrixType & matrix)
  {
    vigra::Shape2 shape(matrix.rows(),matrix.cols());
    vigra::MultiArrayView<2, double> outMatrix(shape,matrix.data());
    return outMatrix;
  }

};
}

#endif //mitkVigraRandomForestClassifier_h
