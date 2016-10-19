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

#ifndef USNAVIGATIONTARGETUPDATEFILTER_H
#define USNAVIGATIONTARGETUPDATEFILTER_H

#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkDataStorage.h"

namespace itk {
  template<class T> class SmartPointer;
}

class vtkPolyData;
template<class T> class vtkSmartPointer;

namespace mitk {
class DataNode;
class LookupTableProperty;
class BaseGeometry;

/**
 * \brief Calculates a score for target placement for every vertex of a given surface.
 * The scores are calculated for placing the targets equally on the surface. It
 * is assumed that the surface approximates a sphere. For every vertex of the
 * surface a score between 0 and 1 is calculated. The scores are stored in the
 * vtkSurface as a vtkFloatArray. The name of this array can be set by calling
 * SetScalarArrayIdentifier().
 *
 * The target surface and the number of targets (between zero and four) or an
 * optimal angle between the targets must be set first. The scores are then
 * updated every time a target node is added by SetControlNode() or removed by
 * RemovePositionOfTarget().
 *
 */
class USNavigationTargetUpdateFilter : public itk::Object
{
public:
  mitkClassMacroItkParent(USNavigationTargetUpdateFilter, itk::Object)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
   * \brief Set the target surface for which the scores should be calculated.
   * \param targetStructure DataNode containing a mitk::Surface for the target structure
   */
  void SetTargetStructure(itk::SmartPointer<DataNode> targetStructure);

  /**
   * \brief Setter for the number of targets to be placed on the target surface.
   * The optimal angle for placing the targets is calculated based on this number.
   *
   * \return false if the number of targets is smaller than one or larger than four
   */
  bool SetNumberOfTargets(unsigned int numberOfTargets);

  /**
   * \brief Setter for the optimal angle between to targets on the target surface.
   * This value overwrites the value calculated by a previous call to
   * SetNumberOfTargets() and will be overwritten when SetNumberOfTargets() is
   * called afterwards.
   */
  void SetOptimalAngle(double optimalAngle);

  /**
   * \return Angle calculated by SetNumberOfTargets() or explicitly set by SetOptimalAngle()
   */
  double GetOptimalAngle();

  /**
   * \brief Sets the identifier for the vtkFloatArray of scores.
   * This array is stored as scalars of the vtkPolyData of the target surface.
   */
  void SetScalarArrayIdentifier(std::string scalarArrayIdentifier);

  /**
   * \brief Set whether the maximum score or the minimum score between the targets should be used.
   * Whenever more then one target position is already set, the scores to every
   * target position are calculated. The resulting score is the the minimum of
   * this scores (default) or the maximum.
   *
   */
  void SetUseMaximumScore(bool useMaximumScore);

  /**
   * \brief Sets the origin of the given node for the filter.
   * This origin is the position of an already placed target and is used to
   * calculate the scores for the surface vertices.
   *
   * \param id number of the node which position should be set or updated
   * \param controlNode DataNode which origin should be set into the filter
   */
  void SetControlNode(unsigned int id, itk::SmartPointer<DataNode> controlNode);

  /**
   * \brief Removes the position of the target with the current id from the filter.
   */
  void RemovePositionOfTarget(unsigned int id);

  /**
   * \brief Removes all target positions from the filter and resets the scores.
   */
  void Reset();

protected:
  USNavigationTargetUpdateFilter();
  virtual ~USNavigationTargetUpdateFilter();

  void UpdateTargetScores();

  vtkSmartPointer<vtkPolyData> GetVtkPolyDataOfTarget();
  itk::SmartPointer<mitk::BaseGeometry> GetGeometryOfTarget();

  itk::SmartPointer<DataNode>                   m_TargetStructure;

  unsigned int                                  m_NumberOfTargets;
  double                                        m_OptimalAngle;
  std::string                                   m_ScalarArrayIdentifier;
  bool                                          m_UseMaximumScore;

  std::vector<itk::SmartPointer<DataNode> >     m_ControlNodesVector;
};
} // namespace mitk

#endif // USNAVIGATIONTARGETUPDATEFILTER_H
