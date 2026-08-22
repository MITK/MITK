/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeVtkMapper3D_h
#define mitkBoundingShapeVtkMapper3D_h

#include <MitkBoundingShapeExports.h>
#include <mitkVtkMapper.h>

namespace mitk
{
  /** \brief VTK-based 3D mapper for rendering bounding shapes as translucent boxes with handles.
   *
   * Renders a bounding box (GeometryData) as a translucent box with interactive handles at
   * the face centers for resizing. The box is drawn unlit, with camera-relative shading
   * baked into its faces so that they stay distinguishable. Selected handles are
   * highlighted with a different color.
   *
   * \sa BoundingShapeVtkMapper2D, BoundingShapeInteractor, VtkMapper
   */
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeVtkMapper3D : public VtkMapper
  {
  public:
    /** \brief Set default rendering properties for bounding shape 3D visualization.
     *
     * \param[in] node      The data node to set properties on.
     * \param[in] renderer  The renderer context, or \c nullptr for all renderers.
     * \param[in] overwrite If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    mitkClassMacro(BoundingShapeVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /** \copydoc VtkMapper::ApplyColorAndOpacityProperties */
    void ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor) override;

    /** \brief Get the VTK prop assembly for 3D rendering.
     *
     * \param[in] renderer The renderer to get the prop for.
     * \return The VTK prop containing the bounding shape box and handle actors.
     */
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;
    //   virtual void UpdateVtkTransform(mitk::BaseRenderer* renderer) override;
  protected:
    void GenerateDataForRenderer(BaseRenderer *renderer) override;

  private:
    BoundingShapeVtkMapper3D();
    ~BoundingShapeVtkMapper3D() override;

    BoundingShapeVtkMapper3D(const Self &);
    Self &operator=(const Self &);

    class Impl;
    Impl *m_Impl;
  };
}

#endif
