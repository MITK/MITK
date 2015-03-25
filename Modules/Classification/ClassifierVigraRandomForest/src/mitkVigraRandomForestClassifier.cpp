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

// MITK includes
#include <mitkVigraRandomForestClassifier.h>

// Vigra includes

#include <vigra/random_forest.hxx>
#include <vigra/random_forest/rf_split.hxx>

// ITK include
#include <itkFastMutexLock.h>
#include <itkMultiThreader.h>


struct mitk::VigraRandomForestClassifier::TrainMultiThreaderData
{
  TrainMultiThreaderData(unsigned int numberOfThreads,unsigned int numberOfTrees,
                         const vigra::RandomForest<int> & refRF,
                         const vigra::GiniSplit & refSplitter,
                         const vigra::MultiArray<2, int> & refLabel,
                         const vigra::MultiArray<2, double> & refFeature)
    :m_NumberOfThreads(numberOfThreads),
      m_NumberOfTrees(numberOfTrees),
      m_RandomForest(refRF),
      m_Splitter(refSplitter),
      m_Label(refLabel),
      m_Feature(refFeature)
  {
    m_mutex = itk::FastMutexLock::New();
  }
  vigra::ArrayVector<vigra::RandomForest<int>::DecisionTree_t>  trees_;

  unsigned int m_NumberOfThreads, m_NumberOfTrees;
  const vigra::RandomForest<int> & m_RandomForest;
  const vigra::GiniSplit & m_Splitter;
  const vigra::MultiArray<2, int> & m_Label;
  const vigra::MultiArray<2, double> & m_Feature;
  itk::FastMutexLock::Pointer m_mutex;
};

struct mitk::VigraRandomForestClassifier::TestMultiThreaderData
{
  TestMultiThreaderData(unsigned int numberOfThreads,
                        const vigra::RandomForest<int> & refRF,
                        const vigra::MultiArray<2, double> & refFeature,
                        vigra::MultiArray<2, int> & refLabel,
                        vigra::MultiArray<2, double> & refProb)
    :m_NumberOfThreads(numberOfThreads),
      m_RandomForest(refRF),
      m_Feature(refFeature),
      m_Label(refLabel),
      m_Probabilities(refProb)
  {
  }

  unsigned int m_NumberOfThreads;
  const vigra::RandomForest<int> & m_RandomForest;
  const vigra::MultiArray<2, double> & m_Feature;
  vigra::MultiArray<2, int> & m_Label;
  vigra::MultiArray<2, double> & m_Probabilities;
};

mitk::VigraRandomForestClassifier::VigraRandomForestClassifier()
//  :m_NumberOfThreads(8)
{
}

mitk::VigraRandomForestClassifier::~VigraRandomForestClassifier()
{

}

bool mitk::VigraRandomForestClassifier::SupportsPointWiseWeight()
{
  return true;
}

void mitk::VigraRandomForestClassifier::Fit(const MatrixType & X, const VectorType & Y)
{
//   MBI --------
//  UpdateDataArrays(true);

  VigraMatrix2dType features = transform(X);
  VigraLabel2dType label = transform(Y);

  vigra::GiniSplit splitter;
  m_RandomForest.set_options().tree_count(1); // Number of trees that are calculated;
  m_RandomForest.learn(features, label,vigra::rf::visitors::VisitorBase(),splitter);

  TrainMultiThreaderData * data = new TrainMultiThreaderData(/*numberOfThreads*/8,/*treeCount*/100,m_RandomForest,splitter,label,features);
  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetNumberOfThreads(8);
  threader->SetSingleMethod(this->TrainTreesCallback,data);
  threader->SingleMethodExecute();

  // set result trees

  m_RandomForest.set_options().tree_count(/*m_TreeCount*/100);
  m_RandomForest.trees_ = data->trees_;

  delete data;
//   MBI --------
}

mitk::VigraRandomForestClassifier::VectorType mitk::VigraRandomForestClassifier::Predict(const MatrixType &X)
{
  VigraMatrix2dType P = VigraMatrix2dType(vigra::Shape2(X.rows(),m_RandomForest.class_count()));
  VigraLabel2dType Y = VigraLabel2dType(vigra::Shape2(X.rows(),1));
  VigraMatrix2dType v_X = transform(X);

  TestMultiThreaderData * data = new TestMultiThreaderData(/*numberOfThreads*/ 8,m_RandomForest,v_X,Y,P);
  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetNumberOfThreads(/*numberOfThreads*/8);
  threader->SetSingleMethod(this->PredictCallback,data);
  threader->SingleMethodExecute();

  VectorType e_Y = transform(Y);

  return e_Y;
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






/**
 * @brief mitk::DecisionForest::TrainTreesCallback
 * @param arg: itk::MultiThreader::ThreadInfoStruct
 * @return value: the thread state
 */

ITK_THREAD_RETURN_TYPE mitk::VigraRandomForestClassifier::TrainTreesCallback(void * arg)
{

  // Get the ThreadInfoStruct
  typedef itk::MultiThreader::ThreadInfoStruct  ThreadInfoType;
  ThreadInfoType * infoStruct = static_cast< ThreadInfoType * >( arg );
  // assigne the thread id
  const unsigned int threadId = infoStruct->ThreadID;
  // Get the user defined parameters containing all
  // neccesary informations
  TrainMultiThreaderData * userData = (TrainMultiThreaderData *)(infoStruct->UserData);
  unsigned int numberOfTreesToCalculate = 0;
  // define the number of tress the forest have to calculate
  numberOfTreesToCalculate = userData->m_NumberOfTrees / userData->m_NumberOfThreads;
  // the 0th thread takes the residuals
  if(threadId == 0) numberOfTreesToCalculate += userData->m_NumberOfTrees % userData->m_NumberOfThreads;

  if(numberOfTreesToCalculate != 0){
    // Initialize a splitter for the leraning process
    // TO DO: define Splitter copy constructur
    vigra::GiniSplit splitter;
//    splitter.UsePointBasedWeights(userData->m_Splitter.IsUsingPointBasedWeights());
//    splitter.UseRandomSplit(userData->m_Splitter.IsUsingRandomSplit());
//    splitter.SetPrecision(userData->m_Splitter.GetPrecision());
//    splitter.SetMaximumTreeDepth(userData->m_Splitter.GetMaximumTreeDepth());

    // Copy the Treestructure defined in userData
    vigra::RandomForest<int> rf = userData->m_RandomForest;
    //mitk::VigraRandomForestLearnAdaptor<AdaptorType,int> adaptor(rf);

    // set the numbers of trees calculated by this thread
    rf.trees_.clear();
    rf.set_options().tree_count(numberOfTreesToCalculate);
    rf.learn(userData->m_Feature, userData->m_Label,vigra::rf::visitors::VisitorBase(),splitter);
    // learn something
    //adaptor.learn(userData->m_Feature, userData->m_Label/*,vigra::rf::visitors::VisitorBase(),splitter*/,RandomNumberGenerator<>(RandomSeed));

    // Write the calculated trees into the return array
    userData->m_mutex->Lock(); // lock the critical areax

    for(unsigned int i = 0 ; i < rf.trees_.size(); i++)
      userData->trees_.push_back(rf.trees_[i]);
    //MITK_INFO << "Thread = " << threadId << " done" << numberOfTreesToCalculate << " with a out-of-bag error of: " << oob_v.oob_breiman << std::endl;;
    userData->m_mutex->Unlock(); // unlock it

  }

  ITK_THREAD_RETURN_TYPE value = NULL;
  return value;
}


ITK_THREAD_RETURN_TYPE mitk::VigraRandomForestClassifier::PredictCallback(void * arg)
{

  // Get the ThreadInfoStruct
  typedef itk::MultiThreader::ThreadInfoStruct  ThreadInfoType;
  ThreadInfoType * infoStruct = static_cast< ThreadInfoType * >( arg );
  // assigne the thread id
  const unsigned int threadId = infoStruct->ThreadID;

  // Get the user defined parameters containing all
  // neccesary informations
  TestMultiThreaderData * data = (TestMultiThreaderData *)(infoStruct->UserData);

  unsigned int numberOfRowsToCalculate = 0;
  //  MITK_INFO << data->m_Feature.shape()[0];

  // Get number of rows to calculate
  numberOfRowsToCalculate = data->m_Feature.shape()[0] / data->m_NumberOfThreads;

  unsigned int start_index = numberOfRowsToCalculate * threadId;
  unsigned int end_index = numberOfRowsToCalculate * (threadId+1);
//  MITK_INFO << start_index << "  "  << end_index;
  // the 0th thread takes the residuals
  if(threadId == data->m_NumberOfThreads-1) numberOfRowsToCalculate += data->m_Feature.shape()[0] % data->m_NumberOfThreads;

  vigra::TinyVector<vigra::MultiArrayIndex, 2> lowerBound(start_index,0);
  vigra::TinyVector<vigra::MultiArrayIndex, 2> upperBound(end_index,data->m_Feature.shape()[1]);
  vigra::MultiArray<2, double> split_features = data->m_Feature.subarray(lowerBound,upperBound);
  vigra::MultiArray<2, double> split_labels(vigra::Shape2(split_features.shape(0),data->m_Label.shape(1)));
  vigra::MultiArray<2, double> split_probability(vigra::Shape2(split_features.shape(0),data->m_Probabilities.shape(1)));

  data->m_RandomForest.predictLabels(split_features,split_labels);
  data->m_RandomForest.predictProbabilities(split_features, split_probability);

  for(unsigned int target_row = start_index, source_row = 0; target_row < end_index; target_row++, source_row++)
  {
    for(int col = 0; col < data->m_Label.shape()[1]; col++)
    {
      data->m_Label(target_row,col) = split_labels(source_row,col);
    }
    for(int col = 0; col < data->m_Probabilities.shape()[1]; col++)
    {
      data->m_Probabilities(target_row,col) = split_probability(source_row,col);
    }
  }

  ITK_THREAD_RETURN_TYPE value = NULL;
  return value;
}


