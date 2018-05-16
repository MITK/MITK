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

#include "mitkRenderingManager.h"
#include "mitkBaseRenderer.h"
#include "mitkCameraController.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProportionalTimeGeometry.h"
#include "mitkRenderingManagerFactory.h"

#include <vtkRenderWindow.h>

#include "mitkNumericTypes.h"
#include <itkAffineGeometryFrame.h>
#include <itkCommand.h>
#include <itkScalableAffineTransform.h>
#include <mitkVtkPropRenderer.h>

#include <algorithm>

namespace mitk
{
  itkEventMacroDefinition(FocusChangedEvent, itk::AnyEvent)

    RenderingManager::Pointer RenderingManager::s_Instance = nullptr;
  RenderingManagerFactory *RenderingManager::s_RenderingManagerFactory = nullptr;

  RenderingManager::RenderingManager()
    : m_UpdatePending(false),
      m_MaxLOD(1),
      m_LODIncreaseBlocked(false),
      m_LODAbortMechanismEnabled(false),
      m_ClippingPlaneEnabled(false),
      m_TimeNavigationController(SliceNavigationController::New()),
      m_DataStorage(nullptr),
      m_ConstrainedPanningZooming(true),
      m_FocusedRenderWindow(nullptr)
  {
    m_ShadingEnabled.assign(3, false);
    m_ShadingValues.assign(4, 0.0);

    InitializePropertyList();
  }

  RenderingManager::~RenderingManager()
  {
    // Decrease reference counts of all registered vtkRenderWindows for
    // proper destruction
    RenderWindowVector::iterator it;
    for (it = m_AllRenderWindows.begin(); it != m_AllRenderWindows.end(); ++it)
    {
      (*it)->UnRegister(nullptr);

      auto callbacks_it = this->m_RenderWindowCallbacksList.find(*it);

      if (callbacks_it != this->m_RenderWindowCallbacksList.end())
      {
        (*it)->RemoveObserver(callbacks_it->second.commands[0u]);
        (*it)->RemoveObserver(callbacks_it->second.commands[1u]);
        (*it)->RemoveObserver(callbacks_it->second.commands[2u]);
      }
    }
  }

  void RenderingManager::SetFactory(RenderingManagerFactory *factory) { s_RenderingManagerFactory = factory; }
  const RenderingManagerFactory *RenderingManager::GetFactory() { return s_RenderingManagerFactory; }
  bool RenderingManager::HasFactory()
  {
    if (RenderingManager::s_RenderingManagerFactory)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  RenderingManager::Pointer RenderingManager::New()
  {
    const RenderingManagerFactory *factory = GetFactory();
    if (factory == nullptr)
      return nullptr;
    return factory->CreateRenderingManager();
  }

  RenderingManager *RenderingManager::GetInstance()
  {
    if (!RenderingManager::s_Instance)
    {
      if (s_RenderingManagerFactory)
      {
        s_Instance = s_RenderingManagerFactory->CreateRenderingManager();
      }
    }

    return s_Instance;
  }

  bool RenderingManager::IsInstantiated()
  {
    if (RenderingManager::s_Instance)
      return true;
    else
      return false;
  }

  void RenderingManager::AddRenderWindow(vtkRenderWindow *renderWindow)
  {
    if (renderWindow && (m_RenderWindowList.find(renderWindow) == m_RenderWindowList.end()))
    {
      m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;
      m_AllRenderWindows.push_back(renderWindow);

      if (m_DataStorage.IsNotNull())
        mitk::BaseRenderer::GetInstance(renderWindow)->SetDataStorage(m_DataStorage.GetPointer());

      // Register vtkRenderWindow instance
      renderWindow->Register(nullptr);

      // Add callbacks for rendering abort mechanism
      // BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
      vtkCallbackCommand *startCallbackCommand = vtkCallbackCommand::New();
      startCallbackCommand->SetCallback(RenderingManager::RenderingStartCallback);
      renderWindow->AddObserver(vtkCommand::StartEvent, startCallbackCommand);

      vtkCallbackCommand *progressCallbackCommand = vtkCallbackCommand::New();
      progressCallbackCommand->SetCallback(RenderingManager::RenderingProgressCallback);
      renderWindow->AddObserver(vtkCommand::AbortCheckEvent, progressCallbackCommand);

      vtkCallbackCommand *endCallbackCommand = vtkCallbackCommand::New();
      endCallbackCommand->SetCallback(RenderingManager::RenderingEndCallback);
      renderWindow->AddObserver(vtkCommand::EndEvent, endCallbackCommand);

      RenderWindowCallbacks callbacks;

      callbacks.commands[0u] = startCallbackCommand;
      callbacks.commands[1u] = progressCallbackCommand;
      callbacks.commands[2u] = endCallbackCommand;
      this->m_RenderWindowCallbacksList[renderWindow] = callbacks;

      // Delete vtk variables correctly
      startCallbackCommand->Delete();
      progressCallbackCommand->Delete();
      endCallbackCommand->Delete();
    }
  }

  void RenderingManager::RemoveRenderWindow(vtkRenderWindow *renderWindow)
  {
    if (m_RenderWindowList.erase(renderWindow))
    {
      auto callbacks_it = this->m_RenderWindowCallbacksList.find(renderWindow);
      if (callbacks_it != this->m_RenderWindowCallbacksList.end())
      {
        renderWindow->RemoveObserver(callbacks_it->second.commands[0u]);
        renderWindow->RemoveObserver(callbacks_it->second.commands[1u]);
        renderWindow->RemoveObserver(callbacks_it->second.commands[2u]);
        this->m_RenderWindowCallbacksList.erase(callbacks_it);
      }

      auto rw_it =
        std::find(m_AllRenderWindows.begin(), m_AllRenderWindows.end(), renderWindow);

      if (rw_it != m_AllRenderWindows.cend())
      {
        // Decrease reference count for proper destruction
        (*rw_it)->UnRegister(nullptr);
        m_AllRenderWindows.erase(rw_it);
      }
    }
  }

  const RenderingManager::RenderWindowVector &RenderingManager::GetAllRegisteredRenderWindows()
  {
    return m_AllRenderWindows;
  }

  void RenderingManager::RequestUpdate(vtkRenderWindow *renderWindow)
  {
    // If the renderWindow is not valid, we do not want to inadvertantly create
    // an entry in the m_RenderWindowList map. It is possible if the user is
    // regularly calling AddRenderer and RemoveRenderer for a rendering update
    // to come into this method with a renderWindow pointer that is valid in the
    // sense that the window does exist within the application, but that
    // renderWindow has been temporarily removed from this RenderingManager for
    // performance reasons.
    if (m_RenderWindowList.find(renderWindow) == m_RenderWindowList.cend())
    {
      return;
    }

    m_RenderWindowList[renderWindow] = RENDERING_REQUESTED;

    if (!m_UpdatePending)
    {
      m_UpdatePending = true;
      this->GenerateRenderingRequestEvent();
    }
  }

  void RenderingManager::ForceImmediateUpdate(vtkRenderWindow *renderWindow)
  {
    // If the renderWindow is not valid, we do not want to inadvertantly create
    // an entry in the m_RenderWindowList map. It is possible if the user is
    // regularly calling AddRenderer and RemoveRenderer for a rendering update
    // to come into this method with a renderWindow pointer that is valid in the
    // sense that the window does exist within the application, but that
    // renderWindow has been temporarily removed from this RenderingManager for
    // performance reasons.
    if (m_RenderWindowList.find(renderWindow) == m_RenderWindowList.cend())
    {
      return;
    }

    // Erase potentially pending requests for this window
    m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;

    m_UpdatePending = false;

    // Immediately repaint this window (implementation platform specific)
    // If the size is 0 it crashes
    int *size = renderWindow->GetSize();
    if (0 != size[0] && 0 != size[1])
    {
      // prepare the camera etc. before rendering
      // Note: this is a very important step which should be called before the VTK render!
      // If you modify the camera anywhere else or after the render call, the scene cannot be seen.
      auto *vPR = dynamic_cast<mitk::VtkPropRenderer *>(mitk::BaseRenderer::GetInstance(renderWindow));
      if (vPR)
        vPR->PrepareRender();
      // Execute rendering
      renderWindow->Render();
    }
  }

  void RenderingManager::RequestUpdateAll(RequestType type)
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
      if ((type == REQUEST_UPDATE_ALL) || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1)) ||
          ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)))
      {
        this->RequestUpdate(it->first);
      }
    }
  }

  void RenderingManager::ForceImmediateUpdateAll(RequestType type)
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
      if ((type == REQUEST_UPDATE_ALL) || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1)) ||
          ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)))
      {
        // Immediately repaint this window (implementation platform specific)
        // If the size is 0, it crashes
        this->ForceImmediateUpdate(it->first);
      }
    }
  }

  void RenderingManager::InitializeViewsByBoundingObjects(const DataStorage *ds)
  {
    if (!ds)
      return;

    // get all nodes that have not set "includeInBoundingBox" to false
    mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(
      mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));

    mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetSubset(pred);
    // calculate bounding geometry of these nodes
    auto bounds = ds->ComputeBoundingGeometry3D(rs, "visible");

    // initialize the views to the bounding geometry
    this->InitializeViews(bounds);
  }

  // TODO_GOETZ
  // Remove old function, so only this one is working.
  bool RenderingManager::InitializeViews(const BaseGeometry *dataGeometry,
                                         RequestType type,
                                         bool preserveRoughOrientationInWorldSpace)
  {
    ProportionalTimeGeometry::Pointer propTimeGeometry = ProportionalTimeGeometry::New();
    propTimeGeometry->Initialize(dynamic_cast<BaseGeometry *>(dataGeometry->Clone().GetPointer()), 1);
    return InitializeViews(propTimeGeometry, type, preserveRoughOrientationInWorldSpace);
  }

  bool RenderingManager::InitializeViews(const TimeGeometry *dataGeometry,
                                         RequestType type,
                                         bool /*preserveRoughOrientationInWorldSpace*/)
  {
    MITK_DEBUG << "initializing views";

    bool boundingBoxInitialized = false;

    TimeGeometry::ConstPointer timeGeometry = dataGeometry;
    TimeGeometry::Pointer modifiedGeometry = nullptr;
    if (dataGeometry != nullptr)
    {
      modifiedGeometry = dataGeometry->Clone();
    }

    int warningLevel = vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();

    if ((timeGeometry.IsNotNull()) &&
        (timeGeometry->GetBoundingBoxInWorld()->GetDiagonalLength2() > mitk::eps))
    {
      boundingBoxInitialized = true;
    }

    if (timeGeometry.IsNotNull())
    { // make sure bounding box has an extent bigger than zero in any direction
      // clone the input geometry
      // Old Geometry3D::Pointer modifiedGeometry = dynamic_cast<Geometry3D*>( dataGeometry->Clone().GetPointer() );
      assert(modifiedGeometry.IsNotNull());
      for (TimeStepType step = 0; step < modifiedGeometry->CountTimeSteps(); ++step)
      {
        BaseGeometry::BoundsArrayType newBounds = modifiedGeometry->GetGeometryForTimeStep(step)->GetBounds();
        for (unsigned int dimension = 0; (2 * dimension) < newBounds.Size(); dimension++)
        {
          // check for equality but for an epsilon
          if (Equal(newBounds[2 * dimension], newBounds[2 * dimension + 1]))
          {
            newBounds[2 * dimension + 1] += 1;
            if (Equal(
                  newBounds[2 * dimension],
                  newBounds[2 * dimension + 1])) // newBounds will still be equal if values are beyond double precision
            {
              mitkThrow() << "One dimension of object data has zero length, please make sure you're not using numbers "
                             "beyond double precision as coordinates.";
            }
          }
        }
        modifiedGeometry->GetGeometryForTimeStep(step)->SetBounds(newBounds);
      }
    }

    timeGeometry = modifiedGeometry;
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance(it->first);

      baseRenderer->SetConstrainZoomingAndPanning(m_ConstrainedPanningZooming);

      int id = baseRenderer->GetMapperID();
      if (((type == REQUEST_UPDATE_ALL) || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1)) ||
           ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2))))
      {
        this->InternalViewInitialization(baseRenderer, timeGeometry, boundingBoxInitialized, id);
      }
    }

    if (boundingBoxInitialized)
    {
      m_TimeNavigationController->SetInputWorldTimeGeometry(timeGeometry);
    }
    m_TimeNavigationController->Update();

    this->RequestUpdateAll(type);

    vtkObject::SetGlobalWarningDisplay(warningLevel);

    // Inform listeners that views have been initialized
    this->InvokeEvent(mitk::RenderingManagerViewsInitializedEvent());

    return boundingBoxInitialized;
  }

  bool RenderingManager::InitializeViews(RequestType type)
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance(it->first);
      int id = baseRenderer->GetMapperID();
      if ((type == REQUEST_UPDATE_ALL) || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1)) ||
          ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)))
      {
        mitk::SliceNavigationController *nc =

          baseRenderer->GetSliceNavigationController();

        // Re-initialize view direction
        nc->SetViewDirectionToDefault();

        // Update the SNC
        nc->Update();
      }
    }

    this->RequestUpdateAll(type);

    return true;
  }

  bool RenderingManager::InitializeView(vtkRenderWindow *renderWindow,
                                        const BaseGeometry *geometry,
                                        bool initializeGlobalTimeSNC)
  {
    ProportionalTimeGeometry::Pointer propTimeGeometry = ProportionalTimeGeometry::New();
    propTimeGeometry->Initialize(dynamic_cast<BaseGeometry *>(geometry->Clone().GetPointer()), 1);
    return InitializeView(renderWindow, propTimeGeometry, initializeGlobalTimeSNC);
  }

  bool RenderingManager::InitializeView(vtkRenderWindow *renderWindow,
                                        const TimeGeometry *geometry,
                                        bool initializeGlobalTimeSNC)
  {
    bool boundingBoxInitialized = false;

    int warningLevel = vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();

    if ((geometry != nullptr) &&
        (geometry->GetBoundingBoxInWorld()->GetDiagonalLength2() > mitk::eps))
    {
      boundingBoxInitialized = true;
    }

    mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);

    int id = baseRenderer->GetMapperID();

    this->InternalViewInitialization(baseRenderer, geometry, boundingBoxInitialized, id);

    if (boundingBoxInitialized && initializeGlobalTimeSNC)
    {
      m_TimeNavigationController->SetInputWorldTimeGeometry(geometry);
    }
    m_TimeNavigationController->Update();

    this->RequestUpdate(renderWindow);

    vtkObject::SetGlobalWarningDisplay(warningLevel);

    return boundingBoxInitialized;
  }

  bool RenderingManager::InitializeView(vtkRenderWindow *renderWindow)
  {
    mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance(renderWindow);

    mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

    // Re-initialize view direction
    nc->SetViewDirectionToDefault();

    // Update the SNC
    nc->Update();

    this->RequestUpdate(renderWindow);

    return true;
  }

  void RenderingManager::InternalViewInitialization(mitk::BaseRenderer *baseRenderer,
                                                    const mitk::TimeGeometry *geometry,
                                                    bool boundingBoxInitialized,
                                                    int mapperID)
  {
    mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

    // Re-initialize view direction
    nc->SetViewDirectionToDefault();

    if (boundingBoxInitialized)
    {
      // Set geometry for NC
      nc->SetInputWorldTimeGeometry(geometry);
      nc->Update();

      if (mapperID == BaseRenderer::Standard2D)
      {
        // For 2D SNCs, steppers are set so that the cross is centered
        // in the image
        nc->GetSlice()->SetPos(nc->GetSlice()->GetSteps() / 2);
      }

      baseRenderer->GetCameraController()->SetViewToAnterior();
      baseRenderer->GetCameraController()->Fit();
    }
    else
    {
      nc->Update();
    }
  }

  const SliceNavigationController *RenderingManager::GetTimeNavigationController() const
  {
    return m_TimeNavigationController.GetPointer();
  }

  SliceNavigationController *RenderingManager::GetTimeNavigationController()
  {
    return m_TimeNavigationController.GetPointer();
  }

  void RenderingManager::ExecutePendingRequests()
  {
    m_UpdatePending = false;

    // Satisfy all pending update requests
    RenderWindowList::const_iterator it;
    int i = 0;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it, ++i)
    {
      if (it->second == RENDERING_REQUESTED)
      {
        this->ForceImmediateUpdate(it->first);
      }
    }
  }

  void RenderingManager::RenderingStartCallback(vtkObject *caller, unsigned long, void *, void *)
  {
    auto *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);
    mitk::RenderingManager *renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();
    RenderWindowList &renderWindowList = renman->m_RenderWindowList;

    if (renderWindow)
    {
      renderWindowList[renderWindow] = RENDERING_INPROGRESS;
    }

    renman->m_UpdatePending = false;
  }

  void RenderingManager::RenderingProgressCallback(vtkObject *caller, unsigned long, void *, void *)
  {
    auto *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);
    mitk::RenderingManager *renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();

    if (renman->m_LODAbortMechanismEnabled)
    {
      auto *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);
      if (renderWindow)
      {
        BaseRenderer *renderer = BaseRenderer::GetInstance(renderWindow);
        if (renderer && (renderer->GetNumberOfVisibleLODEnabledMappers() > 0))
        {
          renman->DoMonitorRendering();
        }
      }
    }
  }

  void RenderingManager::RenderingEndCallback(vtkObject *caller, unsigned long, void *, void *)
  {
    auto *renderWindow = dynamic_cast<vtkRenderWindow *>(caller);

    mitk::RenderingManager *renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();

    RenderWindowList &renderWindowList = renman->m_RenderWindowList;

    RendererIntMap &nextLODMap = renman->m_NextLODMap;

    if (renderWindow)
    {
      BaseRenderer *renderer = BaseRenderer::GetInstance(renderWindow);
      if (renderer)
      {
        renderWindowList[renderer->GetRenderWindow()] = RENDERING_INACTIVE;

        // Level-of-Detail handling
        if (renderer->GetNumberOfVisibleLODEnabledMappers() > 0)
        {
          if (nextLODMap[renderer] == 0)
            renman->StartOrResetTimer();
          else
            nextLODMap[renderer] = 0;
        }
      }
    }
  }

  bool RenderingManager::IsRendering() const
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      if (it->second == RENDERING_INPROGRESS)
      {
        return true;
      }
    }
    return false;
  }

  void RenderingManager::AbortRendering()
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      if (it->second == RENDERING_INPROGRESS)
      {
        it->first->SetAbortRender(true);
        m_RenderingAbortedMap[BaseRenderer::GetInstance(it->first)] = true;
      }
    }
  }

  int RenderingManager::GetNextLOD(BaseRenderer *renderer)
  {
    if (renderer != nullptr)
    {
      return m_NextLODMap[renderer];
    }
    else
    {
      return 0;
    }
  }

  void RenderingManager::ExecutePendingHighResRenderingRequest()
  {
    RenderWindowList::const_iterator it;
    for (it = m_RenderWindowList.cbegin(); it != m_RenderWindowList.cend(); ++it)
    {
      BaseRenderer *renderer = BaseRenderer::GetInstance(it->first);

      if (renderer->GetNumberOfVisibleLODEnabledMappers() > 0)
      {
        if (m_NextLODMap[renderer] == 0)
        {
          m_NextLODMap[renderer] = 1;
          RequestUpdate(it->first);
        }
      }
    }
  }

  void RenderingManager::SetMaximumLOD(unsigned int max) { m_MaxLOD = max; }
  // enable/disable shading
  void RenderingManager::SetShading(bool state, unsigned int lod)
  {
    if (lod > m_MaxLOD)
    {
      itkWarningMacro(<< "LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
      return;
    }
    m_ShadingEnabled[lod] = state;
  }

  bool RenderingManager::GetShading(unsigned int lod)
  {
    if (lod > m_MaxLOD)
    {
      itkWarningMacro(<< "LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
      return false;
    }
    return m_ShadingEnabled[lod];
  }

  // enable/disable the clipping plane
  void RenderingManager::SetClippingPlaneStatus(bool status) { m_ClippingPlaneEnabled = status; }
  bool RenderingManager::GetClippingPlaneStatus() { return m_ClippingPlaneEnabled; }
  void RenderingManager::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
  {
    m_ShadingValues[0] = ambient;
    m_ShadingValues[1] = diffuse;
    m_ShadingValues[2] = specular;
    m_ShadingValues[3] = specpower;
  }

  RenderingManager::FloatVector &RenderingManager::GetShadingValues() { return m_ShadingValues; }
  void RenderingManager::InitializePropertyList()
  {
    if (m_PropertyList.IsNull())
    {
      m_PropertyList = PropertyList::New();
    }

    this->SetProperty("coupled-zoom", BoolProperty::New(false));
    this->SetProperty("coupled-plane-rotation", BoolProperty::New(false));
    this->SetProperty("MIP-slice-rendering", BoolProperty::New(false));
  }

  PropertyList::Pointer RenderingManager::GetPropertyList() const { return m_PropertyList; }
  BaseProperty *RenderingManager::GetProperty(const char *propertyKey) const
  {
    return m_PropertyList->GetProperty(propertyKey);
  }

  void RenderingManager::SetProperty(const char *propertyKey, BaseProperty *propertyValue)
  {
    m_PropertyList->SetProperty(propertyKey, propertyValue);
  }

  void RenderingManager::SetDataStorage(DataStorage *storage)
  {
    if (storage != nullptr)
    {
      m_DataStorage = storage;

      RenderingManager::RenderWindowVector::const_iterator iter;
      for (iter = m_AllRenderWindows.cbegin(); iter < m_AllRenderWindows.cend(); ++iter)
      {
        mitk::BaseRenderer::GetInstance((*iter))->SetDataStorage(m_DataStorage.GetPointer());
      }
    }
  }

  mitk::DataStorage *RenderingManager::GetDataStorage() { return m_DataStorage; }
  void RenderingManager::SetRenderWindowFocus(vtkRenderWindow *focusWindow)
  {
    if (focusWindow != m_FocusedRenderWindow)
    {
      if (!focusWindow || (m_RenderWindowList.find(focusWindow) != m_RenderWindowList.cend()))
      {
        m_FocusedRenderWindow = focusWindow;
        this->InvokeEvent(FocusChangedEvent());
        return;
      }

      MITK_ERROR << "Tried to set a RenderWindow that does not exist.";
    }
  }

  // Create and register generic RenderingManagerFactory.
  TestingRenderingManagerFactory renderingManagerFactory;
} // namespace
