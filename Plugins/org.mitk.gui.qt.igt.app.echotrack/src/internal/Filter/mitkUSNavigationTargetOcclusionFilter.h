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

#ifndef USNAVIGATIONTARGETOCCLUSIONFILTER_H
#define USNAVIGATIONTARGETOCCLUSIONFILTER_H

#include "mitkNavigationDataPassThroughFilter.h"
#include "mitkDataStorage.h"


namespace itk {
  template<class T> class SmartPointer;
}

class vtkPolyData;
template<class T> class vtkSmartPointer;

namespace mitk {
class DataNode;

/**
 * \brief NavigationData filter calcuting occluded positions on a target surface.
 * The occlusion caused by obstacle structures is calculated between the current
 * NavigationData position and a given target surface.
 */
class USNavigationTargetOcclusionFilter : public NavigationDataPassThroughFilter
{
public:
  mitkClassMacro(USNavigationTargetOcclusionFilter, NavigationDataPassThroughFilter)
  itkNewMacro(Self)

  /**
   * \brief Sets the target structure for which the occluded positions should be calculated.
   * \param targetStructure DataNode conatining a mitk::Surface
   */
  void SetTargetStructure(itk::SmartPointer<DataNode> targetStructure);

  /**
   * \brief Sets the obstacle structures which can occlude the target structure.
   * \param obstacleStructures Set of DataNodes containing a mitk::Surface each
   */
  void SetObstacleStructures(DataStorage::SetOfObjects::ConstPointer obstacleStructures);

  /**
   * \brief Sets the index of the input which is used for occlusion calculation.
   * The occlusion will be calculated between the NavigationData position of
   * this input and the target structure.
   */
  void SelectStartPositionInput(unsigned int n);

protected:
  USNavigationTargetOcclusionFilter();
  virtual ~USNavigationTargetOcclusionFilter();

  virtual void GenerateData();

  /**
   * \brief Returns the vtk poly data of the target structure.
   * \return vtkPolyData if the target structure data node contains one, returns null otherwise
   */
  vtkSmartPointer<vtkPolyData> GetVtkPolyDataOfTarget();

  DataStorage::SetOfObjects::ConstPointer   m_ObstacleStructures;
  itk::SmartPointer<DataNode>               m_TargetStructure;

  unsigned int                              m_StartPositionInput;

  std::vector<bool>                         m_OccludedPositions;
};
} // namespace mitk

#endif // USNAVIGATIONTARGETOCCLUSIONFILTER_H
