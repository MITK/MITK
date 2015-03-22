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

#include <../ClassifierVigraRandomForest/MitkClassifierVigraRandomForestExports.h>

#include <mitkAbstractClassifier.h>

namespace mitk
{
  class MITKCLASSIFIERVIGRARANDOMFOREST_EXPORT VigraRandomForestClassifier : public AbstractClassifier
  {
  public:
    VigraRandomForestClassifier();
    ~VigraRandomForestClassifier();

    void Fit(const MatrixType &X, const VectorType &Y) override;
    VectorType Predict(const MatrixType &X) override;

    bool SupportsPointWiseWeight() override;
    void ConvertParameter();

  };
}

#endif //mitkVigraRandomForestClassifier_h
