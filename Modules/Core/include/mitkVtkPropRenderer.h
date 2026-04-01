/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkPropRenderer_h
#define mitkVtkPropRenderer_h

#include <mitkBaseRenderer.h>
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

  /**
   * \brief Organizes the MITK rendering process within the VTK rendering pipeline.
   *
   * The MITK rendering process is completely integrated into the VTK rendering pipeline.
   * vtkMitkRenderProp is a custom vtkProp derived class that implements the rendering
   * interface between MITK and VTK. It redirects render() calls to VtkPropRenderer,
   * which is responsible for rendering the data tree nodes.
   *
   * VtkPropRenderer replaces the old OpenGLRenderer.
   *
   * \sa rendering
   * \ingroup rendering
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
    mitkNewMacro2Param(VtkPropRenderer, const char*, vtkRenderWindow*);

    typedef std::map<int, Mapper *> MappersMapType;

    /** \brief Enumeration of render pass types. */
    enum RenderType
    {
      Opaque,
      Translucent,
      Overlay,
      Volumetric
    };

    /**
     * \brief Store and propagate vtkInformation during rendering.
     * \param[in] info The VTK information object to propagate to mappers.
     */
    void SetPropertyKeys(vtkInformation *info);

    /**
     * \brief Render the scene for the given render type.
     *
     * Called by vtkMitkRenderProp during VTK rendering.
     *
     * \param[in] type The render pass type (Opaque, Translucent, Overlay, or Volumetric).
     * \return The number of props rendered.
     */
    int Render(RenderType type);

    /** \brief Perform all preparations necessary before a VTK Render() call. */
    virtual void PrepareRender();

    /** \brief Make the current render window the active OpenGL context. */
    virtual void MakeCurrent();

    /**
     * \brief Set the data storage that will be used for rendering.
     * \param[in] storage The data storage to use.
     */
    void SetDataStorage(mitk::DataStorage *storage) override;

    /**
     * \brief Initialize the renderer with the given VTK render window.
     * \param[in] renderwindow The VTK render window to initialize with.
     */
    void InitRenderer(vtkRenderWindow *renderwindow) override;

    /**
     * \brief Update a specific data node in the rendering pipeline.
     * \param[in] datatreenode The data node to update.
     */
    virtual void Update(mitk::DataNode *datatreenode);

    /**
     * \brief Set the mapper slot ID to select 2D or 3D mappers.
     * \param[in] mapperId The mapper slot identifier.
     */
    void SetMapperID(const MapperSlotId mapperId) override;

    /**
     * \brief Initialize the renderer size.
     * \param[in] w Width in pixels.
     * \param[in] h Height in pixels.
     */
    void InitSize(int w, int h) override;

    /**
     * \brief Resize the renderer.
     * \param[in] w New width in pixels.
     * \param[in] h New height in pixels.
     */
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

    /**
     * \brief Pick a world point from a display position using the current picking mode.
     *
     * \param[in] displayPoint The 2D display position to pick from.
     * \param[out] worldPoint The resulting 3D world coordinate.
     */
    void PickWorldPoint(const Point2D &displayPoint, Point3D &worldPoint) const override;

    /**
     * \brief Pick the data node at the given display position.
     *
     * \param[in] displayPosition The 2D display position to pick from.
     * \param[out] worldPosition The 3D world position of the picked object.
     * \return The picked DataNode, or nullptr if nothing was picked.
     */
    mitk::DataNode *PickObject(const Point2D &displayPosition, Point3D &worldPosition) const override;

    /**
     * \brief Write 2D text as an overlay in the render window.
     *
     * Returns a unique text ID for each call, which can be used via
     * GetTextLabelProperty() to obtain a vtkTextProperty for configuring
     * font, font size, etc.
     *
     * \deprecated Since 2015_05. Use mitkTextOverlay2D instead.
     * \sa mitkTextOverlay2DRenderingTest
     */
    DEPRECATED(int WriteSimpleText(std::string text,
                                   double posX,
                                   double posY,
                                   double color1 = 0.0,
                                   double color2 = 1.0,
                                   double color3 = 0.0,
                                   float opacity = 1.0));

    /**
     * \brief Return the vtkTextProperty for a given text ID.
     *
     * The returned property enables configuration of font, font size, etc.
     *
     * \param[in] text_id The ID of the text property (returned by WriteSimpleText).
     * \deprecated Since 2015_05. Use mitkTextOverlay2D instead.
     * \sa mitkTextOverlay2DRenderingTest
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

    /**
     * \brief Return the number of assembly paths in the current traversal.
     * \return The number of paths.
     */
    int GetNumberOfPaths();

    /**
     * \brief Return the world point picker.
     * \return Pointer to the internal vtkWorldPointPicker.
     */
    const vtkWorldPointPicker *GetWorldPointPicker() const;

    /**
     * \brief Return the point picker.
     * \return Pointer to the internal vtkPointPicker.
     */
    const vtkPointPicker *GetPointPicker() const;

    /**
     * \brief Return the cell picker.
     * \return Pointer to the internal vtkCellPicker.
     */
    const vtkCellPicker *GetCellPicker() const;

    /**
     * \brief Release VTK-based graphics resources.
     *
     * Called by vtkMitkRenderProp::ReleaseGraphicsResources.
     *
     * \param[in] renWin The VTK window whose resources should be released.
     */
    virtual void ReleaseGraphicsResources(vtkWindow *renWin);

    /**
     * \brief Return the current map of mappers sorted by layer.
     * \return The mappers map keyed by layer index.
     */
    MappersMapType GetMappersMap() const;

    /**
     * \brief Return whether immediate mode rendering is used.
     * \return True if immediate mode rendering is enabled.
     */
    static bool useImmediateModeRendering();

  protected:
    VtkPropRenderer(const char *name = "VtkPropRenderer", vtkRenderWindow *renWin = nullptr);
    ~VtkPropRenderer() override;
    void Update() override;

    static void RenderingCallback(vtkObject *caller, unsigned long eid, void *clientdata, void *calldata);

    virtual void UpdatePaths(); // apply transformations and properties recursively

  private:
    vtkSmartPointer<vtkAssemblyPaths> m_Paths;
    vtkTimeStamp m_PathTime;

    // prepare all mitk::mappers for rendering
    void PrepareMapperQueue();

    /** \brief Propagate vtkInformation object to all VTK-based mappers */
    void PropagateRenderInfoToMappers();

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

   /** \brief Information passed from VTK's rendering to props.

       Used e.g. by vtkDualDepthPeelingPass to pass information.
       Not passing this to all the MITK generated vktProps will
       essentially break VTK's depth peeling / transparency.
    */
    vtkInformation* m_VtkRenderInfo = nullptr;
  };
} // namespace mitk

#endif
