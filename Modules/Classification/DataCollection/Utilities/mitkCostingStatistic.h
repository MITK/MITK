/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCostingStatistic_h
#define mitkCostingStatistic_h

#include <MitkDataCollectionExports.h>

#include <mitkDataCollection.h>

#include <iostream>

namespace mitk {
  class MITKDATACOLLECTION_EXPORT CostingStatistic
  {
  public:
    void SetCollection(DataCollection::Pointer collection);
    DataCollection::Pointer GetCollection();

    void SetMaskName(std::string name) {m_MaskName = name;}
    void SetCombinedA(std::string name) {m_CombinedProbabilityA = name; }
    void SetCombinedB(std::string name) {m_CombinedProbabilityB = name; }
    void SetCombinedLabel(std::string name) {m_CombinedLabelName = name; }
    void SetProbabilitiesA (std::vector<std::string> probA) {m_ProbabilityClassA = probA;}
    void SetProbabilitiesB (std::vector<std::string> probB) {m_ProbabilityClassB = probB;}

    bool UpdateCollection();
    bool CalculateClass(double threshold);

    bool WriteStatistic(std::ostream &out,std::ostream &sout, double stepSize, std::string shortLabel);
  private:

    DataCollection::Pointer m_Collection;
    std::string m_MaskName;
    std::string m_CombinedProbabilityA;
    std::string m_CombinedProbabilityB;
    std::string m_CombinedLabelName;

    std::vector<std::string> m_ProbabilityClassA;
    std::vector<std::string> m_ProbabilityClassB;
  };
}

#endif // mitkCostingStatistic_h
