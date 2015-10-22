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

#ifndef TumorInvasionAnalysis_H
#define TumorInvasionAnalysis_H

//MITK
#include "mitkCommon.h"
#include <MitkTumorInvasionAnalysisExports.h>
// DATA
#include <mitkDataCollection.h>
// FORESTS
#include <mitkVigraRandomForestClassifier.h>


namespace mitk
{
/**
 * @brief The TumorInvasionAnalysis class - Classifies Tumor progression using RF and predicts on new cases
 *
 * Provides a way to learn tumor progression from examples, and predict the growth using random forests.
 *
 *
 * Expected Inputs are DataCollections. The Tumor Segmentations (current extent) are expected to be binary (0 - healthy tissue, 1 - tumor tissue),
 * likewise the target extent that is to be learnt.
 *
 */
class MITKTUMORINVASIONANALYSIS_EXPORT TumorInvasionClassification
{
public:

  TumorInvasionClassification();
  ~TumorInvasionClassification(){}

  /**
   * @brief SelectTrainingSamples
   *
   * Selects healthy/tumor tissue samples to be included into training.
   * \warn This modifies the target data items in the data collection.
   * Afterwards the target data is structured as follows: 0 -voxels excluded from training , 1 - healthy sample , 2 - "tuning-into-tumor" sample
   *
   *
   *
   * @param collection
   * @param erodeTrainingMask - Performs erosion on target such that border voxels of tumor region will not be included into training
   */
  void SelectTrainingSamples(DataCollection* collection, unsigned int mode = 0);


  /**
   * @brief PrepareResponseSamples
   *
   * Selects healthy/tumor tissue samples to be included into training, and weights them to achieve a given ratio.
   * \warn This modifies the target data items in the data collection.
   * Afterwards the target data is encoded as follows:
   * 0 - excluded (e.g. out of brainmask)
   * 1 - no involvement (healthy all the time)
   * 2 - formerly healthy, now tumor (rezivid)
   * 3 - formerly tumor, now necroses (responsive)
   */
  void PrepareResponseSamples(DataCollection* collection);

  /**
   * @brief LearnProgressionFeatures
   *
   *
   * @param collection - data collection used for training
   * @param modalitiesList - modalities (data item names from collection) that are to be used for training
   * @param forestSize - number of trees that are trained
   * @param treeDepth - maximal depth for a single tree
   */

  void LearnProgressionFeatures(DataCollection* collection, std::vector<std::string> modalitiesList, size_t forestSize = 300, size_t treeDepth = 10);


  /**
   * @brief PredictGrowth - Classify voxels into remaining healthy / turning into tumor
   * @param collection
   */
  void PredictInvasion(DataCollection* collection, std::vector<std::string> modalitiesList);


  /**
   * @brief SanitizeResults - Performs an Opening Operation on tha data to remove isolated misclassifications
   *
   * RESULTOPEN
   *
   * @param collection
   */
  void SanitizeResults(DataCollection* collection, std::string resultID = "RESULT");


  void SetTrainMargin(size_t dil2d, size_t dil3d)
  {
    m_TargetDil2D = dil2d;
    m_TargetDil3D = dil3d;
  }

  /**
   * @brief SetClassRatio - set ratio of tumor voxels to healthy voxels that is to be used for training
   *
   * \warn perfect ratio cannot allways be achieved, since by default all tumor voxels are included into training (since they ususally are to rare to be skipped)
   *
   * @param ratio
   */

  void SetClassRatio(ScalarType ratio)
  {
    m_ClassRatio = ratio;
  }

  void SetGlobalsStatsFile(std::string fn)
  {
    m_GlobalStatsFile = fn;
  }

  /**
   * @brief SamplesWeightingActivated If activated a weighted mask for the samples is calculated, weighting samples according to their location and ratio
   * @param isActive
   */
  void SamplesWeightingActivated(bool isActive)
  {
    m_WeightSamples = isActive;
  }


  /**
   * @brief SetTargetID sets binary mask denoting future tumor area
   * @param targetID
   */
  void SetTargetID(std::string targetID = "TARGET")
  {
    m_TargetID = targetID;
  }

  /**
   * @brief SetTumorID sets binary mask denoting current tumor area
   * @param tumorID
   */
  void SetTumorID(std::string tumorID = "GTV")
  {
    m_TumorID = tumorID;
  }

  /**
   * @brief SetMaskID sets binary mask denoting area that is evluated/included in training
   * @param maskID
   */
  void SetMaskID(std::string maskID = "BRAINMASK")
  {
    m_MaskID= maskID;
  }

  /**
   * @brief SetResultID sets data item name in which prediction results are stored
   * @param resultID
   */
  void SetResultID(std::string resultID = "RESULT")
  {
    m_ResultID = resultID;
  }

  /**
   * @brief UseRandomizedTrees - use random splits to build up forest
   * @param useRandom
   */
  void UseRandomizedTrees(bool useRandom)
  {
    m_Randomize=useRandom;
  }


  /**
   * @brief DescriptionToLogFile - Append to log file:  patient ids and time steps contained in this data collection.
   * @param collection
   * @param outputFile
   */
  void DescriptionToLogFile(DataCollection* collection, std::string outputFile);

  /**
   * @brief SaveRandomForest - Saves a trained random forest
   * @param filename
   */
  void SaveRandomForest(std::string filename);


  /**
   * @brief LoadRandomForest - loads a random forest to be used for classification
   * @param filename
   */
  void LoadRandomForest(std::string);

protected:

private:
  // DATA
  mitk::VigraRandomForestClassifier m_Forest;
  unsigned int m_TargetDil2D;
  unsigned int m_TargetDil3D;

  ScalarType m_ClassRatio;

  std::string m_GlobalStatsFile;

  std::string m_TargetID;
  std::string m_TumorID;
  std::string m_MaskID;
  std::string m_ResultID;

  bool m_Randomize;
  bool m_WeightSamples;

};
} // end namespace
#endif
