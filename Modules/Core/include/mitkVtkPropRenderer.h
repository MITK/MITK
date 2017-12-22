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

#ifndef mitkVtkPropRenderer_h
#define mitkVtkPropRenderer_h

#include "mitkBaseRenderer.h"
#include <MitkCoreExports.h>
#include <itkCommand.h>
#include <mitkDataStorage.h>
#include <mitkRenderingManager.h>

#include <map>
#include <utility>

class vtkRenderWindow;
class vtkLight;
class vtkLightKit;
class vtkWorldPointPicker;
class vtkPointPicker;
class vtkCellPicker;
class vtkTextActor;
class vtkTextProperty;
class vtkAssemblyPath;

#include <vtkAssemblyPaths.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  class Mapper;

  /*!
  \brief VtkPropRenderer

  VtkPropRenderer organizes the MITK rendering process. The MITK rendering process is completely integrated into the VTK
  rendering pipeline.
  The vtkMitkRenderProp is a custom vtkProp derived class, which implements the rendering interface between MITK and
  VTK.
  It redirects render() calls to the VtkPropRenderer, which is responsible for rendering of the datatreenodes.
  VtkPropRenderer replaces the old OpenGLRenderer.

  \sa rendering
  \ingroup rendering
  */
  class MITKCORE_EXPORT VtkPropRenderer : public BaseRenderer
  {
    // Workaround for Displaylistbug
  private:
    bool didCount;
    void checkState();

    // Workaround END

  public:
    mitkClassMacro(VtkPropRenderer, BaseRenderer);
    mitkNewMacro3Param(VtkPropRenderer, const char *, vtkRenderWindow *, mitk::RenderingManager *);
    mitkNewMacro4Param(VtkPropRenderer,
                       const char *,
                       vtkRenderWindow *,
                       mitk::RenderingManager *,
                       mitk::BaseRenderer::RenderingMode::Type);

    typedef std::map<int, Mapper *> MappersMapType;

    // Render - called by vtkMitkRenderProp, returns the number of props rendered
    enum RenderType
    {
      Opaque,
      Translucent,
      Overlay,
      Volumetric
    };

    int Render(RenderType type);

    /** \brief This methods contains all method neceassary before a VTK Render() call */
    virtual void PrepareRender();

    // Active current renderwindow
    virtual void MakeCurrent();

    void SetDataStorage(
      mitk::DataStorage *storage) override; ///< set the datastorage that will be used for rendering

    void InitRenderer(vtkRenderWindow *renderwindow) override;
    virtual void Update(mitk::DataNode *datatreenode);
    void SetMapperID(const MapperSlotId mapperId) override;

    // Size
    void InitSize(int w, int h) override;
    void Resize(int w, int h) override;

    // Picking
    enum PickingMode
    {
      WorldPointPicking,
      PointPicking,
      CellPicking
    };
    /** \brief  Set the picking mode.
    This method is used to set the picking mode for 3D object picking. The user can select one of
    the three options WorldPointPicking, PointPicking and CellPicking. The first option uses the zBuffer
    from graphics rendering, the second uses the 3D points from the closest surface mesh, and the third
    option uses the cells   of that mesh. The last option is the slowest, the first one the fastest.
    However, the first option cannot use transparent data object and the tolerance of the picked position
    to the selected point should be considered. PointPicking also need a tolerance around the picking
    position to select the closest point in the mesh. The CellPicker performs very well, if the
    foreground surface part (i.e. the surfacepart that is closest to the scene's cameras) needs to be
    picked. */
    itkSetEnumMacro(PickingMode, PickingMode);
    itkGetEnumMacro(PickingMode, PickingMode);

    void PickWorldPoint(const Point2D &displayPoint, Point3D &worldPoint) const override;
    mitk::DataNode *PickObject(const Point2D &displayPosition, Point3D &worldPosition) const override;

    /**
    * @brief WriteSimpleText Write a text in a renderwindow.
    *
    * Writes some 2D text as overlay. Function returns an unique int Text_ID for each call, which can be used via the
  GetTextLabelProperty(int text_id) function
  in order to get a vtkTextProperty. This property enables the setup of font, font size, etc.
    *
    * @deprecatedSince{2015_05} Please use mitkTextOverlay2D instead.
    * See mitkTextOverlay2DRenderingTest for an example.
    */
    DEPRECATED(int WriteSimpleText(std::string text,
                                   double posX,
                                   double posY,
                                   double color1 = 0.0,
                                   double color2 = 1.0,
                                   double color3 = 0.0,
                                   float opacity = 1.0));

    /**
     * @brief CGetTextLabelProperty an be used in order to get a vtkTextProperty for
     * a specific text_id. This property enables the setup of font, font size, etc.
     * @param text_id the id of the text property.
     * @deprecatedSince{2015_05} Please use mitkTextOverlay2D instead.
     * See mitkTextOverlay2DRenderingTest for an example.
     */
    DEPRECATED(vtkTextProperty *GetTextLabelProperty(int text_id));

    /** This method calculates the bounds of the DataStorage (if it contains any
     * valid data), creates a geometry from these bounds and sets it as world
     * geometry of the renderer.
     *
     * Call this method to re-initialize the renderer to the current DataStorage
     * (e.g. after loading an additional dataset), to ensure that the view is
     * aligned correctly.
     */
    bool SetWorldGeometryToDataStorageBounds() override;

    /**
     * \brief Used by vtkPointPicker/vtkPicker.
     * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
     */
    void InitPathTraversal();

    /**
     * \brief Used by vtkPointPicker/vtkPicker.
     * This will query a list of all objects in MITK and provide every vtk based mapper to the picker.
     */
    vtkAssemblyPath *GetNextPath();

    int GetNumberOfPaths();

    const vtkWorldPointPicker *GetWorldPointPicker() const;
    const vtkPointPicker *GetPointPicker() const;
    const vtkCellPicker *GetCellPicker() const;

    /**
    * \brief Release vtk-based graphics resources. Called by
    * vtkMitkRenderProp::ReleaseGraphicsResources.
    */
    virtual void ReleaseGraphicsResources(vtkWindow *renWin);

    MappersMapType GetMappersMap() const;

    static bool useImmediateModeRendering();

  protected:
    VtkPropRenderer(
      const char *name = "VtkPropRenderer",
      vtkRenderWindow *renWin = nullptr,
      mitk::RenderingManager *rm = nullptr,
      mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard);
    ~VtkPropRenderer() override;
    void Update() override;

    static void RenderingCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);

    virtual void UpdatePaths(); // apply transformations and properties recursively

  private:
    vtkSmartPointer<vtkAssemblyPaths> m_Paths;
    vtkTimeStamp m_PathTime;

    // prepare all mitk::mappers for rendering
    void PrepareMapperQueue();

    /** \brief Set parallel projection, remove the interactor and the lights of VTK. */
    bool Initialize2DvtkCamera();

    bool m_InitNeeded;
    bool m_ResizeNeeded;
    MapperSlotId m_CameraInitializedForMapperID;

    // Picking
    vtkWorldPointPicker *m_WorldPointPicker;
    vtkPointPicker *m_PointPicker;
    vtkCellPicker *m_CellPicker;

    PickingMode m_PickingMode;

    // Explicit use of SmartPointer to avoid circular #includes
    itk::SmartPointer<mitk::Mapper> m_CurrentWorldPlaneGeometryMapper;

    vtkLightKit *m_LightKit;

    // sorted list of mappers
    MappersMapType m_MappersMap;

    // rendering of text
    vtkRenderer *m_TextRenderer;
    typedef std::map<unsigned int, vtkTextActor *> TextMapType;
    TextMapType m_TextCollection;
  };
} // namespace mitk

#endif /* mitkVtkPropRenderer_h */
