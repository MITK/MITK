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

mitk::LibSVMClassifier::LibSVMClassifier():
  m_Model(nullptr),m_Parameter(nullptr)
{
  this->m_Parameter = new LibSVM::svm_parameter();
}

mitk::LibSVMClassifier::~LibSVMClassifier()
{
  if (m_Model)
  {
    LibSVM::svm_free_and_destroy_model(&m_Model);
  }
  if( m_Parameter)
    LibSVM::svm_destroy_param(m_Parameter);
}

void mitk::LibSVMClassifier::Train(const Eigen::MatrixXd &X, const Eigen::MatrixXi &Y)
{
  this->SetPointWiseWeight(Eigen::MatrixXd(Y.rows(),1));
  this->UsePointWiseWeight(false);

  LibSVM::svm_node *xSpace;
  LibSVM::svm_problem problem;

  ConvertParameter();
  ReadYValues(&problem, Y);
  ReadXValues(&problem, &xSpace,X);
  ReadWValues(&problem);

  const char * error_msg = nullptr;
  error_msg = LibSVM::svm_check_parameter(&problem, m_Parameter);
  if (error_msg)
  {
    LibSVM::svm_destroy_param(m_Parameter);
    free(problem.y);
    free(problem.x);
    free(xSpace);
    mitkThrow() << "Error: " << error_msg;
  }

  m_Model = LibSVM::svm_train(&problem, m_Parameter);

  // free(problem.y);
  // free(problem.x);
  // free(xSpace);
}

Eigen::MatrixXi mitk::LibSVMClassifier::Predict(const Eigen::MatrixXd &X)
{
  if ( ! m_Model)
  {
    mitkThrow() << "No Model is trained. Train or load a model before predicting new values.";
  }
  int noOfPoints = static_cast<int>(X.rows());
  int noOfFeatures = static_cast<int>(X.cols());

  Eigen::MatrixXi result(noOfPoints,1);

  LibSVM::svm_node * xVector = static_cast<LibSVM::svm_node *>(malloc(sizeof(LibSVM::svm_node) * (noOfFeatures+1)));
  for (int point = 0; point < noOfPoints; ++point)
  {
    for (int feature = 0; feature < noOfFeatures; ++feature)
    {
      xVector[feature].index = feature+1;
      xVector[feature].value = X(point, feature);
    }
    xVector[noOfFeatures].index = -1;
    result(point,0) = LibSVM::svm_predict(m_Model,xVector);
  }

  free(xVector);
  return result;
}

void  mitk::LibSVMClassifier::ConvertParameter()
{
  // Get the proerty                                                                      // Some defaults
  if(!this->GetPropertyList()->Get("classifier.svm.svm-type",this->m_Parameter->svm_type))        this->m_Parameter->svm_type = 0;
  if(!this->GetPropertyList()->Get("classifier.svm.kernel-type",this->m_Parameter->kernel_type))  this->m_Parameter->kernel_type = 2;
  if(!this->GetPropertyList()->Get("classifier.svm.degree",this->m_Parameter->degree))            this->m_Parameter->degree = 3;
  if(!this->GetPropertyList()->Get("classifier.svm.gamma",this->m_Parameter->gamma))              this->m_Parameter->gamma = 0; // 1/n_features;
  if(!this->GetPropertyList()->Get("classifier.svm.coef0",this->m_Parameter->coef0))              this->m_Parameter->coef0 = 0;
  if(!this->GetPropertyList()->Get("classifier.svm.nu",this->m_Parameter->nu))                    this->m_Parameter->nu = 0.5;
  if(!this->GetPropertyList()->Get("classifier.svm.cache-size",this->m_Parameter->cache_size))    this->m_Parameter->cache_size = 100.0;
  if(!this->GetPropertyList()->Get("classifier.svm.c",this->m_Parameter->C))                      this->m_Parameter->C = 1.0;
  if(!this->GetPropertyList()->Get("classifier.svm.eps",this->m_Parameter->eps))                  this->m_Parameter->eps = 1e-3;
  if(!this->GetPropertyList()->Get("classifier.svm.p",this->m_Parameter->p))                      this->m_Parameter->p = 0.1;
  if(!this->GetPropertyList()->Get("classifier.svm.shrinking",this->m_Parameter->shrinking))      this->m_Parameter->shrinking = 1;
  if(!this->GetPropertyList()->Get("classifier.svm.probability",this->m_Parameter->probability))  this->m_Parameter->probability = 0;
  if(!this->GetPropertyList()->Get("classifier.svm.nr-weight",this->m_Parameter->nr_weight))      this->m_Parameter->nr_weight = 0;

  //options:
  //-s svm_type : set type of SVM (default 0)
  //  0 -- C-SVC
  //  1 -- nu-SVC
  //  2 -- one-class SVM
  //  3 -- epsilon-SVR
  //  4 -- nu-SVR
  //-t kernel_type : set type of kernel function (default 2)
  //  0 -- linear: u'*v
  //  1 -- polynomial: (gamma*u'*v + coef0)^degree
  //  2 -- radial basis function: exp(-gamma*|u-v|^2)
  //  3 -- sigmoid: tanh(gamma*u'*v + coef0)
  //-d degree : set degree in kernel function (default 3)
  //-g gamma : set gamma in kernel function (default 1/num_features)
  //-r coef0 : set coef0 in kernel function (default 0)
  //-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)
  //-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)
  //-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)
  //-m cachesize : set cache memory size in MB (default 100)
  //-e epsilon : set tolerance of termination criterion (default 0.001)
  //-h shrinking: whether to use the shrinking heuristics, 0 or 1 (default 1)
  //-b probability_estimates: whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)
  //-wiweight: set the parameter C of class i to weight*C, for C-SVC (default 1)

  // this->m_Parameter->weight_label = nullptr;
  // this->m_Parameter->weight = 1;
}

/* these are for training only */
//int *weight_label; /* for C_SVC */
//double* weight;  /* for C_SVC */

void mitk::LibSVMClassifier::SetProbability(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.probability",val);
}

void mitk::LibSVMClassifier::SetShrinking(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.shrinking",val);
}

void mitk::LibSVMClassifier::SetNrWeight(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.nr_weight",val);
}

void mitk::LibSVMClassifier::SetNu(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.nu",val);
}

void mitk::LibSVMClassifier::SetP(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.p",val);
}

void mitk::LibSVMClassifier::SetEps(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.eps",val);
}

void mitk::LibSVMClassifier::SetC(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.c",val);
}

void mitk::LibSVMClassifier::SetCacheSize(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.cache-size",val);
}

void mitk::LibSVMClassifier::SetSvmType(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.svm-type",val);
}

void mitk::LibSVMClassifier::SetKernelType(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.kernel-type",val);
}

void mitk::LibSVMClassifier::SetDegree(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.svm.degree",val);
}

void mitk::LibSVMClassifier::SetGamma(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.gamma",val);
}

void mitk::LibSVMClassifier::SetCoef0(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.svm.coef0",val);
}

void mitk::LibSVMClassifier::PrintParameter(std::ostream & str)
{
  if(this->m_Parameter == nullptr)
  {
    MITK_WARN("LibSVMClassifier") << "Parameters are not initialized. Please call ConvertParameter() first!";
    return;
  }

  this->ConvertParameter();

  // Get the proerty                                                                      // Some defaults
  if(!this->GetPropertyList()->Get("classifier.svm.svm-type",this->m_Parameter->svm_type))
    str << "classifier.svm.svm-type\tNOT SET (default " << this->m_Parameter->svm_type << ")" << "\n";
  else
    str << "classifier.svm.svm-type\t" << this->m_Parameter->svm_type << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.kernel-type",this->m_Parameter->kernel_type))
    str << "classifier.svm.kernel-type\tNOT SET (default " << this->m_Parameter->kernel_type << ")" << "\n";
  else
    str << "classifier.svm.kernel-type\t" << this->m_Parameter->kernel_type << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.degree",this->m_Parameter->degree))
    str << "classifier.svm.degree\t\tNOT SET (default " << this->m_Parameter->degree << ")" << "\n";
  else
    str << "classifier.svm.degree\t\t" << this->m_Parameter->degree << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.gamma",this->m_Parameter->gamma))
    str << "classifier.svm.gamma\t\tNOT SET (default " << this->m_Parameter->gamma << ")" << "\n";
  else
    str << "classifier.svm.gamma\t\t" << this->m_Parameter->gamma << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.coef0",this->m_Parameter->coef0))
    str << "classifier.svm.coef0\t\tNOT SET (default " << this->m_Parameter->coef0 << ")" << "\n";
  else
    str << "classifier.svm.coef0\t\t" << this->m_Parameter->coef0 << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.nu",this->m_Parameter->nu))
    str << "classifier.svm.nu\t\tNOT SET (default " << this->m_Parameter->nu << ")" << "\n";
  else
    str << "classifier.svm.nu\t\t" << this->m_Parameter->nu << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.cache-size",this->m_Parameter->cache_size))
    str << "classifier.svm.cache-size\tNOT SET (default " << this->m_Parameter->cache_size << ")" << "\n";
  else
    str << "classifier.svm.cache-size\t" << this->m_Parameter->cache_size << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.c",this->m_Parameter->C))
    str << "classifier.svm.c\t\tNOT SET (default " << this->m_Parameter->C << ")" << "\n";
  else
    str << "classifier.svm.c\t\t" << this->m_Parameter->C << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.eps",this->m_Parameter->eps))
    str << "classifier.svm.eps\t\tNOT SET (default " << this->m_Parameter->eps << ")" << "\n";
  else
    str << "classifier.svm.eps\t\t" << this->m_Parameter->eps << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.p",this->m_Parameter->p))
    str << "classifier.svm.p\t\tNOT SET (default " << this->m_Parameter->p << ")" << "\n";
  else
    str << "classifier.svm.p\t\t" << this->m_Parameter->p << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.shrinking",this->m_Parameter->shrinking))
    str << "classifier.svm.shrinking\tNOT SET (default " << this->m_Parameter->shrinking << ")" << "\n";
  else
    str << "classifier.svm.shrinking\t" << this->m_Parameter->shrinking << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.probability",this->m_Parameter->probability))
    str << "classifier.svm.probability\tNOT SET (default " << this->m_Parameter->probability << ")" << "\n";
  else
    str << "classifier.svm.probability\t" << this->m_Parameter->probability << "\n";

  if(!this->GetPropertyList()->Get("classifier.svm.nr-weight",this->m_Parameter->nr_weight))
    str << "classifier.svm.nr-weight\tNOT SET (default " << this->m_Parameter->nr_weight << ")" << "\n";
  else
    str << "classifier.svm.nr-weight\t" << this->m_Parameter->nr_weight << "\n";
}

// Trying to assign from matrix to noOfPoints
void mitk::LibSVMClassifier::ReadXValues(LibSVM::svm_problem * problem, LibSVM::svm_node** xSpace, const Eigen::MatrixXd &X)
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

void mitk::LibSVMClassifier::ReadYValues(LibSVM::svm_problem * problem, const Eigen::MatrixXi &Y)
{
  problem->l = static_cast<int>(Y.rows());
  problem->y = static_cast<double *>(malloc(sizeof(double)  * problem->l));

  for (int i = 0; i < problem->l; ++i)
  {
    problem->y[i] = Y(i,0);
  }
}

void mitk::LibSVMClassifier::ReadWValues(LibSVM::svm_problem * problem)
{
  Eigen::MatrixXd & W = this->GetPointWiseWeight();
  int noOfPoints = problem->l;
  problem->W = static_cast<double *>(malloc(sizeof(double)  * noOfPoints));

  if (IsUsingPointWiseWeight())
  {
    for (int i = 0; i < noOfPoints; ++i)
    {
      problem->W[i] = W(i,0);
    }
  } else
  {
    for (int i = 0; i < noOfPoints; ++i)
    {
      problem->W[i] = 1;
    }
  }
}