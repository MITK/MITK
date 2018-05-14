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


#ifndef _MITK_TractographyForest_H
#define _MITK_TractographyForest_H

#include <mitkBaseData.h>
#include <MitkFiberTrackingExports.h>

#undef DIFFERENCE
#define VIGRA_STATIC_LIB
#include <vigra/random_forest.hxx>

namespace mitk {

/**
   * \brief Class containing random forest datastructures used for fiber tractography;   */
class MITKFIBERTRACKING_EXPORT TractographyForest : public BaseData
{
public:

  void UpdateOutputInformation() override;
  void SetRequestedRegionToLargestPossibleRegion() override;
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
  bool VerifyRequestedRegion() override;
  void SetRequestedRegion(const itk::DataObject*) override;

  mitkClassMacro( TractographyForest, BaseData )
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  mitkNewMacro1Param(Self, std::shared_ptr< vigra::RandomForest<int> >) // custom constructor

  int GetNumFeatures() const;
  int GetNumTrees() const;
  int GetNumClasses() const;
  int GetMaxTreeDepth() const;
  int IndexToClassLabel(int idx) const;
  bool HasForest() const;
  void PredictProbabilities(vigra::MultiArray<2, float>& features, vigra::MultiArray<2, float>& probabilities) const;
  std::shared_ptr< const vigra::RandomForest<int> > GetForest() const
  { return m_Forest; }

protected:

  TractographyForest( std::shared_ptr< vigra::RandomForest<int> > forest = nullptr );
  ~TractographyForest() override;

  void PrintSelf(std::ostream &os, itk::Indent indent) const override;

private:

  std::shared_ptr< vigra::RandomForest<int> > m_Forest;   ///< random forest classifier

};

} // namespace mitk

#endif /*  _MITK_TractographyForest_H */
