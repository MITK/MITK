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

#include <mitkVigraRandomForestClassifier.h>

#include <mitkExceptionMacro.h>

mitk::VigraRandomForestClassifier::VigraRandomForestClassifier()
{
}

mitk::VigraRandomForestClassifier::~VigraRandomForestClassifier()
{

}

bool mitk::VigraRandomForestClassifier::SupportsPointWiseWeight()
{
  return true;
}

void mitk::VigraRandomForestClassifier::Fit(const MatrixType &/*X*/, const VectorType &/*Y*/)
{

}

mitk::VigraRandomForestClassifier::VectorType mitk::VigraRandomForestClassifier::Predict(const MatrixType &X)
{
  int noOfPoints = static_cast<int>(X.rows());
//  int noOfFeatures = static_cast<int>(X.cols());

  VectorType result(noOfPoints);
  return result;
}

void  mitk::VigraRandomForestClassifier::ConvertParameter()
{
  // Get the proerty                                                                      // Some defaults
//  if(!this->GetPropertyList()->Get("classifier.svm.svm-type",parameter->svm_type))        parameter->svm_type = 0; // value?
//  if(!this->GetPropertyList()->Get("classifier.svm.kernel-type",parameter->kernel_type))  parameter->kernel_type = 0; // value?
//  if(!this->GetPropertyList()->Get("classifier.svm.degree",parameter->degree))            parameter->degree = 3;
//  if(!this->GetPropertyList()->Get("classifier.svm.gamma",parameter->gamma))              parameter->gamma = 0;
//  if(!this->GetPropertyList()->Get("classifier.svm.coef0",parameter->coef0))              parameter->coef0 = 0;
//  if(!this->GetPropertyList()->Get("classifier.svm.nu",parameter->nu))                    parameter->nu = 0.5;
//  if(!this->GetPropertyList()->Get("classifier.svm.cache-size",parameter->cache_size))    parameter->cache_size = 100.0;
//  if(!this->GetPropertyList()->Get("classifier.svm.c",parameter->C))                      parameter->C = 1.0;
//  if(!this->GetPropertyList()->Get("classifier.svm.eps",parameter->eps))                  parameter->eps = 1e-3;
//  if(!this->GetPropertyList()->Get("classifier.svm.p",parameter->p))                      parameter->p = 0.1;
//  if(!this->GetPropertyList()->Get("classifier.svm.shrinking",parameter->shrinking))      parameter->shrinking = 1;
//  if(!this->GetPropertyList()->Get("classifier.svm.probability",parameter->probability))  parameter->probability = 1;
//  if(!this->GetPropertyList()->Get("classifier.svm.nr-weight",parameter->nr_weight))      parameter->nr_weight = 0;

//  parameter->weight_label = nullptr;
//  parameter->weight = nullptr;
}

