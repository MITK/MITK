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

#include <MitkClassifierLibSVMExports.h>

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
  class MITKCLASSIFIERLIBSVM_EXPORT LibSVMClassifier : public AbstractClassifier
  {
  public:

    mitkClassMacro(LibSVMClassifier,AbstractClassifier)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    LibSVMClassifier();
    ~LibSVMClassifier();

    void Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y) override;
    Eigen::MatrixXi Predict(const Eigen::MatrixXd &X) override;

    bool SupportsPointWiseWeight(){return true;}
    bool SupportsPointWiseProbability(){return false;}

  private:

    void ConvertParameter(LibSVM::svm_parameter* parameter);
    void ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const Eigen::MatrixXd &X);
    void ReadYValues(LibSVM::svm_problem * problem, const Eigen::MatrixXi &Y);
    void ReadWValues(LibSVM::svm_problem * problem);

    LibSVM::svm_model* m_Model;

  };
}

#endif //mitkLibSVMClassifier_h
