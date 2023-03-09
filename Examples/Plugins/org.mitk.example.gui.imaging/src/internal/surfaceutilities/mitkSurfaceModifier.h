/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceModifier_h
#define mitkSurfaceModifier_h

// mitk headers
#include <mitkCommon.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>

// itk headers
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

class vtkPolyDataNormals;

namespace mitk
{
  /** Documentation
   * @brief This class offer some methods to modify a surface.
   */

  class SurfaceModifier : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SurfaceModifier, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** @brief Adds gaussian noise to a surface (means to all vertexes).
        * @param maxNoiseVectorLenght Limits the length of the noise vector of each vertex. Value -1 disables the limit.
       * Default value is also -1.
        */
      bool PerturbeSurface(mitk::Surface::Pointer surface,
                           double varianceX,
                           double varianceY,
                           double varianceZ,
                           double maxNoiseVectorLenght = -1);

    /** @brief Adds gaussian noise to a part of the vertexes of the surface.
        @param outlierChance The chance to perturbe a vertex. Consequently this is also approximately the percentage of
       vertexes that will be perturbed.
     */
    bool AddOutlierToSurface(
      mitk::Surface::Pointer surface, double varianceX, double varianceY, double varianceZ, double outlierChance);

    /** @brief Transforms a surface with a given transform. This method transforms the vertexes which means manipulating
     * the vtkPolyData.
      *        In some cases this is needed, for example if you want to use the transformed polydata later on.
      */
    bool TransformSurface(mitk::Surface::Pointer surface,
                          itk::Matrix<double, 3, 3> TransformationR,
                          itk::Vector<double, 3> TransformationT);

    /** @brief Transforms a surface with a given transform (uses the center of gravity of the surface as origin). This
     * method transforms the vertexes which means manipulating the vtkPolyData.
      *        In some cases this is needed, for example if you want to use the transformed polydata later on.
      * @param OverallTransformationR Returns the overall transformation in world coordinates. (rotation)
      * @param OverallTransformationT Returns the overall transformation in world coordinates. (translation)
      */
    bool TransformSurfaceCoGCoordinates(mitk::Surface::Pointer surface,
                                        itk::Matrix<double, 3, 3> TransformationR,
                                        itk::Vector<double, 3> TransformationT,
                                        itk::Matrix<double, 3, 3> &OverallTransformationR,
                                        itk::Vector<double, 3> &OverallTransformationT);

    /** @brief Moves the surface (respectively its center of gravity) to the center of the coordinate system. */
    bool MoveSurfaceToCenter(mitk::Surface::Pointer surface);

    /** @brief Moves the surface (respectively its center of gravity) to the center of the coordinate system.
        @param TransformR (return value) returns the rotation of the transform which was applied to the surface to move
       it to the origin.
        @param TransformT (return value) returns the translation of the transform which was applied to the surface to
       move it to the origin.
    */
    bool MoveSurfaceToCenter(mitk::Surface::Pointer surface,
                             itk::Matrix<double, 3, 3> &TransformR,
                             itk::Vector<double, 3> &TransformT);

    /** @brief Creates a deep copy (clone) of the given surface and returns it */
    mitk::Surface::Pointer DeepCopy(mitk::Surface::Pointer surface);

  protected:
    SurfaceModifier();
    ~SurfaceModifier() override;

    mitk::Point3D PerturbePoint(
      mitk::Point3D point, double varianceX, double varianceY, double varianceZ);

    /** @brief perturbes a point along the given axis */
    mitk::Point3D PerturbePointAlongAxis(mitk::Point3D point, mitk::Vector3D axis, double variance);

    mitk::Point3D TransformPoint(mitk::Point3D point,
                                 itk::Matrix<double, 3, 3> TransformationR,
                                 itk::Vector<double, 3> TransformationT);

    mitk::Point3D GetCenterOfGravity(mitk::Surface::Pointer surface);

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_myRandomGenerator;
  };

} // namespace mitk
#endif
