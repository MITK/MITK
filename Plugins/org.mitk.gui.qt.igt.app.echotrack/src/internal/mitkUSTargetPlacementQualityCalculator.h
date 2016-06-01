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

#ifndef MITKUSTARGETPLACEMENTQUALITYCALCULATOR_H
#define MITKUSTARGETPLACEMENTQUALITYCALCULATOR_H

#include <mitkCommon.h>
#include <mitkNumericTypes.h>
#include <itkObjectFactory.h>

template<class T> class vtkSmartPointer;
class vtkPolyData;

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class Surface;
class PointSet;

/**
 * \brief Calculates qualitiy metrics for given target positions.
 * The calculated metrics are:
 *  - the distance of the centers of mass of the target surface and the target
 *    points
 *  - the differences of the angles between the target points to the given
 *    optimal angle
 *  - the mean of the angle differences
 */
class USTargetPlacementQualityCalculator : public itk::Object
{
public:
  mitkClassMacroItkParent(USTargetPlacementQualityCalculator, itk::Object)
  itkNewMacro(Self)

  /**
   * \brief Setter for the surface where the targets are placed around.
   */
  itkSetMacro(TargetSurface, itk::SmartPointer<Surface>)

  /**
   * \brief Setter for the target points which are placed around the target surface.
   */
  itkSetMacro(TargetPoints, itk::SmartPointer<PointSet>)

  /**
   * \brief Setter for the optimal angle of the target placement.
   * This angle is subtracted from the target angles when calculating the angle
   * differences.
   */
  itkSetMacro(OptimalAngle, double)

  /**
   * \brief Calculates the quality metrics.
   * This method should be called before calling the getters for the metrics.
   */
  void Update();

  itkGetMacro(CentersOfMassDistance, double)
  itkGetMacro(MeanAngleDifference, double)
  itkGetMacro(AngleDifferences, mitk::VnlVector)

protected:
  USTargetPlacementQualityCalculator();
  virtual ~USTargetPlacementQualityCalculator();

  vtkSmartPointer<vtkPolyData> GetTransformedPolydata();

  itk::SmartPointer<Surface>  m_TargetSurface;
  itk::SmartPointer<PointSet> m_TargetPoints;
  double                      m_OptimalAngle;

  double                      m_CentersOfMassDistance;
  double                      m_MeanAngleDifference;
  mitk::VnlVector             m_AngleDifferences;
};
} // namespace mitk

#endif // MITKUSTARGETPLACEMENTQUALITYCALCULATOR_H
