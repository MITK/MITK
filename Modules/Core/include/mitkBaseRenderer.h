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

#pragma GCC visibility push(default)
  itkEventMacroDeclaration(RendererResetEvent, itk::AnyEvent);
#pragma GCC visibility pop

  /*
   * \brief Organizes the rendering process
   *
   * A BaseRenderer contains a reference to a given vtkRenderWindow
   * and a corresponding vtkRenderer.
   * The BaseRenderer defines which mapper should be used (2D / 3D)
   * and which view direction should be rendered.
   *
   * All existing BaseRenderer are stored in a static variable
   * that can be accessed / modified via the static functions.
   * VtkPropRenderer is a concrete implementation of a BaseRenderer.
   */
  class MITKCORE_EXPORT BaseRenderer : public itk::Object
  {
  public:

    typedef std::map<vtkRenderWindow*, BaseRenderer*> BaseRendererMapType;
    static BaseRendererMapType baseRendererMap;

    /**
     * \brief Defines which kind of mapper (e.g. 2D or 3D) should be used.
     */
    enum StandardMapperSlot
    {
      Standard2D = 1,
      Standard3D = 2
    };

    static BaseRenderer* GetInstance(vtkRenderWindow* renderWindow);
    static void AddInstance(vtkRenderWindow* renderWindow, BaseRenderer* baseRenderer);
    static void RemoveInstance(vtkRenderWindow* renderWindow);

    static BaseRenderer* GetByName(const std::string& name);
    static vtkRenderWindow* GetRenderWindowByName(const std::string& name);

    /**
     * \brief Get a map of specific RenderWindows
     */
    static BaseRendererMapType GetSpecificRenderWindows(MapperSlotId mapper);

    /**
     * \brief Convenience function: Get a map of all 2D RenderWindows
     */
    static BaseRendererMapType GetAll2DRenderWindows();

    /**
     * \brief Convenience function: Get a map of all 3D RenderWindows
     */
    static BaseRendererMapType GetAll3DRenderWindows();

    mitkClassMacroItkParent(BaseRenderer, itk::Object);

    BaseRenderer(const char* name = nullptr, vtkRenderWindow* renderWindow = nullptr);

    void RemoveAllLocalStorages();
    void RegisterLocalStorageHandler(BaseLocalStorageHandler* lsh);
    void UnregisterLocalStorageHandler(BaseLocalStorageHandler* lsh);

    virtual void SetDataStorage(DataStorage* storage);

    virtual DataStorage::Pointer GetDataStorage() const
    {
      return m_DataStorage.GetPointer();
    }

    vtkRenderWindow* GetRenderWindow() const
    {
      return m_RenderWindow;
    }

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
     */
    virtual void Resize(int w, int h);

    /**
     * \brief Initialize the base renderer with a vtk render window.
     *        Set the new renderer for the camera controller.
     */
    virtual void InitRenderer(vtkRenderWindow* renderwindow);

    /**
     * \brief Set the initial size for the render window.
     */
    virtual void InitSize(int w, int h);

    virtual void DrawOverlayMouse(Point2D&)
    {
      MITK_INFO << "BaseRenderer::DrawOverlayMouse() should be in concret implementation OpenGLRenderer." << std::endl;
    }

    /**
     * \brief Set the world time geometry using the given TimeGeometry.
     *
     * Setting a new world time geometry updates the current world geometry and the
     * curent world plane geometry, using the currently selected slice and timestep.
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

    virtual bool SetWorldGeometryToDataStorageBounds()
    {
      return false;
    }

    /**
     * \brief Set the slice that should be used for geometry extraction.
     *
     * The slice defines the current slice-extracted 2D-geometry (CurrentWorldPlaneGeometry).
     * Setting a new slice will update the current world geometry and the
     * curent world plane geometry.
     */
    virtual void SetSlice(unsigned int slice);

    itkGetConstMacro(Slice, unsigned int);

    /**
     * \brief Set the timestep that should be used for geometry extraction.
     *
     * The timestep defines the current time-extracted 3D-geometry (CurrentWorldGeometry).
     * Setting a new timestep will update the current world geometry and the
     * curent world plane geometry.
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

    virtual int* GetSize() const;
    virtual int* GetViewportSize() const;

    void SetSliceNavigationController(SliceNavigationController* SlicenavigationController);
    itkGetObjectMacro(CameraController, CameraController);
    itkGetObjectMacro(SliceNavigationController, SliceNavigationController);
    itkGetObjectMacro(CameraRotationController, CameraRotationController);
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

    void RequestUpdate();
    void ForceImmediateUpdate();

    /**
     * \brief Return the number of mappers which are visible and have
     *        level-of-detail rendering enabled.
     */
    unsigned int GetNumberOfVisibleLODEnabledMappers() const;

    /**
     * \brief Convert a display point to the 3D world index
     *        using the geometry of the renderWindow.
     */
    void DisplayToWorld(const Point2D& displayPoint, Point3D& worldIndex) const;

    /**
     * \brief Convert a display point to the 2D world index, mapped onto the display plane
     *        using the geometry of the renderWindow.
     */
    void DisplayToPlane(const Point2D& displayPoint, Point2D& planePointInMM) const;

    /**
     * \brief Convert a 3D world index to the display point
     *        using the geometry of the renderWindow.
     */
    void WorldToDisplay(const Point3D& worldIndex, Point2D& displayPoint) const;

    /**
     * \brief Convert a 3D world index to the point on the viewport
     *        using the geometry of the renderWindow.
     */
    void WorldToView(const Point3D& worldIndex, Point2D& viewPoint) const;

    /**
     * \brief Convert a 2D plane coordinate to the display point
     *        using the geometry of the renderWindow.
     */
    void PlaneToDisplay(const Point2D& planePointInMM, Point2D& displayPoint) const;

    /**
     * \brief Convert a 2D plane coordinate to the point on the viewport
     *        using the geometry of the renderWindow.
     */
    void PlaneToView(const Point2D& planePointInMM, Point2D& viewPoint) const;

    double GetScaleFactorMMPerDisplayUnit() const;

    Point2D GetDisplaySizeInMM() const;
    Point2D GetViewportSizeInMM() const;

    Point2D GetOriginInMM() const;

    itkGetConstMacro(ConstrainZoomingAndPanning, bool)
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
