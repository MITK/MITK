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
#include <mitkThresholdSplit.h>
#include <mitkImpurityLoss.h>
#include <mitkLinearSplitting.h>
#include <mitkProperties.h>

// Vigra includes
#include <vigra/random_forest.hxx>
#include <vigra/random_forest/rf_split.hxx>

// ITK include
#include <itkFastMutexLock.h>
#include <itkMultiThreader.h>

typedef mitk::ThresholdSplit<mitk::LinearSplitting< mitk::ImpurityLoss<> >,int,vigra::ClassificationTag> DefaultSplitType;

struct mitk::VigraRandomForestClassifier::Parameter
{
  vigra::RF_OptionTag Stratification;
  bool SampleWithReplacement;
  bool UseRandomSplit;
  bool UsePointBasedWeights;
  int TreeCount;
  int MinimumSplitNodeSize;
  int TreeDepth;
  double Precision;
  double WeightLambda;
  double SamplesPerTree;

};

struct mitk::VigraRandomForestClassifier::TrainingData
{
  TrainingData(unsigned int numberOfTrees,
               const vigra::RandomForest<int> & refRF,
               const DefaultSplitType & refSplitter,
               const vigra::MultiArrayView<2, double> refFeature,
               const vigra::MultiArrayView<2, int> refLabel)
    : m_ClassCount(0),
      m_NumberOfTrees(numberOfTrees),
      m_RandomForest(refRF),
      m_Splitter(refSplitter),
      m_Feature(refFeature),
      m_Label(refLabel)
  {
    m_mutex = itk::FastMutexLock::New();
  }
  vigra::ArrayVector<vigra::RandomForest<int>::DecisionTree_t>  trees_;

  int m_ClassCount;
  unsigned int m_NumberOfTrees;
  const vigra::RandomForest<int> & m_RandomForest;
  const DefaultSplitType & m_Splitter;
  const vigra::MultiArrayView<2, double> m_Feature;
  const vigra::MultiArrayView<2, int> m_Label;
  itk::FastMutexLock::Pointer m_mutex;
};

struct mitk::VigraRandomForestClassifier::PredictionData
{
  PredictionData(const vigra::RandomForest<int> & refRF,
                 const vigra::MultiArrayView<2, double> refFeature,
                 vigra::MultiArrayView<2, int> refLabel,
                 vigra::MultiArrayView<2, double> refProb)
    : m_RandomForest(refRF),
      m_Feature(refFeature),
      m_Label(refLabel),
      m_Probabilities(refProb)
  {
  }
  const vigra::RandomForest<int> & m_RandomForest;
  const vigra::MultiArrayView<2, double> m_Feature;
  vigra::MultiArrayView<2, int> m_Label;
  vigra::MultiArrayView<2, double> m_Probabilities;
};

mitk::VigraRandomForestClassifier::VigraRandomForestClassifier()
  :m_Parameter(nullptr)
{
}

mitk::VigraRandomForestClassifier::~VigraRandomForestClassifier()
{

}

bool mitk::VigraRandomForestClassifier::SupportsPointWiseWeight()
{
  return true;
}

bool mitk::VigraRandomForestClassifier::SupportsPointWiseProbability()
{
  return true;
}

vigra::MultiArrayView<2, double> mitk::VigraRandomForestClassifier::GetPointWiseWeight()
{
  vigra::MultiArrayView<2, double> eigenview;
  return eigenview;
}

void mitk::VigraRandomForestClassifier::Train(const Eigen::MatrixXd & X_in, const Eigen::MatrixXi &Y_in)
{
  DefaultSplitType splitter;
  splitter.UsePointBasedWeights(m_Parameter->UsePointBasedWeights);
  splitter.UseRandomSplit(m_Parameter->UseRandomSplit);
  splitter.SetPrecision(m_Parameter->Precision);
  splitter.SetMaximumTreeDepth(m_Parameter->TreeDepth);
  //  splitter.SetWeights(m_Parameter->/*WeightLambda*/);

  if (m_Parameter->UsePointBasedWeights)
    splitter.SetWeights(this->GetPointWiseWeight());

  vigra::MultiArrayView<2, double> X(vigra::Shape2(X_in.rows(),X_in.cols()),X_in.data());
  vigra::MultiArrayView<2, int> Y(vigra::Shape2(Y_in.rows(),Y_in.cols()),Y_in.data());


  m_RandomForest.set_options().tree_count(1); // Number of trees that are calculated;
  m_RandomForest.learn(X, Y,vigra::rf::visitors::VisitorBase(),splitter);

  std::auto_ptr<TrainingData> data(new TrainingData(m_Parameter->TreeCount,m_RandomForest,splitter,X,Y));

  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetSingleMethod(this->TrainTreesCallback,data.get());
  threader->SingleMethodExecute();

  // set result trees
  m_RandomForest.set_options().tree_count(m_Parameter->TreeCount);
  m_RandomForest.ext_param_.class_count_ = data->m_ClassCount;
  m_RandomForest.trees_ = data->trees_;
}

Eigen::MatrixXi mitk::VigraRandomForestClassifier::Predict(const Eigen::MatrixXd &X_in)
{

  // Initialize output Eigen matrices
  m_OutProbability = Eigen::MatrixXd(X_in.rows(),m_RandomForest.class_count());
  m_OutProbability.fill(0);
  m_OutLabel = Eigen::MatrixXi(X_in.rows(),1);
  m_OutLabel.fill(0);

  vigra::MultiArrayView<2, double> P(vigra::Shape2(m_OutProbability.rows(),m_OutProbability.cols()),m_OutProbability.data());
  vigra::MultiArrayView<2, int> Y(vigra::Shape2(m_OutLabel.rows(),m_OutLabel.cols()),m_OutLabel.data());
  vigra::MultiArrayView<2, double> X(vigra::Shape2(X_in.rows(),X_in.cols()),X_in.data());

  std::auto_ptr<PredictionData> data;
  data.reset( new PredictionData(m_RandomForest,X,Y,P));

  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetSingleMethod(this->PredictCallback,data.get());
  threader->SingleMethodExecute();

  return m_OutLabel;
}


ITK_THREAD_RETURN_TYPE mitk::VigraRandomForestClassifier::TrainTreesCallback(void * arg)
{

  // Get the ThreadInfoStruct
  typedef itk::MultiThreader::ThreadInfoStruct  ThreadInfoType;
  ThreadInfoType * infoStruct = static_cast< ThreadInfoType * >( arg );

  TrainingData * data = (TrainingData *)(infoStruct->UserData);
  unsigned int numberOfTreesToCalculate = 0;

  // define the number of tress the forest have to calculate
  numberOfTreesToCalculate = data->m_NumberOfTrees / infoStruct->NumberOfThreads;

  // the 0th thread takes the residuals
  if(infoStruct->ThreadID == 0) numberOfTreesToCalculate += data->m_NumberOfTrees % infoStruct->NumberOfThreads;

  if(numberOfTreesToCalculate != 0){
    // Copy the Treestructure defined in userData
    vigra::RandomForest<int> rf = data->m_RandomForest;

    // Initialize a splitter for the leraning process
    DefaultSplitType splitter;
    splitter.UsePointBasedWeights(data->m_Splitter.IsUsingPointBasedWeights());
    splitter.UseRandomSplit(data->m_Splitter.IsUsingRandomSplit());
    splitter.SetPrecision(data->m_Splitter.GetPrecision());
    splitter.SetMaximumTreeDepth(data->m_Splitter.GetMaximumTreeDepth());


    rf.trees_.clear();
    rf.set_options().tree_count(numberOfTreesToCalculate);
    rf.learn(data->m_Feature, data->m_Label,vigra::rf::visitors::VisitorBase(),splitter);

    data->m_mutex->Lock();

    for(const auto & tree : rf.trees_)
      data->trees_.push_back(tree);

    data->m_ClassCount = rf.class_count();
    data->m_mutex->Unlock();

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
  PredictionData * data = (PredictionData *)(infoStruct->UserData);
  unsigned int numberOfRowsToCalculate = 0;

  // Get number of rows to calculate
  numberOfRowsToCalculate = data->m_Feature.shape()[0] / infoStruct->NumberOfThreads;

  unsigned int start_index = numberOfRowsToCalculate * threadId;
  unsigned int end_index = numberOfRowsToCalculate * (threadId+1);

  // the 0th thread takes the residuals
  if(threadId == infoStruct->NumberOfThreads-1) numberOfRowsToCalculate += data->m_Feature.shape()[0] % infoStruct->NumberOfThreads;

  vigra::MultiArrayView<2, double> split_features;
  vigra::MultiArrayView<2, int> split_labels;
  vigra::MultiArrayView<2, double> split_probability;
  {
    vigra::TinyVector<vigra::MultiArrayIndex, 2> lowerBound(start_index,0);
    vigra::TinyVector<vigra::MultiArrayIndex, 2> upperBound(end_index,data->m_Feature.shape(1));
    split_features = data->m_Feature.subarray(lowerBound,upperBound);
  }

  {
    vigra::TinyVector<vigra::MultiArrayIndex, 2> lowerBound(start_index,0);
    vigra::TinyVector<vigra::MultiArrayIndex, 2> upperBound(end_index, data->m_Label.shape(1));
    split_labels = data->m_Label.subarray(lowerBound,upperBound);
  }

  {
    vigra::TinyVector<vigra::MultiArrayIndex, 2> lowerBound(start_index,0);
    vigra::TinyVector<vigra::MultiArrayIndex, 2> upperBound(end_index,data->m_Probabilities.shape(1));
    split_probability = data->m_Probabilities.subarray(lowerBound,upperBound);
  }

  data->m_RandomForest.predictLabels(split_features,split_labels);
  data->m_RandomForest.predictProbabilities(split_features, split_probability);

  ITK_THREAD_RETURN_TYPE value = NULL;
  return value;
}


void  mitk::VigraRandomForestClassifier::ConvertParameter()
{
  if(this->m_Parameter == nullptr)
    this->m_Parameter = new Parameter();
  // Get the proerty                                                                      // Some defaults
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.usepointbasedweight",this->m_Parameter->UsePointBasedWeights))      this->m_Parameter->UsePointBasedWeights = false;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.treedepth",this->m_Parameter->TreeDepth))                           this->m_Parameter->TreeDepth = 20;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.treecount",this->m_Parameter->TreeCount))                           this->m_Parameter->TreeCount = 100;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.minimalsplitnodesize",this->m_Parameter->MinimumSplitNodeSize))     this->m_Parameter->MinimumSplitNodeSize = 5;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.precision",this->m_Parameter->Precision))                           this->m_Parameter->Precision = mitk::eps;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplespertree",this->m_Parameter->SamplesPerTree))                 this->m_Parameter->SamplesPerTree = 0.6;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplewithreplacement",this->m_Parameter->SampleWithReplacement))   this->m_Parameter->SampleWithReplacement = true;
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.lambda",this->m_Parameter->WeightLambda))                           this->m_Parameter->WeightLambda = 1.0; // Not used yet
  //  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplewithreplacement",this->m_Parameter->Stratification))
  this->m_Parameter->Stratification = vigra::RF_NONE; // no Property given
}

void mitk::VigraRandomForestClassifier::PrintParameter()
{
  if(this->m_Parameter == nullptr)
  {
    MITK_WARN("VigraRandomForestClassifier") << "Parameters are not initialized. Please call ConvertParameter() first!";
    return;
  }
  // Get the proerty                                                                      // Some defaults
  if(!this->GetPropertyList()->Get("classifier.vigra-rf.usepointbasedweight",this->m_Parameter->UsePointBasedWeights))
    MITK_INFO << "classifier.vigra-rf.usepointbasedweight\tNOT SET (default " << this->m_Parameter->UsePointBasedWeights << ")";
  else
    MITK_INFO << "classifier.vigra-rf.usepointbasedweight\t" << this->m_Parameter->UsePointBasedWeights;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.treedepth",this->m_Parameter->TreeDepth))
    MITK_INFO << "classifier.vigra-rf.treedepth\t\tNOT SET (default " << this->m_Parameter->TreeDepth << ")";
  else
    MITK_INFO << "classifier.vigra-rf.treedepth\t\t" << this->m_Parameter->TreeDepth;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.minimalsplitnodesize",this->m_Parameter->MinimumSplitNodeSize))
    MITK_INFO << "classifier.vigra-rf.minimalsplitnodesize\tNOT SET (default " << this->m_Parameter->MinimumSplitNodeSize << ")";
  else
    MITK_INFO << "classifier.vigra-rf.minimalsplitnodesize\t" << this->m_Parameter->MinimumSplitNodeSize;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.precision",this->m_Parameter->Precision))
    MITK_INFO << "classifier.vigra-rf.precision\t\tNOT SET (default " << this->m_Parameter->Precision << ")";
  else
    MITK_INFO << "classifier.vigra-rf.precision\t\t" << this->m_Parameter->Precision;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplespertree",this->m_Parameter->SamplesPerTree))
    MITK_INFO << "classifier.vigra-rf.samplespertree\t\tNOT SET (default " << this->m_Parameter->SamplesPerTree << ")";
  else
    MITK_INFO << "classifier.vigra-rf.samplespertree\t\t" << this->m_Parameter->SamplesPerTree;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplewithreplacement",this->m_Parameter->SampleWithReplacement))
    MITK_INFO << "classifier.vigra-rf.samplewithreplacement\tNOT SET (default " << this->m_Parameter->SampleWithReplacement << ")";
  else
    MITK_INFO << "classifier.vigra-rf.samplewithreplacement\t" << this->m_Parameter->SampleWithReplacement;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.treecount",this->m_Parameter->TreeCount))
    MITK_INFO << "classifier.vigra-rf.treecount\t\tNOT SET (default " << this->m_Parameter->TreeCount << ")";
  else
    MITK_INFO << "classifier.vigra-rf.treecount\t\t" << this->m_Parameter->TreeCount;

  if(!this->GetPropertyList()->Get("classifier.vigra-rf.lambda",this->m_Parameter->WeightLambda))
    MITK_INFO << "classifier.vigra-rf.lambda\t\tNOT SET (default " << this->m_Parameter->WeightLambda << ")";
  else
    MITK_INFO << "classifier.vigra-rf.lambda\t\t" << this->m_Parameter->WeightLambda;

  //  if(!this->GetPropertyList()->Get("classifier.vigra-rf.samplewithreplacement",this->m_Parameter->Stratification))
  //  this->m_Parameter->Stratification = vigra:RF_NONE; // no Property given
}

void mitk::VigraRandomForestClassifier::UsePointBasedWeights(bool val)
{
  this->GetPropertyList()->SetBoolProperty("classifier.vigra-rf.usepointbasedweight",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetMaximumTreeDepth(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.vigra-rf.treedepth",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetMinimumSplitNodeSize(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.vigra-rf.minimalsplitnodesize",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetPrecision(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.vigra-rf.precision",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetSamplesPerTree(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.vigra-rf.samplespertree",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::UseSampleWithReplacement(bool val)
{
  this->GetPropertyList()->SetBoolProperty("classifier.vigra-rf.samplewithreplacement",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetTreeCount(int val)
{
  this->GetPropertyList()->SetIntProperty("classifier.vigra-rf.treecount",val);
  this->ConvertParameter();
}

void mitk::VigraRandomForestClassifier::SetWeightLambda(double val)
{
  this->GetPropertyList()->SetDoubleProperty("classifier.vigra-rf.lambda",val);
  this->ConvertParameter();
}

