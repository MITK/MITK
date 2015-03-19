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

#include <mitkLibSVMClassifier.h>

#include <mitkConfigurationHolder.h>
namespace LibSVM
{
#include "svm.h"
}
#include <mitkExceptionMacro.h>

mitk::LibSVMClassifier::LibSVMClassifier() :
  m_Model(NULL)
{
}

mitk::LibSVMClassifier::~LibSVMClassifier()
{
  if (m_Model)
  {
    LibSVM::svm_free_and_destroy_model(&m_Model);
  }
}

bool mitk::LibSVMClassifier::SupportsPointWiseWeight()
{
  return true;
}

void mitk::LibSVMClassifier::Train(const MatrixType &X, const VectorType &Y)
{
  LibSVM::svm_parameter parameter;
  LibSVM::svm_problem problem;
  LibSVM::svm_node *xSpace;

  ConvertParameter(&parameter);
  problem.l = static_cast<int>(Y.rows());
  ReadXValues(&problem, &xSpace,X);
  ReadYValues(&problem, Y);
  ReadWValues(&problem);

  const char * error_msg = NULL;
  error_msg = LibSVM::svm_check_parameter(&problem, &parameter);
  if (error_msg)
  {
    LibSVM::svm_destroy_param(&parameter);
    free(problem.y);
    free(problem.x);
    free(xSpace);
    mitkThrow() << "Error: " << error_msg;
  }

  m_Model = LibSVM::svm_train(&problem, &parameter);

  LibSVM::svm_destroy_param(&parameter);
  free(problem.y);
  free(problem.x);
  free(xSpace);
}

mitk::LibSVMClassifier::VectorType mitk::LibSVMClassifier::Predict(const MatrixType &X)
{
  if ( ! m_Model)
  {
    mitkThrow() << "No Model is trained. Train or load a model before predicting new values.";
  }
  int noOfPoints = static_cast<int>(X.rows());
  int noOfFeatures = static_cast<int>(X.cols());

  VectorType result(noOfPoints);

  LibSVM::svm_node * xVector = static_cast<LibSVM::svm_node *>(malloc(sizeof(LibSVM::svm_node) * (noOfFeatures+1)));
  for (int point = 0; point < noOfPoints; ++point)
  {
    for (int feature = 0; feature < noOfFeatures; ++feature)
    {
      xVector[feature].index = feature+1;
      xVector[feature].value = X(point, feature);
    }
    xVector[noOfFeatures].index = -1;
    result[point] = LibSVM::svm_predict(m_Model,xVector);
  }
  free(xVector);
  return result;
}

void  mitk::LibSVMClassifier::ConvertParameter(LibSVM::svm_parameter* parameter)
{
  parameter->svm_type = Configuration().At("SVM Type").AsInt(LibSVM::C_SVC);
  parameter->kernel_type = Configuration().At("Kernel Type").AsInt(LibSVM::RBF);
  parameter->degree = Configuration().At("Degree").AsInt(3);
  parameter->gamma = Configuration().At("Gamma").AsDouble(0);
  parameter->coef0 = Configuration().At("Coef 0").AsDouble(0);
  parameter->nu = Configuration().At("Nu").AsDouble(0.5);
  parameter->cache_size = Configuration().At("Cache Size").AsDouble(100);
  parameter->C = Configuration().At("C").AsDouble(1);
  parameter->eps = Configuration().At("Eps").AsDouble(1e-3);
  parameter->p = Configuration().At("P").AsDouble(0.1);
  parameter->shrinking = Configuration().At("Shrinking").AsInt(1);
  parameter->probability = Configuration().At("Probability").AsInt(1);
  parameter->nr_weight = Configuration().At("Nr Weight").AsInt(0);
  parameter->weight_label = NULL;
  parameter->weight = NULL;
}

void mitk::LibSVMClassifier::ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const MatrixType &X)
{
  int noOfPoints = static_cast<int>(X.rows());
  int features = static_cast<int>(X.cols());

  problem->x = static_cast<LibSVM::svm_node **>(malloc(sizeof(LibSVM::svm_node *)  * noOfPoints));
  (*xSpace) = static_cast<LibSVM::svm_node *> (malloc(sizeof(LibSVM::svm_node) * noOfPoints * (features+1)));

  for (int row = 0; row < noOfPoints; ++row)
  {
    for (int col = 0; col < features; ++col)
    {
      (*xSpace)[row*features + col].index = col;
      (*xSpace)[row*features + col].value = X(row,col);
    }
    (*xSpace)[row*features + features].index = -1;

    problem->x[row] = &((*xSpace)[row*features]);
  }
}

void mitk::LibSVMClassifier::ReadYValues(LibSVM::svm_problem * problem, const VectorType &Y)
{
  int noOfPoints = static_cast<int>(Y.rows());
  problem->y = static_cast<double *>(malloc(sizeof(double)  * noOfPoints));

  for (int i = 0; i < noOfPoints; ++i)
  {
    problem->y[i] = Y[i];
  }
}

void mitk::LibSVMClassifier::ReadWValues(LibSVM::svm_problem * problem)
{
  VectorType W = PointWeight();
  int noOfPoints = problem->l;
  problem->W = static_cast<double *>(malloc(sizeof(double)  * noOfPoints));

  if (IsUsingPointWiseWeight())
  {
    for (int i = 0; i < noOfPoints; ++i)
    {
      problem->y[i] = W[i];
    }
  } else
  {
    for (int i = 0; i < noOfPoints; ++i)
    {
      problem->y[i] = 1;
    }
  }
}