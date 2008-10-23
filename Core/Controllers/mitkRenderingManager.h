/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197
#define MITKRENDERINGMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkCommon.h"
#include "mitkDataTree.h"


#include <string>
#include <itkObject.h>
#include <itkObjectFactory.h>

class vtkRenderWindow;
class vtkObject;

namespace mitk
{

class RenderingManager;
class RenderingManagerFactory;
class Geometry3D;
class SliceNavigationController;
class DataStorage;

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
 * \sa GenericRenderingManager An "empty" RenderingManager implementation which
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
   * given DataTreeNode. PLATFORM SPECIFIC. */
  virtual bool InitializeViews( DataTreeIteratorBase *dataIt,
    RequestType type = REQUEST_UPDATE_ALL );


  /** Initializes the windows specified by requestType to the geometry of the
   * given DataStorage. */
  virtual bool InitializeViews( const DataStorage *storage,
     RequestType type = REQUEST_UPDATE_ALL );

  /** Initializes the windows specified by requestType to the given
   * geometry. PLATFORM SPECIFIC. */
  virtual bool InitializeViews( const Geometry3D *geometry,
    RequestType type = REQUEST_UPDATE_ALL );


  /** Initializes the windows to the default viewing direction
   * (geomtry information is NOT changed). PLATFORM SPECIFIC. */
  virtual bool InitializeViews( RequestType type = REQUEST_UPDATE_ALL );


  /** Initializes the specified window to the geometry of the given
   * DataTreeNode. Set "initializeGlobalTimeSNC" to true in order to use this
   * geometry as global TimeSlicedGeometry. PLATFORM SPECIFIC. */
  virtual bool InitializeView( vtkRenderWindow *renderWindow,
    DataTreeIteratorBase *dataIt, bool initializeGlobalTimeSNC = false );

  /** Initializes the specified window to the given geometry. Set
   * "initializeGlobalTimeSNC" to true in order to use this geometry as
   * global TimeSlicedGeometry. PLATFORM SPECIFIC. */
  virtual bool InitializeView( vtkRenderWindow *renderWindow,
    const Geometry3D *geometry, bool initializeGlobalTimeSNC = false);

  /** Initializes the specified window to the default viewing direction
   * (geomtry information is NOT changed). PLATFORM SPECIFIC. */
  virtual bool InitializeView( vtkRenderWindow *renderWindow );


  /** Sets the (global) SliceNavigationController responsible for
   * time-slicing. */
  void SetTimeNavigationController( SliceNavigationController *snc );

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
  virtual void UpdateCallback();

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


  virtual void DoStartRendering() {};
  virtual void DoMonitorRendering() {};
  virtual void DoFinishAbortRendering() {};

  int GetNextLOD( BaseRenderer *renderer );

  /** Set current LOD (NULL means all renderers)*/
  void SetNextLOD( unsigned int lod, BaseRenderer *renderer = NULL );
  void SetMaximumLOD( unsigned int max );

  void SetShading( bool state, unsigned int lod );
  bool GetShading( unsigned int lod );

  void SetClippingPlaneStatus( bool status );
  bool GetClippingPlaneStatus();

  void SetShadingValues( float ambient, float diffuse,
    float specular, float specpower );

  FloatVector &GetShadingValues();


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

  SliceNavigationController *m_TimeNavigationController;

  static RenderingManager::Pointer s_Instance;
  static RenderingManagerFactory *s_RenderingManagerFactory;

private:

  void InternalViewInitialization(
      mitk::BaseRenderer *baseRenderer, const mitk::Geometry3D *geometry,
      bool boundingBoxInitialized, int mapperID );

};

itkEventMacro( RenderingManagerEvent, itk::AnyEvent );
itkEventMacro( RenderingManagerViewsInitializedEvent, RenderingManagerEvent );


/**
 * Generic RenderingManager implementation for "non-rendering-plattform",
 * e.g. for tests. Its factory (GenericRenderingManagerFactory) is
 * automatically on start-up and is used by default if not other
 * RenderingManagerFactory is instantiated explicitly thereafter.
 * (see mitkRenderingManager.cpp)
 */
class MITK_CORE_EXPORT GenericRenderingManager : public RenderingManager
{
public:
  mitkClassMacro(GenericRenderingManager,RenderingManager);
  itkNewMacro(Self);

protected:
  virtual void GenerateRenderingRequestEvent()
  {
  };

};


} // namespace mitk

#endif /* MITKRenderingManager_H_HEADER_INCLUDED_C135A197 */
