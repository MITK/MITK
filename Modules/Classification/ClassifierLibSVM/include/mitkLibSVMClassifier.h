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
    LibSVMClassifier();
    ~LibSVMClassifier();

    void Fit(const EigenMatrixXdType &X, const EigenVectorXdType &Y) override;
    EigenVectorXdType Predict(const EigenMatrixXdType &X) override;

    bool SupportsPointWiseWeight() override;

  private:

    void ConvertParameter(LibSVM::svm_parameter* parameter);
    void ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const EigenMatrixXdType &X);
    void ReadYValues(LibSVM::svm_problem * problem, const EigenVectorXdType &Y);
    void ReadWValues(LibSVM::svm_problem * problem);

    LibSVM::svm_model* m_Model;
  };
}

#endif //mitkLibSVMClassifier_h
