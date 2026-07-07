/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGizmoMapper2D_h
#define mitkGizmoMapper2D_h

#include <mitkVtkMapper.h>

#include <vtkPolyDataMapper.h>

namespace mitk
{
  // forward declaration
  class Gizmo;

  /**
   * \brief 2D mapper for mitk::Gizmo.
   *
   * Paints a similar representation as for 3D (see mitk::Gizmo itself).
   * The three axes of the manipulated object are visualized as arrows. They are
   * pickable and will report "move along axis" or "scale" when clicked
   * at the arrow shafts or the arrow tips, respectively.
   *
   * For usability, the circles for rotation are omitted in 2D because they
   * would most often coincide with the arrows, making distinction difficult.
   *
   * \sa Gizmo, GizmoInteractor
   */
  class GizmoMapper2D : public mitk::VtkMapper
  {
  public:
    mitkClassMacro(GizmoMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Provides the given node with a set of default properties.
     *
     * Configures color, scalar visibility, lookup table for axis coloring,
     * and visibility properties for the gizmo rendering.
     *
     * \param node The DataNode to decorate with default properties.
     * \param renderer When not nullptr, generate renderer-specific properties.
     * \param overwrite Whether already existing properties shall be overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode *node,
                                     mitk::BaseRenderer *renderer = nullptr,
                                     bool overwrite = false);

    /**
     * \brief Return the vtkProp that represents the rendering result.
     *
     * \param renderer The renderer for which to retrieve the vtkProp.
     * \return The vtkActor used for rendering this gizmo in the given renderer.
     */
    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override { return m_LSH.GetLocalStorage(renderer)->m_Actor; }

    /**
     * \brief Reset the mapper, setting its result to invisible.
     *
     * \param renderer The renderer whose local storage should be reset.
     */
    void ResetMapper(mitk::BaseRenderer *renderer) override;

    /**
     * \brief Return the internal vtkPolyData for the given renderer.
     *
     * This serves for picking by the associated GizmoInteractor class.
     * The poly data contains scalar values that encode the handle type
     * (e.g., Gizmo::MoveAlongAxisX, Gizmo::ScaleX).
     *
     * \param renderer The renderer for which to retrieve the poly data.
     * \return The vtkPolyData used as mapper input for the given renderer.
     */
    vtkPolyData *GetVtkPolyData(mitk::BaseRenderer *renderer);

  private:
    /** \brief Retrieve the input Gizmo data object from the associated DataNode. */
    const Gizmo *GetInput();

    /**
     * \brief Update the vtkProp by generating 2D arrow geometry for the current slice.
     *
     * Creates arrows along the three gizmo axes and a center disk for free movement,
     * projected onto the current world plane geometry of the renderer.
     *
     * \param renderer The renderer for which to generate the 2D representation.
     */
    void GenerateDataForRenderer(mitk::BaseRenderer *renderer) override;

    /**
     * \brief Apply visual properties (line width, lookup table, scalar visibility)
     *        to the local storage actor.
     *
     * \param renderer The renderer whose local storage should be updated.
     */
    void ApplyVisualProperties(BaseRenderer *renderer);

    /** \brief Render-window-specific data storage for the 2D gizmo mapper. */
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      /** \brief The overall rendering result actor. */
      vtkSmartPointer<vtkActor> m_Actor;

      /** \brief The mapper of the resulting vtkPolyData (2D contours for 2D mapper). */
      vtkSmartPointer<vtkPolyDataMapper> m_VtkPolyDataMapper;

      /** \brief Last time this storage has been updated. */
      itk::TimeStamp m_LastUpdateTime;

      LocalStorage();
    };

    /** \brief Render-window-specific local storage handler. */
    LocalStorageHandler<LocalStorage> m_LSH;
  };

} // namespace

#endif
