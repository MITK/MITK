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

#ifndef mitkPURFClassifier_h
#define mitkPURFClassifier_h

#include <MitkCLVigraRandomForestExports.h>
#include <mitkAbstractClassifier.h>

//#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>

#include <mitkBaseData.h>

namespace mitk
{
  class MITKCLVIGRARANDOMFOREST_EXPORT PURFClassifier : public AbstractClassifier
  {
  public:

    mitkClassMacro(PURFClassifier, AbstractClassifier)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      PURFClassifier();

    ~PURFClassifier();

    void Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y);

    Eigen::MatrixXi Predict(const Eigen::MatrixXd &X);
    Eigen::MatrixXi PredictWeighted(const Eigen::MatrixXd &X);


    bool SupportsPointWiseWeight();
    bool SupportsPointWiseProbability();
    void ConvertParameter();
    vigra::ArrayVector<double> CalculateKappa(const Eigen::MatrixXd & X_in, const Eigen::MatrixXi &Y_in);

    void SetRandomForest(const vigra::RandomForest<int> & rf);
    const vigra::RandomForest<int> & GetRandomForest() const;

    void UsePointWiseWeight(bool);
    void SetMaximumTreeDepth(int);
    void SetMinimumSplitNodeSize(int);
    void SetPrecision(double);
    void SetSamplesPerTree(double);
    void UseSampleWithReplacement(bool);
    void SetTreeCount(int);
    void SetWeightLambda(double);

    void PrintParameter(std::ostream &str = std::cout);

    void SetClassProbabilities(Eigen::VectorXd probabilities);
    Eigen::VectorXd GetClassProbabilites();

  private:
    // *-------------------
    // * THREADING
    // *-------------------


    struct TrainingData;
    struct PredictionData;
    struct EigenToVigraTransform;
    struct Parameter;

    vigra::MultiArrayView<2, double> m_Probabilities;
    Eigen::MatrixXd m_TreeWeights;
    Eigen::VectorXd m_ClassProbabilities;

    Parameter * m_Parameter;
    vigra::RandomForest<int> m_RandomForest;

    static ITK_THREAD_RETURN_TYPE TrainTreesCallback(void *);
    static ITK_THREAD_RETURN_TYPE PredictCallback(void *);
    static ITK_THREAD_RETURN_TYPE PredictWeightedCallback(void *);
    static void VigraPredictWeighted(PredictionData *data, vigra::MultiArrayView<2, double> & X, vigra::MultiArrayView<2, int> & Y, vigra::MultiArrayView<2, double> & P);
  };
}

#endif //mitkPURFClassifier_h
