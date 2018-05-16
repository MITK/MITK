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
#include <mitkPURFClassifier.h>
#include <mitkThresholdSplit.h>
#include <mitkPUImpurityLoss.h>
#include <mitkImpurityLoss.h>
#include <mitkLinearSplitting.h>
#include <mitkProperties.h>

// Vigra includes
#include <vigra/random_forest.hxx>
#include <vigra/random_forest/rf_split.hxx>

// ITK include
#include <itkFastMutexLock.h>
#include <itkMultiThreader.h>
#include <itkCommand.h>

typedef mitk::ThresholdSplit<mitk::LinearSplitting< mitk::PUImpurityLoss<> >,int,vigra::ClassificationTag> DefaultPUSplitType;

struct mitk::PURFClassifier::Parameter
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

struct mitk::PURFClassifier::TrainingData
{
  TrainingData(unsigned int numberOfTrees,
    const vigra::RandomForest<int> & refRF,
    const DefaultPUSplitType & refSplitter,
    const vigra::MultiArrayView<2, double> refFeature,
    const vigra::MultiArrayView<2, int> refLabel,
    const Parameter parameter)
    : m_ClassCount(0),
    m_NumberOfTrees(numberOfTrees),
    m_RandomForest(refRF),
    m_Splitter(refSplitter),
    m_Feature(refFeature),
    m_Label(refLabel),
    m_Parameter(parameter)
  {
    m_mutex = itk::FastMutexLock::New();
  }
  vigra::ArrayVector<vigra::RandomForest<int>::DecisionTree_t>  trees_;

  int m_ClassCount;
  unsigned int m_NumberOfTrees;
  const vigra::RandomForest<int> & m_RandomForest;
  const DefaultPUSplitType & m_Splitter;
  const vigra::MultiArrayView<2, double> m_Feature;
  const vigra::MultiArrayView<2, int> m_Label;
  itk::FastMutexLock::Pointer m_mutex;
  Parameter m_Parameter;
};

struct mitk::PURFClassifier::PredictionData
{
  PredictionData(const vigra::RandomForest<int> & refRF,
    const vigra::MultiArrayView<2, double> refFeature,
    vigra::MultiArrayView<2, int> refLabel,
    vigra::MultiArrayView<2, double> refProb,
    vigra::MultiArrayView<2, double> refTreeWeights)
    : m_RandomForest(refRF),
    m_Feature(refFeature),
    m_Label(refLabel),
    m_Probabilities(refProb),
    m_TreeWeights(refTreeWeights)
  {
  }
  const vigra::RandomForest<int> & m_RandomForest;
  const vigra::MultiArrayView<2, double> m_Feature;
  vigra::MultiArrayView<2, int> m_Label;
  vigra::MultiArrayView<2, double> m_Probabilities;
  vigra::MultiArrayView<2, double> m_TreeWeights;
};

mitk::PURFClassifier::PURFClassifier()
  :m_Parameter(nullptr)
{
  itk::SimpleMemberCommand<mitk::PURFClassifier>::Pointer command = itk::SimpleMemberCommand<mitk::PURFClassifier>::New();
  command->SetCallbackFunction(this, &mitk::PURFClassifier::ConvertParameter);
  this->GetPropertyList()->AddObserver( itk::ModifiedEvent(), command );
}

mitk::PURFClassifier::~PURFClassifier()
{
}

void mitk::PURFClassifier::SetClassProbabilities(Eigen::VectorXd probabilities)
{
  m_ClassProbabilities = probabilities;
}

Eigen::VectorXd mitk::PURFClassifier::GetClassProbabilites()
{
  return m_ClassProbabilities;
}

bool mitk::PURFClassifier::SupportsPointWiseWeight()
{
  return true;
}

bool mitk::PURFClassifier::SupportsPointWiseProbability()
{
  return true;
}


vigra::ArrayVector<double> mitk::PURFClassifier::CalculateKappa(const Eigen::MatrixXd & /* X_in */, const Eigen::MatrixXi & Y_in)
{
  int maximumValue = Y_in.maxCoeff();
  vigra::ArrayVector<double> kappa(maximumValue + 1);
  vigra::ArrayVector<double> counts(maximumValue + 1);
  for (int i = 0; i < Y_in.rows(); ++i)
  {
    counts[Y_in(i, 0)] += 1;
  }
  for (int i = 0; i < maximumValue+1; ++i)
  {
    if (counts[i] > 0)
    {
      kappa[i] = counts[0] * m_ClassProbabilities[i] / counts[i] + 1;
    }
    else
    {
      kappa[i] = 1;
    }
  }
  return kappa;
}


void mitk::PURFClassifier::Train(const Eigen::MatrixXd & X_in, const Eigen::MatrixXi &Y_in)
{
  this->ConvertParameter();

  PURFData* purfData = new PURFData;
  purfData->m_Kappa = this->CalculateKappa(X_in, Y_in);

  DefaultPUSplitType splitter;
  splitter.UsePointBasedWeights(m_Parameter->UsePointBasedWeights);
  splitter.UseRandomSplit(m_Parameter->UseRandomSplit);
  splitter.SetPrecision(m_Parameter->Precision);
  splitter.SetMaximumTreeDepth(m_Parameter->TreeDepth);
  splitter.SetAdditionalData(purfData);

  // Weights handled as member variable
  if (m_Parameter->UsePointBasedWeights)
  {
    // Set influence of the weight (0 no influenc to 1 max influence)
    this->m_PointWiseWeight.unaryExpr([this](double t){ return std::pow(t, this->m_Parameter->WeightLambda) ;});

    vigra::MultiArrayView<2, double> W(vigra::Shape2(this->m_PointWiseWeight.rows(),this->m_PointWiseWeight.cols()),this->m_PointWiseWeight.data());
    splitter.SetWeights(W);
  }

  vigra::MultiArrayView<2, double> X(vigra::Shape2(X_in.rows(),X_in.cols()),X_in.data());
  vigra::MultiArrayView<2, int> Y(vigra::Shape2(Y_in.rows(),Y_in.cols()),Y_in.data());

  m_RandomForest.set_options().tree_count(1); // Number of trees that are calculated;

  m_RandomForest.set_options().use_stratification(m_Parameter->Stratification);
  m_RandomForest.set_options().sample_with_replacement(m_Parameter->SampleWithReplacement);
  m_RandomForest.set_options().samples_per_tree(m_Parameter->SamplesPerTree);
  m_RandomForest.set_options().min_split_node_size(m_Parameter->MinimumSplitNodeSize);

  m_RandomForest.learn(X, Y,vigra::rf::visitors::VisitorBase(),splitter);

  std::unique_ptr<TrainingData> data(new TrainingData(m_Parameter->TreeCount,m_RandomForest,splitter,X,Y, *m_Parameter));

  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetSingleMethod(this->TrainTreesCallback,data.get());
  threader->SingleMethodExecute();

  // set result trees
  m_RandomForest.set_options().tree_count(m_Parameter->TreeCount);
  m_RandomForest.ext_param_.class_count_ = data->m_ClassCount;
  m_RandomForest.trees_ = data->trees_;

  // Set Tree Weights to default
  m_TreeWeights = Eigen::MatrixXd(m_Parameter->TreeCount,1);
  m_TreeWeights.fill(1.0);
  delete purfData;
}

Eigen::MatrixXi mitk::PURFClassifier::Predict(const Eigen::MatrixXd &X_in)
{
  // Initialize output Eigen matrices
  m_OutProbability = Eigen::MatrixXd(X_in.rows(),m_RandomForest.class_count());
  m_OutProbability.fill(0);
  m_OutLabel = Eigen::MatrixXi(X_in.rows(),1);
  m_OutLabel.fill(0);

  // If no weights provided
  if(m_TreeWeights.rows() != m_RandomForest.tree_count())
  {
    m_TreeWeights = Eigen::MatrixXd(m_RandomForest.tree_count(),1);
    m_TreeWeights.fill(1);
  }

  vigra::MultiArrayView<2, double> P(vigra::Shape2(m_OutProbability.rows(),m_OutProbability.cols()),m_OutProbability.data());
  vigra::MultiArrayView<2, int> Y(vigra::Shape2(m_OutLabel.rows(),m_OutLabel.cols()),m_OutLabel.data());
  vigra::MultiArrayView<2, double> X(vigra::Shape2(X_in.rows(),X_in.cols()),X_in.data());
  vigra::MultiArrayView<2, double> TW(vigra::Shape2(m_RandomForest.tree_count(),1),m_TreeWeights.data());

  std::unique_ptr<PredictionData> data;
  data.reset(new PredictionData(m_RandomForest, X, Y, P, TW));

  itk::MultiThreader::Pointer threader = itk::MultiThreader::New();
  threader->SetSingleMethod(this->PredictCallback, data.get());
  threader->SingleMethodExecute();

  m_Probabilities = data->m_Probabilities;
  return m_OutLabel;
}

ITK_THREAD_RETURN_TYPE mitk::PURFClassifier::TrainTreesCallback(void * arg)
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
    DefaultPUSplitType splitter;
    splitter.UsePointBasedWeights(data->m_Splitter.IsUsingPointBasedWeights());
    splitter.UseRandomSplit(data->m_Splitter.IsUsingRandomSplit());
    splitter.SetPrecision(data->m_Splitter.GetPrecision());
    splitter.SetMaximumTreeDepth(data->m_Splitter.GetMaximumTreeDepth());
    splitter.SetWeights(data->m_Splitter.GetWeights());
    splitter.SetAdditionalData(data->m_Splitter.GetAdditionalData());

    rf.trees_.clear();
    rf.set_options().tree_count(numberOfTreesToCalculate);
    rf.set_options().use_stratification(data->m_Parameter.Stratification);
    rf.set_options().sample_with_replacement(data->m_Parameter.SampleWithReplacement);
    rf.set_options().samples_per_tree(data->m_Parameter.SamplesPerTree);
    rf.set_options().min_split_node_size(data->m_Parameter.MinimumSplitNodeSize);
    rf.learn(data->m_Feature, data->m_Label,vigra::rf::visitors::VisitorBase(),splitter);

    data->m_mutex->Lock();

    for(const auto & tree : rf.trees_)
      data->trees_.push_back(tree);

    data->m_ClassCount = rf.class_count();
    data->m_mutex->Unlock();
  }

  return NULL;

}

ITK_THREAD_RETURN_TYPE mitk::PURFClassifier::PredictCallback(void * arg)
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

  // the last thread takes the residuals
  if(threadId == infoStruct->NumberOfThreads-1) {
    end_index += data->m_Feature.shape()[0] % infoStruct->NumberOfThreads;
  }

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


  return NULL;

}

void  mitk::PURFClassifier::ConvertParameter()
{
  if(this->m_Parameter == nullptr)
    this->m_Parameter = new Parameter();
  // Get the proerty                                                                      // Some defaults

  MITK_INFO("PURFClassifier") << "Convert Parameter";
  if(!this->GetPropertyList()->Get("usepointbasedweight",this->m_Parameter->UsePointBasedWeights))      this->m_Parameter->UsePointBasedWeights = false;
  if(!this->GetPropertyList()->Get("userandomsplit",this->m_Parameter->UseRandomSplit))                 this->m_Parameter->UseRandomSplit = false;
  if(!this->GetPropertyList()->Get("treedepth",this->m_Parameter->TreeDepth))                           this->m_Parameter->TreeDepth = 20;
  if(!this->GetPropertyList()->Get("treecount",this->m_Parameter->TreeCount))                           this->m_Parameter->TreeCount = 100;
  if(!this->GetPropertyList()->Get("minimalsplitnodesize",this->m_Parameter->MinimumSplitNodeSize))     this->m_Parameter->MinimumSplitNodeSize = 5;
  if(!this->GetPropertyList()->Get("precision",this->m_Parameter->Precision))                           this->m_Parameter->Precision = mitk::eps;
  if(!this->GetPropertyList()->Get("samplespertree",this->m_Parameter->SamplesPerTree))                 this->m_Parameter->SamplesPerTree = 0.6;
  if(!this->GetPropertyList()->Get("samplewithreplacement",this->m_Parameter->SampleWithReplacement))   this->m_Parameter->SampleWithReplacement = true;
  if(!this->GetPropertyList()->Get("lambda",this->m_Parameter->WeightLambda))                           this->m_Parameter->WeightLambda = 1.0; // Not used yet
  //  if(!this->GetPropertyList()->Get("samplewithreplacement",this->m_Parameter->Stratification))
  this->m_Parameter->Stratification = vigra::RF_NONE; // no Property given
}

void mitk::PURFClassifier::PrintParameter(std::ostream & str)
{
  if(this->m_Parameter == nullptr)
  {
    MITK_WARN("PURFClassifier") << "Parameters are not initialized. Please call ConvertParameter() first!";
    return;
  }

  this->ConvertParameter();

  // Get the proerty                                                                      // Some defaults
  if(!this->GetPropertyList()->Get("usepointbasedweight",this->m_Parameter->UsePointBasedWeights))
    str << "usepointbasedweight\tNOT SET (default " << this->m_Parameter->UsePointBasedWeights << ")" << "\n";
  else
    str << "usepointbasedweight\t" << this->m_Parameter->UsePointBasedWeights << "\n";

  if(!this->GetPropertyList()->Get("userandomsplit",this->m_Parameter->UseRandomSplit))
    str << "userandomsplit\tNOT SET (default " << this->m_Parameter->UseRandomSplit << ")" << "\n";
  else
    str << "userandomsplit\t" << this->m_Parameter->UseRandomSplit << "\n";

  if(!this->GetPropertyList()->Get("treedepth",this->m_Parameter->TreeDepth))
    str << "treedepth\t\tNOT SET (default " << this->m_Parameter->TreeDepth << ")" << "\n";
  else
    str << "treedepth\t\t" << this->m_Parameter->TreeDepth << "\n";

  if(!this->GetPropertyList()->Get("minimalsplitnodesize",this->m_Parameter->MinimumSplitNodeSize))
    str << "minimalsplitnodesize\tNOT SET (default " << this->m_Parameter->MinimumSplitNodeSize << ")" << "\n";
  else
    str << "minimalsplitnodesize\t" << this->m_Parameter->MinimumSplitNodeSize << "\n";

  if(!this->GetPropertyList()->Get("precision",this->m_Parameter->Precision))
    str << "precision\t\tNOT SET (default " << this->m_Parameter->Precision << ")" << "\n";
  else
    str << "precision\t\t" << this->m_Parameter->Precision << "\n";

  if(!this->GetPropertyList()->Get("samplespertree",this->m_Parameter->SamplesPerTree))
    str << "samplespertree\tNOT SET (default " << this->m_Parameter->SamplesPerTree << ")" << "\n";
  else
    str << "samplespertree\t" << this->m_Parameter->SamplesPerTree << "\n";

  if(!this->GetPropertyList()->Get("samplewithreplacement",this->m_Parameter->SampleWithReplacement))
    str << "samplewithreplacement\tNOT SET (default " << this->m_Parameter->SampleWithReplacement << ")" << "\n";
  else
    str << "samplewithreplacement\t" << this->m_Parameter->SampleWithReplacement << "\n";

  if(!this->GetPropertyList()->Get("treecount",this->m_Parameter->TreeCount))
    str << "treecount\t\tNOT SET (default " << this->m_Parameter->TreeCount << ")" << "\n";
  else
    str << "treecount\t\t" << this->m_Parameter->TreeCount << "\n";

  if(!this->GetPropertyList()->Get("lambda",this->m_Parameter->WeightLambda))
    str << "lambda\t\tNOT SET (default " << this->m_Parameter->WeightLambda << ")" << "\n";
  else
    str << "lambda\t\t" << this->m_Parameter->WeightLambda << "\n";

  //  if(!this->GetPropertyList()->Get("samplewithreplacement",this->m_Parameter->Stratification))
  //  this->m_Parameter->Stratification = vigra:RF_NONE; // no Property given
}

void mitk::PURFClassifier::UsePointWiseWeight(bool val)
{
  mitk::AbstractClassifier::UsePointWiseWeight(val);
  this->GetPropertyList()->SetBoolProperty("usepointbasedweight",val);
}

void mitk::PURFClassifier::SetMaximumTreeDepth(int val)
{
  this->GetPropertyList()->SetIntProperty("treedepth",val);
}

void mitk::PURFClassifier::SetMinimumSplitNodeSize(int val)
{
  this->GetPropertyList()->SetIntProperty("minimalsplitnodesize",val);
}

void mitk::PURFClassifier::SetPrecision(double val)
{
  this->GetPropertyList()->SetDoubleProperty("precision",val);
}

void mitk::PURFClassifier::SetSamplesPerTree(double val)
{
  this->GetPropertyList()->SetDoubleProperty("samplespertree",val);
}

void mitk::PURFClassifier::UseSampleWithReplacement(bool val)
{
  this->GetPropertyList()->SetBoolProperty("samplewithreplacement",val);
}

void mitk::PURFClassifier::SetTreeCount(int val)
{
  this->GetPropertyList()->SetIntProperty("treecount",val);
}

void mitk::PURFClassifier::SetWeightLambda(double val)
{
  this->GetPropertyList()->SetDoubleProperty("lambda",val);
}

void mitk::PURFClassifier::SetRandomForest(const vigra::RandomForest<int> & rf)
{
  this->SetMaximumTreeDepth(rf.ext_param().max_tree_depth);
  this->SetMinimumSplitNodeSize(rf.options().min_split_node_size_);
  this->SetTreeCount(rf.options().tree_count_);
  this->SetSamplesPerTree(rf.options().training_set_proportion_);
  this->UseSampleWithReplacement(rf.options().sample_with_replacement_);
  this->m_RandomForest = rf;
}

const vigra::RandomForest<int> & mitk::PURFClassifier::GetRandomForest() const
{
  return this->m_RandomForest;
}
