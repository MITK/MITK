/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseRenderer_h
#define mitkBaseRenderer_h

#include <mitkDataStorage.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlaneGeometryData.h>
#include <mitkTimeGeometry.h>

#include <mitkCameraController.h>
#include <mitkCameraRotationController.h>
#include <mitkSliceNavigationController.h>
#include <mitkTimeNavigationController.h>

#include <mitkBindDispatcherInteractor.h>
#include <mitkDispatcher.h>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>

#include <map>
#include <set>

namespace mitk
{
  class Mapper;
  class BaseLocalStorageHandler;

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
  itkEventMacroDeclaration(RendererResetEvent, itk::AnyEvent);
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

  /**
   * \brief Organizes the rendering process.
   *
   * A BaseRenderer contains a reference to a given vtkRenderWindow
   * and a corresponding vtkRenderer. It manages geometry extraction
   * (world time geometry, current world geometry, current world plane geometry),
   * coordinate conversions between display and world space,
   * and the mapper type (2D / 3D) used for rendering.
   *
   * All existing BaseRenderer instances are stored in a static map
   * that can be accessed / modified via the static functions.
   * VtkPropRenderer is the concrete implementation of a BaseRenderer.
   *
   * \sa VtkPropRenderer
   * \sa RenderingManager
   * \sa SliceNavigationController
   * \sa Mapper
   * \ingroup Rendering
   */
  class MITKCORE_EXPORT BaseRenderer : public itk::Object
  {
  public:

    /** \brief Map type associating vtkRenderWindow pointers with BaseRenderer pointers. */
    typedef std::map<vtkRenderWindow*, BaseRenderer*> BaseRendererMapType;

    /** \brief Static map holding all registered BaseRenderer instances, keyed by their vtkRenderWindow. */
    static BaseRendererMapType baseRendererMap;

    /**
     * \brief Defines which kind of mapper (e.g. 2D or 3D) should be used.
     */
    enum StandardMapperSlot
    {
      Standard2D = 1, ///< Use 2D mappers for rendering.
      Standard3D = 2  ///< Use 3D mappers for rendering.
    };

    /**
     * \brief Get the BaseRenderer instance associated with the given vtkRenderWindow.
     * \param[in] renderWindow The vtkRenderWindow to look up.
     * \return Pointer to the associated BaseRenderer, or nullptr if not found.
     */
    static BaseRenderer* GetInstance(vtkRenderWindow* renderWindow);

    /**
     * \brief Register a BaseRenderer instance for the given vtkRenderWindow.
     *
     * If a BaseRenderer is already registered for this render window, it is removed first.
     *
     * \param[in] renderWindow The vtkRenderWindow to associate with the renderer.
     * \param[in] baseRenderer The BaseRenderer instance to register.
     */
    static void AddInstance(vtkRenderWindow* renderWindow, BaseRenderer* baseRenderer);

    /**
     * \brief Remove the BaseRenderer registration for the given vtkRenderWindow.
     * \param[in] renderWindow The vtkRenderWindow whose registration should be removed.
     */
    static void RemoveInstance(vtkRenderWindow* renderWindow);

    /**
     * \brief Find a BaseRenderer by its name.
     * \param[in] name The name of the renderer to search for.
     * \return Pointer to the matching BaseRenderer, or nullptr if not found.
     */
    static BaseRenderer* GetByName(const std::string& name);

    /**
     * \brief Find a vtkRenderWindow by the name of its associated BaseRenderer.
     * \param[in] name The name of the renderer whose render window is requested.
     * \return Pointer to the matching vtkRenderWindow, or nullptr if not found.
     */
    static vtkRenderWindow* GetRenderWindowByName(const std::string& name);

    /**
     * \brief Get a map of all BaseRenderers that use the specified mapper type.
     * \param[in] mapper The MapperSlotId (Standard2D or Standard3D) to filter by.
     * \return A map of matching vtkRenderWindow-to-BaseRenderer pairs.
     */
    static BaseRendererMapType GetSpecificRenderWindows(MapperSlotId mapper);

    /**
     * \brief Convenience function: Get a map of all 2D RenderWindows.
     * \return A map of all BaseRenderers using Standard2D mappers.
     */
    static BaseRendererMapType GetAll2DRenderWindows();

    /**
     * \brief Convenience function: Get a map of all 3D RenderWindows.
     * \return A map of all BaseRenderers using Standard3D mappers.
     */
    static BaseRendererMapType GetAll3DRenderWindows();

    mitkClassMacroItkParent(BaseRenderer, itk::Object);

    /**
     * \brief Construct a BaseRenderer with an optional name and vtkRenderWindow.
     * \param[in] name The name of the renderer. If nullptr, an unnamed renderer is created.
     * \param[in] renderWindow The vtkRenderWindow to associate with this renderer. Can be nullptr.
     */
    BaseRenderer(const char* name = nullptr, vtkRenderWindow* renderWindow = nullptr);

    /**
     * \brief Remove all registered local storages and notify observers via RendererResetEvent.
     */
    void RemoveAllLocalStorages();

    /**
     * \brief Register a local storage handler with this renderer.
     * \param[in] lsh The local storage handler to register.
     */
    void RegisterLocalStorageHandler(BaseLocalStorageHandler* lsh);

    /**
     * \brief Unregister a previously registered local storage handler.
     * \param[in] lsh The local storage handler to unregister.
     */
    void UnregisterLocalStorageHandler(BaseLocalStorageHandler* lsh);

    /**
     * \brief Set the DataStorage used by this renderer.
     *
     * The DataStorage provides the data nodes whose contents are rendered.
     * Also updates the dispatcher's data storage reference.
     *
     * \param[in] storage The DataStorage to use. If nullptr, no change is made.
     */
    virtual void SetDataStorage(DataStorage* storage);

    /**
     * \brief Get the DataStorage used by this renderer.
     * \return Pointer to the current DataStorage.
     */
    virtual DataStorage::Pointer GetDataStorage() const
    {
      return m_DataStorage.GetPointer();
    }

    /**
     * \brief Get the vtkRenderWindow associated with this renderer.
     * \return Pointer to the vtkRenderWindow.
     */
    vtkRenderWindow* GetRenderWindow() const
    {
      return m_RenderWindow;
    }

    /**
     * \brief Get the underlying vtkRenderer used for VTK rendering.
     * \return Pointer to the vtkRenderer.
     */
    vtkRenderer* GetVtkRenderer() const
    {
      return m_VtkRenderer;
    }

    /**
     * \brief Get the dispatcher, which handles events for this base renderer.
     */
    Dispatcher::Pointer GetDispatcher() const;

    /**
     * \brief Set a new size for the render window.
     * \param[in] w Width in pixels.
     * \param[in] h Height in pixels.
     */
    virtual void Resize(int w, int h);

    /**
     * \brief Initialize the base renderer with a vtk render window.
     *
     * Replaces the current render window, removes all local storages,
     * and re-initializes the camera controller.
     *
     * \param[in] renderwindow The new vtkRenderWindow to use.
     */
    virtual void InitRenderer(vtkRenderWindow* renderwindow);

    /**
     * \brief Set the initial size for the render window.
     * \param[in] w Width in pixels.
     * \param[in] h Height in pixels.
     */
    virtual void InitSize(int w, int h);

    /**
     * \brief Draw an overlay mouse cursor at the given position.
     *
     * \note This is a stub; the actual implementation must be provided in concrete subclasses.
     */
    virtual void DrawOverlayMouse(Point2D&)
    {
      MITK_INFO << "BaseRenderer::DrawOverlayMouse() should be in concret implementation OpenGLRenderer." << std::endl;
    }

    /**
     * \brief Set the world time geometry using the given TimeGeometry.
     *
     * Setting a new world time geometry updates the current world geometry and the
     * current world plane geometry, using the currently selected slice and timestep.
     */
    virtual void SetWorldTimeGeometry(const TimeGeometry* geometry);
    itkGetConstObjectMacro(WorldTimeGeometry, TimeGeometry);

    /**
     * \brief Set the interaction reference world time geometry using the given TimeGeometry.
     *
     * Setting a new interaction reference world time geometry also updates the
     * alignment status of the reference geometry, which can be retrieved using
     * 'GetReferenceGeometryAligned'.
     * Using a nullptr as the interaction reference geomertry implies that
     * no requirements on the geometry exist, thus in this case any check
     * will result in 'ReferenceGeometryAligned' being true.
     *
     * \param geometry  The reference geometry used for render window interaction.
     */
    virtual void SetInteractionReferenceGeometry(const TimeGeometry* geometry);

    /**
     * \brief Get the current interaction reference geometry.
     */
    itkGetConstObjectMacro(InteractionReferenceGeometry, TimeGeometry);

    /**
     * \brief Return if the reference geometry aligns with the base renderer's world geometry.
     *        If true, the interaction reference geometry aligns with the base renderer's
     *        current world geometry. False otherwise.
     */
    itkGetMacro(ReferenceGeometryAligned, bool);

    /**
     * \brief Get the current time-extracted 3D-geometry.
     */
    itkGetConstObjectMacro(CurrentWorldGeometry, BaseGeometry);

    /**
     * \brief Get the current slice-extracted 2D-geometry.
     */
    itkGetConstObjectMacro(CurrentWorldPlaneGeometry, PlaneGeometry);

    /**
     * \brief Set the world geometry so that it encompasses all objects in the DataStorage.
     * \return true if the geometry was successfully updated, false otherwise.
     * \note Default implementation returns false; subclasses may override.
     */
    virtual bool SetWorldGeometryToDataStorageBounds()
    {
      return false;
    }

    /**
     * \brief Set the slice that should be used for geometry extraction.
     *
     * The slice defines the current slice-extracted 2D-geometry (CurrentWorldPlaneGeometry).
     * Setting a new slice will update the current world geometry and the
     * current world plane geometry.
     */
    virtual void SetSlice(unsigned int slice);

    itkGetConstMacro(Slice, unsigned int);

    /**
     * \brief Set the timestep that should be used for geometry extraction.
     *
     * The timestep defines the current time-extracted 3D-geometry (CurrentWorldGeometry).
     * Setting a new timestep will update the current world geometry and the
     * current world plane geometry.
     */
    virtual void SetTimeStep(unsigned int timeStep);

    itkGetConstMacro(TimeStep, unsigned int);

    /**
     * \brief Get the timestep of a BaseData object which
     *        exists at the time of the currently displayed content.
     *
     * Returns -1 if there is no data at the current time.
     */
    TimeStepType GetTimeStep(const BaseData* data) const;

    /**
     * \brief Get the time in ms of the currently display content (geometry).
     */
    ScalarType GetTime() const;

    /**
     * \brief Set the world time geometry using the geometry of the given event.
     *
     * The function is triggered by a SliceNavigationController::GeometrySendEvent.
     */
    virtual void SetGeometry(const itk::EventObject& geometrySliceEvent);

    /**
     * \brief Set the current world plane geometry using the existing current world geometry.
     *
     * The function is triggered by a SliceNavigationController::GeometryUpdateEvent.
     */
    virtual void UpdateGeometry(const itk::EventObject& geometrySliceEvent);

    /**
     * \brief Set the current slice using "SetSlice" and update the current world geometry
     *        and the current world plane geometry.
     *
     * The function is triggered by a SliceNavigationController::GeometrySliceEvent.
     */
    virtual void SetGeometrySlice(const itk::EventObject& geometrySliceEvent);

    /**
     * \brief Set the current time using "SetTimeStep" and update the current world geometry
     *        and the current world plane geometry.
     *
     * The function is triggered by a TimeNavigationController::TimeEvent.
     */
    virtual void SetGeometryTime(const itk::EventObject& geometryTimeEvent);

    itkGetObjectMacro(CurrentWorldPlaneGeometryNode, DataNode);

    /**
     * \brief Modify the update time of the current world plane geometry and force reslicing.
     */
    void SendUpdateSlice();

    /**
     * \brief Get timestamp of the update time of the current world plane geometry.
     */
    itkGetMacro(CurrentWorldPlaneGeometryUpdateTime, unsigned long);

    /**
     * \brief Get timestamp of the update time of the current timestep.
     */
    itkGetMacro(TimeStepUpdateTime, unsigned long);

    /**
     * \brief Pick a world coordinate (x,y,z) given a display coordinate (x,y).
     *
     * \warning Not implemented; has to be overwritten in subclasses.
     */
    virtual void PickWorldPoint(const Point2D& diplayPosition, Point3D& worldPosition) const = 0;

    /**
     * \brief Determines the object (mitk::DataNode) closest to the current
     *        position by means of picking.
     *
     * \warning Implementation currently empty for 2D rendering; intended to be
     *          implemented for 3D renderers.
     */
    virtual DataNode* PickObject(const Point2D& /*displayPosition*/, Point3D& /*worldPosition*/) const
    {
      return nullptr;
    }

    /**
     * \brief Get the currently used mapperID.
     */
    itkGetMacro(MapperID, MapperSlotId);
    itkGetConstMacro(MapperID, MapperSlotId);

    /**
     * \brief Set the used mapperID.
     */
    virtual void SetMapperID(MapperSlotId id);

    /**
     * \brief Get the size of the render window in pixels.
     * \return Pointer to an array of two integers [width, height].
     */
    virtual int* GetSize() const;

    /**
     * \brief Get the size of the VTK viewport in pixels.
     * \return Pointer to an array of two integers [width, height].
     * \note This may differ from GetSize() when multiple viewports share a render window.
     */
    virtual int* GetViewportSize() const;

    /**
     * \brief Replace the current SliceNavigationController.
     *
     * Copies the world geometry from the new controller, connects geometry events,
     * and sets the renderer reference on the new controller.
     *
     * \param[in] SlicenavigationController The new SliceNavigationController. Ignored if nullptr.
     */
    void SetSliceNavigationController(SliceNavigationController* SlicenavigationController);

    /** \brief Get the CameraController for this renderer. */
    itkGetObjectMacro(CameraController, CameraController);

    /** \brief Get the SliceNavigationController for this renderer. */
    itkGetObjectMacro(SliceNavigationController, SliceNavigationController);

    /** \brief Get the CameraRotationController for this renderer. */
    itkGetObjectMacro(CameraRotationController, CameraRotationController);

    /** \brief Return whether the current world geometry is empty (has zero extent). */
    itkGetMacro(EmptyWorldGeometry, bool);

    /**
     * \brief Getter/Setter for defining if the displayed region should be shifted
     *        or rescaled if the render window is resized.
     */
    itkGetMacro(KeepDisplayedRegion, bool);
    itkSetMacro(KeepDisplayedRegion, bool);

    /**
     * \brief Return the name of the base renderer
     */
    const char* GetName() const
    {
      return m_Name.c_str();
    }

    /**
     * \brief Return the size in x-direction of the base renderer.
     */
    int GetSizeX() const
    {
      return this->GetSize()[0];
    }

    /**
     * \brief Return the size in y-direction of the base renderer.
     */
    int GetSizeY() const
    {
      return this->GetSize()[1];
    }

    /**
     * \brief Return the bounds of the bounding box of the
     *        current world geometry (time-extracted 3D-geometry).
     *
     * If the geometry is empty, the bounds are set to zero.
     */
    const double* GetBounds() const;

    /**
     * \brief Request an asynchronous rendering update for this render window via the RenderingManager.
     *
     * Also constrains zooming and panning before requesting the update.
     */
    void RequestUpdate();

    /**
     * \brief Force an immediate synchronous rendering update for this render window.
     */
    void ForceImmediateUpdate();

    /**
     * \brief Return the number of mappers which are visible and have
     *        level-of-detail rendering enabled.
     */
    unsigned int GetNumberOfVisibleLODEnabledMappers() const;

    /**
     * \brief Convert a display point to a 3D world coordinate.
     *
     * For 2D renderers, the z-depth is obtained from the camera focal point.
     * For 3D renderers, PickWorldPoint() is used.
     *
     * \param[in] displayPoint 2D display coordinate in pixels.
     * \param[out] worldIndex The resulting 3D world coordinate.
     */
    void DisplayToWorld(const Point2D& displayPoint, Point3D& worldIndex) const;

    /**
     * \brief Convert a display point to a 2D plane coordinate in millimeters.
     *
     * Only meaningful for 2D renderers. Logs a warning for 3D renderers.
     *
     * \param[in] displayPoint 2D display coordinate in pixels.
     * \param[out] planePointInMM The resulting 2D coordinate on the current world plane in mm.
     */
    void DisplayToPlane(const Point2D& displayPoint, Point2D& planePointInMM) const;

    /**
     * \brief Convert a 3D world coordinate to a 2D display point.
     * \param[in] worldIndex 3D world coordinate.
     * \param[out] displayPoint The resulting 2D display coordinate in pixels.
     */
    void WorldToDisplay(const Point3D& worldIndex, Point2D& displayPoint) const;

    /**
     * \brief Convert a 3D world coordinate to a 2D viewport point.
     *
     * The viewport point is in pixels relative to the VTK viewport origin.
     *
     * \param[in] worldIndex 3D world coordinate.
     * \param[out] viewPoint The resulting 2D viewport coordinate in pixels.
     */
    void WorldToView(const Point3D& worldIndex, Point2D& viewPoint) const;

    /**
     * \brief Convert a 2D plane coordinate in millimeters to a 2D display point.
     * \param[in] planePointInMM 2D coordinate on the current world plane in mm.
     * \param[out] displayPoint The resulting 2D display coordinate in pixels.
     */
    void PlaneToDisplay(const Point2D& planePointInMM, Point2D& displayPoint) const;

    /**
     * \brief Convert a 2D plane coordinate in millimeters to a 2D viewport point.
     * \param[in] planePointInMM 2D coordinate on the current world plane in mm.
     * \param[out] viewPoint The resulting 2D viewport coordinate in pixels.
     */
    void PlaneToView(const Point2D& planePointInMM, Point2D& viewPoint) const;

    /**
     * \brief Get the scale factor relating millimeters to display pixels.
     *
     * For 2D renderers this is computed from the camera's parallel scale and
     * the viewport height. For 3D renderers, returns 1.0.
     *
     * \return The number of millimeters per display pixel.
     */
    double GetScaleFactorMMPerDisplayUnit() const;

    /**
     * \brief Get the size of the render window in millimeters.
     * \return A Point2D with (width_mm, height_mm).
     */
    Point2D GetDisplaySizeInMM() const;

    /**
     * \brief Get the size of the VTK viewport in millimeters.
     * \return A Point2D with (width_mm, height_mm).
     */
    Point2D GetViewportSizeInMM() const;

    /**
     * \brief Get the origin of the viewport in plane coordinates (millimeters).
     * \return A Point2D representing the top-left corner of the render window in plane mm.
     */
    Point2D GetOriginInMM() const;

    /** \brief Get whether zooming and panning are constrained to the displayed geometry. */
    itkGetConstMacro(ConstrainZoomingAndPanning, bool)

    /**
     * \brief Enable or disable constraining of zooming and panning.
     *
     * When enabled, the camera controller is adjusted to fit the plane.
     *
     * \param[in] constrain True to constrain zooming and panning, false to allow free navigation.
     */
    virtual void SetConstrainZoomingAndPanning(bool constrain);

  protected:

    ~BaseRenderer() override;

    virtual void Update() = 0;

    vtkRenderWindow* m_RenderWindow;
    vtkRenderer* m_VtkRenderer;

    MapperSlotId m_MapperID;
    DataStorage::Pointer m_DataStorage;
    unsigned long m_LastUpdateTime;

    CameraController::Pointer m_CameraController;
    CameraRotationController::Pointer m_CameraRotationController;
    SliceNavigationController::Pointer m_SliceNavigationController;

    void UpdateCurrentGeometries();
    virtual void SetCurrentWorldPlaneGeometry(const PlaneGeometry* geometry2d);
    virtual void SetCurrentWorldGeometry(const BaseGeometry *geometry);

  private:

    /**
     * \brief Pointer to the current TimeGeometry.
     *
     * This WorldTimeGeometry is used to extract a SlicedGeometry3D,
     * using the current timestep (set via SetTimeStep).
     * The time-extracted 3D-geometry is used as the "CurrentWorldGeometry".
     * A PlaneGeometry can further be extracted using the current slice (set via SetSlice).
     * The slice-extracted 2D-geometry is used as the "CurrentWorldPlaneGeometry".
     */
    TimeGeometry::ConstPointer m_WorldTimeGeometry;

    /**
     * \brief Pointer to the interaction reference geometry used for interaction.
     *
     * This InteractionReferenceGeometry is used to decide if a base renderer /
     * render window is able to correctly handle display interaction, e.g. drawing.
     * It will be set using the "SetInteractionReferenceGeometry"-function.
     */
    TimeGeometry::ConstPointer m_InteractionReferenceGeometry;

    /**
     * \brief Pointer to the current time-extracted 3D-geometry.
     *
     * This CurrentWorldGeometry is used to define the bounds for this
     * BaseRenderer.
     * It will be set using the "SetCurrentWorldGeometry"-function.
     */
    BaseGeometry::ConstPointer m_CurrentWorldGeometry;

    /**
     * \brief Pointer to the current slice-extracted 2D-geometry.
     *
     * This CurrentWorldPlaneGeometry is used to define the maximal
     * area (2D manifold) to be rendered in case we are doing 2D-rendering.
     * It will be set using the "SetCurrentWorldPlaneGeometry"-function.
     */
    PlaneGeometry::Pointer m_CurrentWorldPlaneGeometry;

    unsigned int m_Slice;
    unsigned int m_TimeStep;

    itk::TimeStamp m_CurrentWorldPlaneGeometryUpdateTime;
    itk::TimeStamp m_TimeStepUpdateTime;

    BindDispatcherInteractor* m_BindDispatcherInteractor;

    bool m_KeepDisplayedRegion;
    bool m_ReferenceGeometryAligned;

  protected:

    void PrintSelf(std::ostream& os, itk::Indent indent) const override;

    PlaneGeometryData::Pointer m_CurrentWorldPlaneGeometryData;
    DataNode::Pointer m_CurrentWorldPlaneGeometryNode;
    unsigned long m_CurrentWorldPlaneGeometryTransformTime;

    std::string m_Name;

    double m_Bounds[6];

    bool m_EmptyWorldGeometry;

    typedef std::set<Mapper*> LODEnabledMappersType;

    unsigned int m_NumberOfVisibleLODEnabledMappers;

    std::list<BaseLocalStorageHandler*> m_RegisteredLocalStorageHandlers;

    bool m_ConstrainZoomingAndPanning;
  };
} // namespace mitk

#endif
