/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureVtkMapper2D_h
#define mitkPlanarFigureVtkMapper2D_h

#include <mitkLocalStorageHandler.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkVtkMapper.h>
#include <MitkPlanarFigureExports.h>

#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <optional>

class vtkActor2D;
class vtkPolyDataMapper2D;
class vtkPropAssembly;
class vtkTextActor;

namespace mitk
{
  /**
   * \brief Draws planar figures in 2D render windows.
   *
   * The figure is drawn on top of the scene as anti-aliased geometry in
   * display coordinates, so line widths and handle sizes are given in logical
   * pixels and do not depend on the zoom. The base color never changes with
   * the state of the figure: hovering and selection add a halo, and the
   * handles appear only while the mouse hovers over the figure.
   *
   * Node properties:
   *   - \c color (ColorProperty): base color of the figure
   *   - \c opacity (FloatProperty): opacity of the figure
   *   - \c line \c width (FloatProperty): width of the outline in logical pixels
   *   - \c planarfigure.drawname (BoolProperty): label with the node name
   *   - \c planarfigure.drawquantities (BoolProperty): label with the measurements
   *   - \c font.size (IntProperty): font size of the label in points
   *   - \c animated.pulse (BoolProperty): while true, the figure grows and shrinks by up to 8 percent
   *   - \c animated.pulse.frequency (FloatProperty): pulses per second, 0.5 by default
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureVtkMapper2D : public VtkMapper
  {
  public:
    mitkClassMacro(PlanarFigureVtkMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self);

    /**
     * \brief Adds the default properties of planar figures in 2D render windows.
     *
     * \param[in] node The node to add the properties to.
     * \param[in] renderer The renderer to add renderer-specific properties for, or \c nullptr.
     * \param[in] overwrite If \c true, existing properties are replaced.
     */
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    vtkProp *GetVtkProp(BaseRenderer *renderer) override;

  protected:
    PlanarFigureVtkMapper2D();
    ~PlanarFigureVtkMapper2D() override;

    void GenerateDataForRenderer(BaseRenderer *renderer) override;
    void ResetMapper(BaseRenderer *renderer) override;

  private:
    /** \brief Everything the drawing depends on; the figure is drawn again only if it changes. */
    struct BuildKey
    {
      itk::ModifiedTimeType FigureTime = 0;
      itk::ModifiedTimeType NodeTime = 0;
      itk::ModifiedTimeType PropertiesTime = 0;
      itk::ModifiedTimeType RendererPropertiesTime = 0;
      const void *RendererPlane = nullptr;
      itk::ModifiedTimeType RendererPlaneTime = 0;
      vtkMTimeType CameraTime = 0;
      int ViewportWidth = 0;
      int ViewportHeight = 0;
      int DPI = 0;
      PlanarFigureInteractor::Feedback Feedback;
      double AnimationTime = 0.0;

      bool operator==(const BuildKey &) const = default;
    };

    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
      vtkSmartPointer<vtkActor2D> m_Actor;
      vtkSmartPointer<vtkPolyDataMapper2D> m_Mapper;
      vtkSmartPointer<vtkTextActor> m_TextActor;
      std::optional<BuildKey> m_BuildKey;
    };

    BuildKey GetBuildKey(BaseRenderer *renderer) const;
    void Draw(BaseRenderer *renderer, LocalStorage *localStorage) const;

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
