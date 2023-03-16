/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSliceNavigationController_h
#define mitkSliceNavigationController_h

#include <MitkCoreExports.h>

#include <mitkBaseController.h>
#include <mitkMessage.h>
#include <mitkAnatomicalPlanes.h>
#include <mitkRenderingManager.h>
#include <mitkRestorePlanePositionOperation.h>
#include <mitkTimeGeometry.h>

#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop

#include <itkCommand.h>

namespace mitk
{
#define mitkTimeGeometryEventMacro(classname, super)                                                                   \
  class MITKCORE_EXPORT classname : public super                                                                       \
  {                                                                                                                    \
  public:                                                                                                              \
    typedef classname Self;                                                                                            \
    typedef super Superclass;                                                                                          \
    classname(TimeGeometry *aTimeGeometry, unsigned int aPos) : Superclass(aTimeGeometry, aPos) {}                     \
    virtual ~classname() {}                                                                                            \
    virtual const char *GetEventName() const { return #classname; }                                                    \
    virtual bool CheckEvent(const ::itk::EventObject *e) const { return dynamic_cast<const Self *>(e); }               \
    virtual ::itk::EventObject *MakeObject() const { return new Self(GetTimeGeometry(), GetPos()); }                   \
  private:                                                                                                             \
    void operator=(const Self &);                                                                                      \
  }

  class PlaneGeometry;
  class BaseGeometry;
  class BaseRenderer;

  /**
   * \brief Controls the selection of the slice the associated BaseRenderer
   * will display
   *
   * A SliceNavigationController takes a BaseGeometry or a TimeGeometry as input world geometry
   * (TODO what are the exact requirements?) and generates a TimeGeometry
   * as output. The TimeGeometry holds a number of SlicedGeometry3Ds and
   * these in turn hold a series of PlaneGeometries. One of these PlaneGeometries is
   * selected as world geometry for the BaseRenderers associated to 2D views.
   *
   * The SliceNavigationController holds has Steppers (one for the slice, a
   * second for the time step), which control the selection of a single
   * PlaneGeometry from the TimeGeometry. SliceNavigationController generates
   * ITK events to tell observers, like a BaseRenderer, when the selected slice
   * or timestep changes.
   *
   * Example:
   * \code
   * // Initialization
   * sliceCtrl = mitk::SliceNavigationController::New();
   *
   * // Tell the navigation controller the geometry to be sliced
   * // (with geometry a BaseGeometry::ConstPointer)
   * sliceCtrl->SetInputWorldTimeGeometry(geometry.GetPointer());
   *
   * // Tell the navigation controller in which direction it shall slice the data
   * sliceCtrl->SetViewDirection(mitk::AnatomicalPlane::Axial);
   *
   * // Connect one or more BaseRenderer to this navigation controller, i.e.:
   * // events sent by the navigation controller when stepping through the slices
   * // (e.g. by sliceCtrl->GetSlice()->Next()) will be received by the BaseRenderer
   * // (in this example only slice-changes, see also ConnectGeometryTimeEvent
   * // and ConnectGeometryEvents.)
   * sliceCtrl->ConnectGeometrySliceEvent(renderer.GetPointer());
   *
   * //create a world geometry and send the information to the connected renderer(s)
   * sliceCtrl->Update();
   * \endcode
   *
   *
   * You can connect visible navigation widgets to a SliceNavigationController, e.g., a
   * QmitkSliceNavigationWidget (for Qt):
   *
   * \code
   * // Create the visible navigation widget (a slider with a spin-box)
   * QmitkSliceNavigationWidget* navigationWidget =
   *   new QmitkSliceNavigationWidget(parent);
   *
   * // Connect the navigation widget to the slice-stepper of the
   * // SliceNavigationController. For initialization (position, mininal and
   * // maximal values) the values of the SliceNavigationController are used.
   * // Thus, accessing methods of a navigation widget is normally not necessary,
   * // since everything can be set via the (Qt-independent) SliceNavigationController.
   * // The QmitkStepperAdapter converts the Qt-signals to Qt-independent
   * // itk-events.
   * new QmitkStepperAdapter(navigationWidget, sliceCtrl->GetSlice());
   * \endcode
   *
   * If you do not want that all renderwindows are updated when a new slice is
   * selected, you can use a specific RenderingManager, which updates only those
   * renderwindows that should be updated. This is sometimes useful when a 3D view
   * does not need to be updated when the slices in some 2D views are changed.
   *
   * \code
   * // create a specific RenderingManager
   * mitk::RenderingManager::Pointer myManager = mitk::RenderingManager::New();
   *
   * // tell the RenderingManager to update only renderwindow1 and renderwindow2
   * myManager->AddRenderWindow(renderwindow1);
   * myManager->AddRenderWindow(renderwindow2);
   *
   * // tell the SliceNavigationController of renderwindow1 and renderwindow2
   * // to use the specific RenderingManager instead of the global one
   * renderwindow1->GetSliceNavigationController()->SetRenderingManager(myManager);
   * renderwindow2->GetSliceNavigationController()->SetRenderingManager(myManager);
   * \endcode
   *
   * \todo implement for non-evenly-timed geometry!
   * \ingroup NavigationControl
   */
  class MITKCORE_EXPORT SliceNavigationController : public BaseController
  {
  public:

    mitkClassMacro(SliceNavigationController, BaseController);
    itkNewMacro(Self);

    /**
     * \brief Set the input world time geometry out of which the
     * geometries for slicing will be created.
     *
     * Any previous previous set input geometry (3D or Time) will
     * be ignored in future.
     */
    void SetInputWorldTimeGeometry(const TimeGeometry* geometry);
    itkGetConstObjectMacro(InputWorldTimeGeometry, TimeGeometry);

    /**
     * \brief Access the created geometry
     */
    itkGetConstObjectMacro(CreatedWorldGeometry, TimeGeometry);
    itkGetObjectMacro(CreatedWorldGeometry, TimeGeometry);

    /**
     * \brief Set the desired view directions
     *
     * \sa ViewDirection
     * \sa Update(AnatomicalPlane viewDirection, bool top = true,
     *     bool frontside = true, bool rotated = false)
     */
    itkSetEnumMacro(ViewDirection, AnatomicalPlane);
    itkGetEnumMacro(ViewDirection, AnatomicalPlane);

    /**
     * \brief Set the default view direction
     *
     * This is used to re-initialize the view direction of the SNC to the
     * default value with SetViewDirectionToDefault()
     *
     * \sa ViewDirection
     * \sa Update(AnatomicalPlane viewDirection, bool top = true,
     *     bool frontside = true, bool rotated = false)
     */
    itkSetEnumMacro(DefaultViewDirection, AnatomicalPlane);
    itkGetEnumMacro(DefaultViewDirection, AnatomicalPlane);

    const char *GetViewDirectionAsString() const;

    virtual void SetViewDirectionToDefault();

    /**
     * \brief Do the actual creation and send it to the connected
     * observers (renderers)
     *
     */
    virtual void Update();

    /**
     * \brief Extended version of Update, additionally allowing to
     * specify the direction/orientation of the created geometry.
     *
     */
    virtual void Update(AnatomicalPlane viewDirection, bool top = true, bool frontside = true, bool rotated = false);

    /**
     * \brief Send the created geometry to the connected
     * observers (renderers)
     *
     * Called by Update().
     */
    virtual void SendCreatedWorldGeometry();

    /**
     * \brief Tell observers to re-read the currently selected 2D geometry
     *
     */
    virtual void SendCreatedWorldGeometryUpdate();

    /**
     * \brief Send the currently selected slice to the connected
     * observers (renderers)
     *
     * Called by Update().
     */
    virtual void SendSlice();

    /**
     * \brief Send the currently selected time to the connected
     * observers (renderers)
     *
     * Called by Update().
     */
    virtual void SendTime();

    class MITKCORE_EXPORT TimeGeometryEvent : public itk::AnyEvent
    {
    public:
      typedef TimeGeometryEvent Self;
      typedef itk::AnyEvent Superclass;

      TimeGeometryEvent(TimeGeometry* aTimeGeometry, unsigned int aPos) : m_TimeGeometry(aTimeGeometry), m_Pos(aPos) {}
      ~TimeGeometryEvent() override {}
      const char* GetEventName() const override { return "TimeGeometryEvent"; }
      bool CheckEvent(const ::itk::EventObject* e) const override { return dynamic_cast<const Self*>(e); }
      ::itk::EventObject* MakeObject() const override { return new Self(m_TimeGeometry, m_Pos); }
      TimeGeometry* GetTimeGeometry() const { return m_TimeGeometry; }
      unsigned int GetPos() const { return m_Pos; }

    private:
      TimeGeometry::Pointer m_TimeGeometry;
      unsigned int m_Pos;
      // TimeGeometryEvent(const Self&);
      void operator=(const Self&); // just hide
    };


    mitkTimeGeometryEventMacro(GeometrySendEvent, TimeGeometryEvent);
    mitkTimeGeometryEventMacro(GeometryUpdateEvent, TimeGeometryEvent);
    mitkTimeGeometryEventMacro(GeometryTimeEvent, TimeGeometryEvent);
    mitkTimeGeometryEventMacro(GeometrySliceEvent, TimeGeometryEvent);

    template <typename T>
    void ConnectGeometrySendEvent(T* receiver)
    {
      auto eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometry);
      unsigned long tag = AddObserver(GeometrySendEvent(nullptr, 0), eventReceptorCommand);
      m_ReceiverToObserverTagsMap[static_cast<void*>(receiver)].push_back(tag);
    }

    template <typename T>
    void ConnectGeometryUpdateEvent(T* receiver)
    {
      auto eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::UpdateGeometry);
      unsigned long tag = AddObserver(GeometryUpdateEvent(nullptr, 0), eventReceptorCommand);
      m_ReceiverToObserverTagsMap[static_cast<void*>(receiver)].push_back(tag);
    }

    template <typename T>
    void ConnectGeometrySliceEvent(T* receiver)
    {
      auto eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometrySlice);
      unsigned long tag = AddObserver(GeometrySliceEvent(nullptr, 0), eventReceptorCommand);
      m_ReceiverToObserverTagsMap[static_cast<void*>(receiver)].push_back(tag);
    }

    template <typename T>
    void ConnectGeometryTimeEvent(T* receiver)
    {
      auto eventReceptorCommand = itk::ReceptorMemberCommand<T>::New();
      eventReceptorCommand->SetCallbackFunction(receiver, &T::SetGeometryTime);
      unsigned long tag = AddObserver(GeometryTimeEvent(nullptr, 0), eventReceptorCommand);
      m_ReceiverToObserverTagsMap[static_cast<void*>(receiver)].push_back(tag);
    }

    template <typename T>
    void ConnectGeometryEvents(T* receiver)
    {
      // connect sendEvent only once
      ConnectGeometrySliceEvent(receiver, false);
      ConnectGeometryTimeEvent(receiver);
    }

    // use a templated method to get the right offset when casting to void*
    template <typename T>
    void Disconnect(T* receiver)
    {
      auto i = m_ReceiverToObserverTagsMap.find(static_cast<void*>(receiver));
      if (i == m_ReceiverToObserverTagsMap.end())
        return;
      const std::list<unsigned long>& tags = i->second;
      for (auto tagIter = tags.begin(); tagIter != tags.end(); ++tagIter)
      {
        RemoveObserver(*tagIter);
      }
      m_ReceiverToObserverTagsMap.erase(i);
    }

    Message1<const Point3D&> SetCrosshairEvent;

    /**
     * \brief To connect multiple SliceNavigationController, we can
     * act as an observer ourselves: implemented interface
     * \warning not implemented
     */
    virtual void SetGeometry(const itk::EventObject& geometrySliceEvent);

    /**
     * \brief To connect multiple SliceNavigationController, we can
     * act as an observer ourselves: implemented interface
     */
    virtual void SetGeometrySlice(const itk::EventObject& geometrySliceEvent);

    /**
     * \brief To connect multiple SliceNavigationController, we can
     * act as an observer ourselves: implemented interface
     */
    virtual void SetGeometryTime(const itk::EventObject& geometryTimeEvent);

    /** \brief Positions the SNC according to the specified point */
    void SelectSliceByPoint(const Point3D& point);

    /** \brief Returns the BaseGeometry of the currently selected time step. */
    const BaseGeometry* GetCurrentGeometry3D();

    /** \brief Returns the currently selected Plane in the current
     * BaseGeometry (if existent).
     */
    const PlaneGeometry* GetCurrentPlaneGeometry();

    /** \brief Sets / gets the BaseRenderer associated with this SNC (if any).
     * While the BaseRenderer is not directly used by SNC, this is a convenience
     * method to enable BaseRenderer access via the SNC.
     */
    itkSetObjectMacro(Renderer, BaseRenderer);
    itkGetMacro(Renderer, BaseRenderer*);

    /** \brief Re-orients the slice stack. All slices will be oriented to the given normal vector.
         The given point (world coordinates) defines the selected slice.
         Careful: The resulting axis vectors are not clearly defined this way. If you want to define them clearly, use
         ReorientSlices (const Point3D &point, const Vector3D &axisVec0, const Vector3D &axisVec1).
     */
    void ReorientSlices(const Point3D& point, const Vector3D& normal);

    /** \brief Re-orients the slice stack so that all planes are oriented according to the
     * given axis vectors. The given Point eventually defines selected slice.
     */
    void ReorientSlices(const Point3D& point, const Vector3D& axisVec0, const Vector3D& axisVec1);

    void ExecuteOperation(Operation* operation) override;

    /**
     * \brief Feature option to lock planes during mouse interaction.
     * This option flag disables the mouse event which causes the center
     * cross to move near by.
     */
    itkSetMacro(SliceLocked, bool);
    itkGetMacro(SliceLocked, bool);
    itkBooleanMacro(SliceLocked);

    /**
     * \brief Feature option to lock slice rotation.
     *
     * This option flag disables separately the rotation of a slice which is
     * implemented in mitkSliceRotator.
     */
    itkSetMacro(SliceRotationLocked, bool);
    itkGetMacro(SliceRotationLocked, bool);
    itkBooleanMacro(SliceRotationLocked);

    /**
     * \brief Adjusts the numerical range of the slice stepper according to
     * the current geometry orientation of this SNC's SlicedGeometry.
     */
    void AdjustSliceStepperRange();

    /** \brief Convenience method that returns the time step currently selected by the controller.*/
    TimeStepType GetSelectedTimeStep() const;

    /** \brief Convenience method that returns the time point that corresponds to the selected
     * time step. The conversion is done using the time geometry of the SliceNavigationController.
     * If the time geometry is not yet set, this function will always return 0.0.*/
    TimePointType GetSelectedTimePoint() const;

  protected:

    SliceNavigationController();
    ~SliceNavigationController() override;

    void CreateWorldGeometry(bool top, bool frontside, bool rotated);

    TimeGeometry::ConstPointer m_InputWorldTimeGeometry;
    TimeGeometry::Pointer m_CreatedWorldGeometry;

    AnatomicalPlane m_ViewDirection;
    AnatomicalPlane m_DefaultViewDirection;

    RenderingManager::Pointer m_RenderingManager;

    BaseRenderer* m_Renderer;

    bool m_BlockUpdate;

    bool m_SliceLocked;
    bool m_SliceRotationLocked;

    typedef std::map<void*, std::list<unsigned long>> ObserverTagsMapType;
    ObserverTagsMapType m_ReceiverToObserverTagsMap;
  };

} // namespace mitk

#endif
