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

#ifndef mitkLibSVMClassifier_h
#define mitkLibSVMClassifier_h

#include <MitkCLLibSVMExports.h>

#include <mitkAbstractClassifier.h>

namespace LibSVM
{
  struct svm_parameter;
  struct svm_problem;
  struct svm_node;
  struct svm_model;
}

namespace mitk
{
  class MITKCLLIBSVM_EXPORT LibSVMClassifier : public AbstractClassifier
  {
  public:

    mitkClassMacro(LibSVMClassifier,AbstractClassifier)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    LibSVMClassifier();
    ~LibSVMClassifier() override;

    void Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y) override;
    Eigen::MatrixXi Predict(const Eigen::MatrixXd &X) override;

    bool SupportsPointWiseWeight() override{return true;}
    bool SupportsPointWiseProbability() override{return false;}

    void PrintParameter(std::ostream & str);
    void ConvertParameter();

    void SetSvmType(int val);
    void SetProbability(int val);
    void SetShrinking(int val);
    void SetNrWeight(int val);
    void SetNu(double val);
    void SetP(double val);
    void SetEps(double val);
    void SetC(double val);
    void SetCacheSize(double val);
    void SetKernelType(int val);
    void SetDegree(int val);
    void SetGamma(double val);
    void SetCoef0(double val);

  private:

    void ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const Eigen::MatrixXd &X);
    void ReadYValues(LibSVM::svm_problem * problem, const Eigen::MatrixXi &Y);
    void ReadWValues(LibSVM::svm_problem * problem);

    LibSVM::svm_model* m_Model;
    LibSVM::svm_parameter * m_Parameter;

  };
}

#endif //mitkLibSVMClassifier_h
