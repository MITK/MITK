/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKNAVIGATIONDATASLICEVISUALIZATION_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATASLICEVISUALIZATION_H_HEADER_INCLUDED_

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkVector.h"

namespace mitk
{
/**Documentation
* \brief Control the position and orientation of rendered slices with NavigationData
*
* A NavigationDataToNavigationDataFilter that takes NavigationData as input and
* sets the position and, optionally, the orientation of the slice plane for a
* user-specified renderer.
*
* \ingroup IGT
*/
class MITKIGT_EXPORT NavigationDataSliceVisualization : public NavigationDataToNavigationDataFilter
{
  public:
    mitkClassMacro(NavigationDataSliceVisualization, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    enum ViewDirection
    {
      /**
       * Tracked slice planes are NOT re-oriented, only the position
       * of the slice plane is controlled by the input navigation data.
       */
      Axial = 0,
      Sagittal,
      Frontal,
      /**
       * Axial plane "tilted" about the lateral vector so that it is coplanar
       * with the tool trajectory
       */
      AxialOblique,
      /**
       * Sagittal plane "tilted" about the axial vector so that it is coplanar
       * with the tool trajectory
       */
      SagittalOblique,
      /**
       * Slice plane normal to the tool trajectory
       */
      Oblique
    };

    /**
     * \brief Set/get the renderer that visualizes the navigation data
     */
    itkSetObjectMacro(Renderer,BaseRenderer);
    itkGetConstObjectMacro(Renderer,BaseRenderer);

    /**
     * \brief Set/get the tip offset used for plane tracking
     *
     * This is an additional offset vector applied to the input navigation
     * data. It is defined in tool tip coordinates. In other words:
     *
     * \code
     * position_slice = position_input + orient_input.rotate(TipOffset)
     * \endcode
     *
     * Default is [0,0,0].
     */
    itkSetMacro(TipOffset, Vector3D);
    itkGetConstMacro(TipOffset,Vector3D);

    /**
     * \brief Set/get the tool trajectory used to define the cutting plane
     * normal direction.
     *
     * This vector, defined in tool tip coordinates, applies only when the
     * ViewDirection is Oblique.
     *
     * Default is [0,0,-1].
     */
    virtual void SetToolTrajectory(Vector3D direction);
    itkGetConstMacro(ToolTrajectory, Vector3D);

    /**
     * \brief Set/get the world vertical vector used to define the y-axis of the
     * cutting plane
     *
     * This vector, defined in world coordinates, applies only when the
     * ViewDirection is Oblique. It is projected onto the cutting plane to
     * define the vertical orientation of the slice.
     *
     * The direction of this vector does not matter (i.e. [0,1,0] is the same
     * as [0,-1,0]). The direction will be determined automatically by
     * choosing the one that is closest to the direction of the y-axis of the
     * PlaneGeometry before each update. This way, the anatomical axis
     * directions that get set initially will be maintained after every update
     * of this filter.
     *
     * Default is [0,1,0].
     */
    itkSetMacro(WorldVerticalVector, Vector3D);
    itkGetConstMacro(WorldVerticalVector, Vector3D);

    /**
     * \brief Set/get the orientation of the sliced plane
     *
     * Default is Axial.
     */
    itkSetEnumMacro(ViewDirection,ViewDirection);
    itkGetEnumMacro(ViewDirection,ViewDirection);

  protected:
    NavigationDataSliceVisualization();
    void GenerateData() override;

    BaseRenderer::Pointer m_Renderer;
    Vector3D m_TipOffset;
    Vector3D m_ToolTrajectory;
    Vector3D m_WorldVerticalVector;
    ViewDirection m_ViewDirection;
};

} // end namespace mitk

#endif // NEMOSLICEVISUALIZATIONFILTER_H
