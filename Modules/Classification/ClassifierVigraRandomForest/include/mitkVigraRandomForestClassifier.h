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

  void Train(const MatrixType &X, const VectorType &Y);
  VectorType Predict(const MatrixType &X);

  bool SupportsPointWiseWeight();
  bool SupportsPointWiseProbability();
  void ConvertParameter();

  vigra::MultiArrayView<2, double> GetPointWiseWeight();

  typedef AbstractClassifier::MatrixType MatrixType;
  typedef AbstractClassifier::VectorType VectorType;

//  typedef  //VigraMatrix2dType;
//  typedef     VigraLabel2dType;

  void SetRandomForest(const vigra::RandomForest<int> & rf)
  {
    m_RandomForest = rf;
  }

  const vigra::RandomForest<int> & GetRandomForest() const
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
  vigra::RandomForest<int> m_RandomForest;

  static vigra::MultiArray<2, double> transform(const MatrixType & matrix)
  {
    vigra::MultiArray<2, double> outMatrix(matrix.rows(),matrix.cols());
    for(int x = 0 ; x < matrix.rows(); x++)
      for(int y = 0; y < matrix.cols(); y++)
        outMatrix(x,y) = matrix(x,y);
    return outMatrix;
  }

  static MatrixType transform(const vigra::MultiArray<2, double> & matrix)
  {
    MatrixType outMatrix(matrix.shape()[0],matrix.shape()[1]);
    for(int x = 0 ; x < matrix.shape()[0]; x++)
      for(int y = 0; y < matrix.shape()[1]; y++)
        outMatrix(x,y) = matrix(x,y);
    return outMatrix;
  }

  static vigra::MultiArray<2, int> transform(const VectorType & vec)
  {
    vigra::MultiArray<2, int> outVec(vec.rows(),1);
    for(int x = 0 ; x < vec.rows(); x++)
      outVec(x,0) = vec(x);
    return outVec;
  }

  static VectorType transform(const vigra::MultiArray<2, int> & vec)
  {
    VectorType outVec(vec.shape()[0]);
    for(int x = 0 ; x < vec.shape()[0]; x++)
      outVec(x) = vec(x,0);
    return outVec;
  }


  //    unsigned int m_NumberOfThreads;

};
}

#endif //mitkVigraRandomForestClassifier_h
