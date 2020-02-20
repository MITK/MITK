/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLRDensityEstimation_h
#define mitkLRDensityEstimation_h

#include <vigra/matrix.hxx>
#include <vigra/random_forest.hxx>

#include <mitkDataCollection.h>

#include <MitkCLImportanceWeightingExports.h>

namespace mitk
{
  class MITKCLIMPORTANCEWEIGHTING_EXPORT LRDensityEstimation
  {
  public:
    typedef vigra::MultiArray<2, double> MatrixType;

    void SetCollection(DataCollection::Pointer data);
    DataCollection::Pointer GetCollection();

    void SetTestMask(std::string name);
    std::string GetTestMask();

    void SetTrainMask(std::string name);
    std::string GetTrainMask();

    void SetWeightName(std::string name);
    std::string GetWeightName();

    void Update();
    void WeightsForAll(mitk::DataCollection::Pointer train, mitk::DataCollection::Pointer test);
    void SetModalities(std::vector<std::string> modalities);
    std::vector<std::string> GetModalities();

    void Predict();

  private:
    DataCollection::Pointer m_Collection;
    MatrixType m_Weights;
    std::string m_TestMask;
    std::string m_TrainMask;
    std::string m_WeightName;
    std::vector<std::string> m_Modalities;

    std::string m_WeightFileName;
  }; // class LRDensityEstimation
} //namespace mitk

#endif //mitkLRDensityEstimation_h
