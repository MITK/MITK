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

    void Fit(const MatrixType &X, const VectorType &Y) override;
    VectorType Predict(const MatrixType &X) override;

    bool SupportsPointWiseWeight() override;

  private:

    void ConvertParameter(LibSVM::svm_parameter* parameter);
    void ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const MatrixType &X);
    void ReadYValues(LibSVM::svm_problem * problem, const VectorType &Y);
    void ReadWValues(LibSVM::svm_problem * problem);

    LibSVM::svm_model* m_Model;
  };
}

#endif //mitkLibSVMClassifier_h
