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

#ifndef MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include <MitkCoreExports.h>

#include <vtkCallbackCommand.h>

#include <string>
#include <itkObject.h>
#include <itkObjectFactory.h>

#include "mitkPropertyList.h"
#include "mitkProperties.h"
#include "mitkTimeGeometry.h"

class vtkRenderWindow;
class vtkObject;

namespace mitk
{
class RenderingManager;
class RenderingManagerFactory;
class BaseGeometry;
class SliceNavigationController;
class BaseRenderer;
class DataStorage;
class GlobalInteraction;

/**
 * \brief Manager for coordinating the rendering process.
 *
 * RenderingManager is a central instance retrieving and executing
 * RenderWindow update requests. Its main purpose is to coordinate
 * distributed requests which cannot be aware of each other - lacking the
 * knowledge of whether they are really necessary or not. For example, two
 * objects might determine that a specific RenderWindow needs to be updated.
 * This would result in one unnecessary update, if both executed the update
 * on their own.
 *
 * The RenderingManager addresses this by letting each such object
 * <em>request</em> an update, and waiting for other objects to possibly
 * issue the same request. The actual update will then only be executed at a
 * well-defined point in the main event loop (this may be each time after
 * event processing is done).
 *
 * Convinience methods for updating all RenderWindows which have been
 * registered with the RenderingManager exist. If theses methods are not
 * used, it is not required to register (add) RenderWindows prior to using
 * the RenderingManager.
 *
 * The methods #ForceImmediateUpdate() and #ForceImmediateUpdateAll() can
 * be used to force the RenderWindow update execution without any delay,
 * bypassing the request functionality.
 *
 * The interface of RenderingManager is platform independent. Platform
 * specific subclasses have to be implemented, though, to supply an
 * appropriate event issueing for controlling the update execution process.
 * See method documentation for a description of how this can be done.
 *
 * \sa TestingRenderingManager An "empty" RenderingManager implementation which
 * can be used in tests etc.
 *
 */
class MITK_CORE_EXPORT RenderingManager : public itk::Object
{
public:

  mitkClassMacro(RenderingManager,itk::Object);

  typedef std::vector< vtkRenderWindow* > RenderWindowVector;
  typedef std::vector< float > FloatVector;
  typedef std::vector< bool > BoolVector;

  typedef itk::SmartPointer< DataStorage > DataStoragePointer;
  typedef itk::SmartPointer< GlobalInteraction > GlobalInteractionPointer;

  enum RequestType
  {
    REQUEST_UPDATE_ALL = 0,
    REQUEST_UPDATE_2DWINDOWS,
    REQUEST_UPDATE_3DWINDOWS
  };

  static Pointer New();

  /** Set the object factory which produces the desired platform specific
   * RenderingManager singleton instance. */
  static void SetFactory( RenderingManagerFactory *factory );

  /** Get the object factory which produces the platform specific
   * RenderingManager instances. */
  static const RenderingManagerFactory *GetFactory();

  /** Returns true if a factory has already been set. */
  static bool HasFactory();

  /** Get the RenderingManager singleton instance. */
  static RenderingManager *GetInstance();

  /** Returns true if the singleton instance does already exist. */
  static bool IsInstantiated();

  /** Adds a RenderWindow. This is required if the methods #RequestUpdateAll
   * or #ForceImmediateUpdate are to be used. */
  void AddRenderWindow( vtkRenderWindow *renderWindow );

  /** Removes a RenderWindow. */
  void RemoveRenderWindow( vtkRenderWindow *renderWindow );

  /** Get a list of all registered RenderWindows */
  const RenderWindowVector &GetAllRegisteredRenderWindows();

    /** Requests an update for the specified RenderWindow, to be executed as
   * soon as the main loop is ready for rendering. */
  void RequestUpdate( vtkRenderWindow *renderWindow );

  /** Immediately executes an update of the specified RenderWindow. */
  void ForceImmediateUpdate( vtkRenderWindow *renderWindow );

  /** Requests all currently registered RenderWindows to be updated.
   * If only 2D or 3D windows should be updated, this can be specified
   * via the parameter requestType. */
  void RequestUpdateAll( RequestType type = REQUEST_UPDATE_ALL );

  /** Immediately executes an update of all registered RenderWindows.
   * If only 2D or 3D windows should be updated, this can be specified
   * via the parameter requestType. */
  void ForceImmediateUpdateAll( RequestType type = REQUEST_UPDATE_ALL );

  /** Initializes the windows specified by requestType to the geometry of the
   * given DataStorage. */
  //virtual bool InitializeViews( const DataStorage *storage, const DataNode* node = NULL,
  //   RequestType type = REQUEST_UPDATE_ALL, bool preserveRoughOrientationInWorldSpace = false );

  /** Initializes the windows specified by requestType to the given
   * geometry. PLATFORM SPECIFIC. TODO: HOW IS THIS PLATFORM SPECIFIC? */
  virtual bool InitializeViews( const BaseGeometry *geometry,
    RequestType type = REQUEST_UPDATE_ALL, bool preserveRoughOrientationInWorldSpace = false );
  virtual bool InitializeViews( const TimeGeometry *geometry,
    RequestType type = REQUEST_UPDATE_ALL, bool preserveRoughOrientationInWorldSpace = false );

  /** Initializes the windows to the default viewing direction
   * (geomtry information is NOT changed). PLATFORM SPECIFIC. */
  virtual bool InitializeViews( RequestType type = REQUEST_UPDATE_ALL );

  /** Initializes the specified window to the geometry of the given
   * DataNode. Set "initializeGlobalTimeSNC" to true in order to use this
   * geometry as global TimeGeometry. PLATFORM SPECIFIC. */
  //virtual bool InitializeView( vtkRenderWindow *renderWindow, const DataStorage* ds, const DataNode* node = NULL, bool initializeGlobalTimeSNC = false );

  /** Initializes the specified window to the given geometry. Set
   * "initializeGlobalTimeSNC" to true in order to use this geometry as
   * global TimeGeometry. PLATFORM SPECIFIC. */
  virtual bool InitializeView( vtkRenderWindow *renderWindow, const BaseGeometry *geometry, bool initializeGlobalTimeSNC = false);
  virtual bool InitializeView( vtkRenderWindow *renderWindow, const TimeGeometry *geometry, bool initializeGlobalTimeSNC = false);

  /** Initializes the specified window to the default viewing direction
   * (geomtry information is NOT changed). PLATFORM SPECIFIC. */
  virtual bool InitializeView( vtkRenderWindow *renderWindow );

  /**
   * @brief Initializes the renderwindows by the aggregated geometry of
   *        all objects that are held in the data storage.
   * This is basically a global reinit
   * @param The data storage from which the bounding object can be retrieved
   */
  virtual void InitializeViewsByBoundingObjects(const DataStorage * );

  /** Gets the (global) SliceNavigationController responsible for
   * time-slicing. */
  const SliceNavigationController *GetTimeNavigationController() const;

  /** Gets the (global) SliceNavigationController responsible for
   * time-slicing. */
  SliceNavigationController *GetTimeNavigationController();

  virtual ~RenderingManager();

  /** Executes all pending requests. This method has to be called by the
   * system whenever a RenderingManager induced request event occurs in
   * the system pipeline (see concrete RenderingManager implementations). */
  virtual void ExecutePendingRequests();

  bool IsRendering() const;
  void AbortRendering();

  /** En-/Disable LOD increase globally. */
  itkSetMacro( LODIncreaseBlocked, bool );

  /** En-/Disable LOD increase globally. */
  itkGetMacro( LODIncreaseBlocked, bool );

  /** En-/Disable LOD increase globally. */
  itkBooleanMacro( LODIncreaseBlocked );

  /** En-/Disable LOD abort mechanism. */
  itkSetMacro( LODAbortMechanismEnabled, bool );

  /** En-/Disable LOD abort mechanism. */
  itkGetMacro( LODAbortMechanismEnabled, bool );

  /** En-/Disable LOD abort mechanism. */
  itkBooleanMacro( LODAbortMechanismEnabled );

  /** Force a sub-class to start a timer for a pending hires-rendering request */
  virtual void StartOrResetTimer() {};

  /** To be called by a sub-class from a timer callback */
  void ExecutePendingHighResRenderingRequest();

  virtual void DoStartRendering() {};
  virtual void DoMonitorRendering() {};
  virtual void DoFinishAbortRendering() {};

  int GetNextLOD( BaseRenderer* renderer );

  /** Set current LOD (NULL means all renderers)*/
  void SetMaximumLOD( unsigned int max );

  void SetShading( bool state, unsigned int lod );
  bool GetShading( unsigned int lod );

  void SetClippingPlaneStatus( bool status );
  bool GetClippingPlaneStatus();

  void SetShadingValues( float ambient, float diffuse,
    float specular, float specpower );

  FloatVector &GetShadingValues();

  /** Returns a property list */
  PropertyList::Pointer GetPropertyList() const;

  /** Returns a property from m_PropertyList */
  BaseProperty* GetProperty(const char *propertyKey) const;

  /** Sets or adds (if not present) a property in m_PropertyList  */
  void SetProperty(const char *propertyKey, BaseProperty* propertyValue);

  /**
  * \brief Setter / Getter for internal DataStorage
  *
  * Sets / returns the mitk::DataStorage that is used internally. This instance holds all mitk::DataNodes that are
  * rendered by the registered BaseRenderers.
  *
  * If this DataStorage is changed at runtime by calling SetDataStorage(),
  * all currently registered BaseRenderers are automatically given the correct instance.
  * When a new BaseRenderer is added, it is automatically initialized with the currently active DataStorage.
  */
  void SetDataStorage( mitk::DataStorage* storage );

  /**
  * \brief Setter / Getter for internal DataStorage
  *
  * Sets / returns the mitk::DataStorage that is used internally. This instance holds all mitk::DataNodes that are
  * rendered by the registered BaseRenderers.
  *
  * If this DataStorage is changed at runtime by calling SetDataStorage(),
  * all currently registered BaseRenderers are automatically given the correct instance.
  * When a new BaseRenderer is added, it is automatically initialized with the currently active DataStorage.
  */
  mitk::DataStorage* GetDataStorage();

  /**
  * \brief Setter / Getter for internal GloabInteraction
  *
  * Sets / returns the instance of mitk::GlobalInteraction that is internally held.
  * It'S not actually used by this class but offers it to all registered BaseRenderers.
  * These need it for their own internal initialization of the FocusManager and the corresponding EventMappers.
  */
  void SetGlobalInteraction( mitk::GlobalInteraction* globalInteraction );

  /**
  * \brief Setter / Getter for internal GloabInteraction
  *
  * Sets / returns the instance of mitk::GlobalInteraction that is internally held.
  * It'S not actually used by this class but offers it to all registered BaseRenderers.
  * These need it for their own internal initialization of the FocusManager and the corresponding EventMappers.
  */
  mitk::GlobalInteraction* GetGlobalInteraction();

  itkSetMacro(ConstrainedPaddingZooming, bool);

protected:
  enum
  {
    RENDERING_INACTIVE = 0,
    RENDERING_REQUESTED,
    RENDERING_INPROGRESS
  };

  RenderingManager();

  /** Abstract method for generating a system specific event for rendering
   * request. This method is called whenever an update is requested */
  virtual void GenerateRenderingRequestEvent() = 0;

  virtual void InitializePropertyList();

  bool m_UpdatePending;

  typedef std::map< BaseRenderer *, unsigned int > RendererIntMap;
  typedef std::map< BaseRenderer *, bool > RendererBoolMap;

  RendererBoolMap m_RenderingAbortedMap;

  RendererIntMap m_NextLODMap;

  unsigned int m_MaxLOD;

  bool m_LODIncreaseBlocked;

  bool m_LODAbortMechanismEnabled;

  BoolVector m_ShadingEnabled;

  bool m_ClippingPlaneEnabled;

  FloatVector m_ShadingValues;

  static void RenderingStartCallback(
    vtkObject *caller, unsigned long eid, void *clientdata, void *calldata );
  static void RenderingProgressCallback(
    vtkObject *caller, unsigned long eid, void *clientdata, void *calldata );
  static void RenderingEndCallback(
    vtkObject *caller, unsigned long eid, void *clientdata, void *calldata );

  typedef std::map< vtkRenderWindow *, int > RenderWindowList;

  RenderWindowList m_RenderWindowList;
  RenderWindowVector m_AllRenderWindows;

  struct RenderWindowCallbacks
  {
    vtkCallbackCommand* commands[3u];
  };

  typedef std::map<vtkRenderWindow*, RenderWindowCallbacks> RenderWindowCallbacksList;

  RenderWindowCallbacksList m_RenderWindowCallbacksList;

  itk::SmartPointer<SliceNavigationController> m_TimeNavigationController;

  static RenderingManager::Pointer s_Instance;
  static RenderingManagerFactory *s_RenderingManagerFactory;

  PropertyList::Pointer m_PropertyList;

  DataStoragePointer m_DataStorage;

  GlobalInteractionPointer m_GlobalInteraction;

  bool m_ConstrainedPaddingZooming;

private:

  void InternalViewInitialization(
      mitk::BaseRenderer *baseRenderer, const mitk::TimeGeometry *geometry,
      bool boundingBoxInitialized, int mapperID );
};

#pragma GCC visibility push(default)

itkEventMacro( RenderingManagerEvent, itk::AnyEvent );
itkEventMacro( RenderingManagerViewsInitializedEvent, RenderingManagerEvent );

#pragma GCC visibility pop

/**
 * Generic RenderingManager implementation for "non-rendering-plattform",
 * e.g. for tests. Its factory (TestingRenderingManagerFactory) is
 * automatically on start-up and is used by default if not other
 * RenderingManagerFactory is instantiated explicitly thereafter.
 * (see mitkRenderingManager.cpp)
 */
class MITK_CORE_EXPORT TestingRenderingManager : public RenderingManager
{
public:
  mitkClassMacro(TestingRenderingManager,RenderingManager);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

protected:
  virtual void GenerateRenderingRequestEvent()
  {
   // ForceImmediateUpdateAll();
  };
};
} // namespace mitk

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
