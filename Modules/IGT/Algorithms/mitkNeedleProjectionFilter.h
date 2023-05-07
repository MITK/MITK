/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNeedleProjectionFilter_h
#define mitkNeedleProjectionFilter_h

#include "MitkIGTExports.h"

// MITK
#include <mitkNavigationDataPassThroughFilter.h>
#include <mitkNavigationData.h>
#include <mitkPointSet.h>
#include <mitkGeometry3D.h>

namespace mitk {
  /**
  * \brief This filter projects a needle's path onto a plane.
  *
  * To use it, hook it up to a NavigationDataStream,
  * select an input and set an AffineTransform 3D that represents the target plane.
  * You can then call GetProjection to retrieve a pointset that represents the projected path.
  * You may change the PointSet's properties, these changes will not be overwritten.
  * If no Input is selected, the target Pointset will not update
  * If no Target Plane is selected, The projection line will always be 40 cm long
  * Any points you add to the pointSet will be overwritten during the next Update.
  * The point with index zero is the Tip of the Needle.
  * The Point with index one is the projection onto the plane.
  *
  * Projection will happen onto an extension of the plane as well - the filter does not regard boundaries
  * This Filter currently only supports projection of one needle. Extension to multiple needles / planes should be easy.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NeedleProjectionFilter : public NavigationDataPassThroughFilter
  {
  public:
    mitkClassMacro(NeedleProjectionFilter, NavigationDataPassThroughFilter);
    itkNewMacro(Self);

    virtual void SelectInput(int i);

    itkGetMacro(TargetPlane, mitk::AffineTransform3D::Pointer);
    itkSetMacro(TargetPlane, mitk::AffineTransform3D::Pointer);
    itkGetMacro(Projection,  mitk::PointSet::Pointer);
    /** Sets the tool axis for this filter. The default tool axis is along the z-axis in
     *  tool coordinates. */
    void SetToolAxisForFilter(mitk::Point3D point);
    /** Sets whether the tool axis should be visualized. This is required if no surface is available.
     *  If disabled only the projection and not the axis is shown. It's disabled by default. */
    void ShowToolAxis(bool enabled);

  protected:
    NeedleProjectionFilter();
    ~NeedleProjectionFilter() override;

    void GenerateData() override;

    mitk::AffineTransform3D::Pointer m_TargetPlane;

    mitk::PointSet::Pointer          m_Projection;
    mitk::PointSet::Pointer          m_OriginalPoints;
    bool                             m_ShowToolAxis;
    mitk::Point3D                    m_ToolAxis;

    int                              m_SelectedInput;

    /** Internal method for initialization of the projection / tool axis representation
     *  by the point set m_OriginalPoints. */
    void InitializeOriginalPoints(mitk::Point3D toolAxis, bool showToolAxis);


    /**
    * \brief Creates an Affine Transformation from a Navigation Data Object.
    */
    mitk::AffineTransform3D::Pointer NavigationDataToTransform(const mitk::NavigationData * nd);
    /**
    * \brief Creates an Geometry 3D Object from an AffineTransformation.
    */
    mitk::Geometry3D::Pointer TransformToGeometry(mitk::AffineTransform3D::Pointer transform);
  };
} // namespace mitk


#endif
