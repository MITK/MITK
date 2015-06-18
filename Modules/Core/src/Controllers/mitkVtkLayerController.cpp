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

#include "mitkVtkLayerController.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <algorithm>

mitk::VtkLayerController::vtkLayerControllerMapType mitk::VtkLayerController::s_LayerControllerMap;

mitk::VtkLayerController* mitk::VtkLayerController::GetInstance(vtkSmartPointer<vtkRenderWindow> renWin)
{
  for(auto mapit = s_LayerControllerMap.begin();
      mapit != s_LayerControllerMap.end(); mapit++)
  {
    if( (*mapit).first == renWin)
      return (*mapit).second;

  }
  return nullptr;
}

void  mitk::VtkLayerController::AddInstance(vtkSmartPointer<vtkRenderWindow> renWin,
                                            vtkSmartPointer<vtkRenderer> mitkSceneRenderer)
{
  // ensure that no vtkRenderWindow is managed twice
  mitk::VtkLayerController::RemoveInstance(renWin);

  // instanciate controller, add it to the map
  mitk::VtkLayerController* ControllerInstance = new mitk::VtkLayerController(renWin);
  ControllerInstance->InsertSceneRenderer(mitkSceneRenderer);

  s_LayerControllerMap.insert(vtkLayerControllerMapType::value_type(renWin,ControllerInstance));
}

void  mitk::VtkLayerController::RemoveInstance(vtkSmartPointer<vtkRenderWindow> renWin)
{
  auto mapit = s_LayerControllerMap.find(renWin);
  if(mapit != s_LayerControllerMap.end())
  {
    delete mapit->second;
    s_LayerControllerMap.erase( mapit );
  }
}

mitk::VtkLayerController::VtkLayerController(vtkSmartPointer<vtkRenderWindow> renderWindow)
{
  m_RenderWindow = renderWindow;
  m_RenderWindow->Register( nullptr );
  m_BackgroundRenderers.clear();
  m_ForegroundRenderers.clear();
  m_SceneRenderers.clear();

}

mitk::VtkLayerController::~VtkLayerController()
{
  if ( m_RenderWindow != nullptr )
  {
    m_RenderWindow->UnRegister( nullptr );
  }
}

/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the background.
 * With forceAbsoluteBackground set true a renderer can be placed at the absolute background of the scene.
 * Multiple calls with forceAbsoluteBackground set true will set the latest registered renderer as background.
 */
void mitk::VtkLayerController::InsertBackgroundRenderer(vtkSmartPointer<vtkRenderer>
                                                        renderer, bool forceAbsoluteBackground)
{

  if(renderer == nullptr)
    return;

  // Remove renderer if it already exists
  RemoveRenderer(renderer);

  if(forceAbsoluteBackground)
  {
    auto it = m_BackgroundRenderers.begin();
    m_BackgroundRenderers.insert(it,renderer);
  }
  else
    m_BackgroundRenderers.push_back(renderer);
  UpdateLayers();
}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the foreground.
 * With forceAbsoluteBackground set true a renderer can be placed at the absolute foreground of the scene.
 * Multiple calls with forceAbsoluteForeground set true will set the latest registered renderer as foreground.
 */
void mitk::VtkLayerController::InsertForegroundRenderer(vtkSmartPointer<vtkRenderer> renderer,
                                                        bool forceAbsoluteForeground)
{

  if(renderer == nullptr)
    return;

  // Remove renderer if it already exists
  RemoveRenderer(renderer);

  if(forceAbsoluteForeground)
  {
    auto it = m_ForegroundRenderers.begin();
    m_ForegroundRenderers.insert(it,renderer);
  }
  else
    m_ForegroundRenderers.push_back(renderer);

  renderer->PreserveDepthBufferOn();
  UpdateLayers();
}
/**
 * Returns the Scene Renderer
 */
vtkSmartPointer<vtkRenderer> mitk::VtkLayerController::GetSceneRenderer()
{
  if(m_SceneRenderers.size() > 0)
  {
    auto it = m_SceneRenderers.begin();
    return (*it);
  }
  else
    return nullptr;
}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered between background renderers and
 * foreground renderers.
 */
void mitk::VtkLayerController::InsertSceneRenderer(vtkSmartPointer<vtkRenderer> renderer)
{

  if(renderer == nullptr)
    return;

  // Remove renderer if it already exists
  RemoveRenderer(renderer);

  m_SceneRenderers.push_back(renderer);
  UpdateLayers();
}
/**
 * A renderer which has been inserted via a insert... function can be removed from the vtkRenderWindow with
 * RemoveRenderer.
 */
void mitk::VtkLayerController::RemoveRenderer(vtkSmartPointer<vtkRenderer> renderer)
{
  RendererVectorType::iterator it;
  // background layers
  if(m_BackgroundRenderers.size() > 0)
  {
    it = std::find(m_BackgroundRenderers.begin(),m_BackgroundRenderers.end(),renderer);
    if(it != m_BackgroundRenderers.end())
    {
      m_BackgroundRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
  // scene layers
  if(m_SceneRenderers.size() > 0)
  {
    it = std::find(m_SceneRenderers.begin(),m_SceneRenderers.end(),renderer);
    if(it != m_SceneRenderers.end())
    {
      m_SceneRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
  // foreground layers
  if(m_ForegroundRenderers.size() > 0 )
  {
    it = std::find(m_ForegroundRenderers.begin(),m_ForegroundRenderers.end(),renderer);
    if(it != m_ForegroundRenderers.end())
    {
      m_ForegroundRenderers.erase(it);
      UpdateLayers();
      return;
    }
  }
}
/**
 * Connects a VtkRenderWindow with the layer controller.
 */
void mitk::VtkLayerController::SetRenderWindow(vtkSmartPointer<vtkRenderWindow> renwin)
{
  if(renwin != nullptr)
  {
    RendererVectorType::iterator it;
    // Tell all renderers that there is a new renderwindow
    for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
    {
      (*it)->SetRenderWindow(renwin);
    }
    // Set the new RenderWindow
    m_RenderWindow = renwin;
  }
  // Now sort renderers and add them to the renderwindow
  UpdateLayers();
}

/**
* Returns true if a renderer has been inserted
*/
bool mitk::VtkLayerController::IsRendererInserted(vtkSmartPointer<vtkRenderer> renderer)
{
  RendererVectorType::iterator it;
  // background layers
  if(m_BackgroundRenderers.size() > 0)
  {
    it = std::find(m_BackgroundRenderers.begin(),m_BackgroundRenderers.end(),renderer);
    if ( it != m_BackgroundRenderers.end() )
    {
      return true;
    }
  }
  // scene layers
  if(m_SceneRenderers.size() > 0)
  {
    it = std::find(m_SceneRenderers.begin(),m_SceneRenderers.end(),renderer);
    if ( it != m_SceneRenderers.end() )
    {
      return true;
    }
  }
  // foreground layers
  if(m_ForegroundRenderers.size() > 0 )
  {
    it = std::find(m_ForegroundRenderers.begin(),m_ForegroundRenderers.end(),renderer);
    if ( it != m_ForegroundRenderers.end() )
    {
      return true;
    }
  }
  return false;

}
/**
 * Internally used to sort all registered renderers and to connect the with the vtkRenderWindow.
 * Mention that VTK Version 5 and above is rendering higher numbers in the background and VTK
 * Verison < 5 in the foreground.
 */
void mitk::VtkLayerController::UpdateLayers()
{
  // Remove all Renderers from renderwindow
  vtkSmartPointer<vtkRendererCollection> v = m_RenderWindow->GetRenderers();
  v->RemoveAllItems();


  unsigned int numberOfLayers = static_cast<unsigned int>(m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size());
  int currentLayerNumber;
  bool traverseUpwards;

  currentLayerNumber  = 0;
  traverseUpwards     = true;

  m_RenderWindow->SetNumberOfLayers(numberOfLayers);
  RendererVectorType::iterator it;
  // assign a layer number for the backround renderers
  for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
  {
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));

    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
  // assign a layer number for the scene renderers
  for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
  {
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));

    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
  // assign a layer number for the foreground renderers
  for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
  {
    (*it)->SetRenderWindow(m_RenderWindow);
    (*it)->SetLayer(currentLayerNumber);
    m_RenderWindow->AddRenderer((*it));

    if(traverseUpwards)
      currentLayerNumber++;
    else
      currentLayerNumber--;
  }
}
/**
 * Returns the number of renderers in the renderwindow.
 */
unsigned int mitk::VtkLayerController::GetNumberOfRenderers()
{
  return static_cast<unsigned int>(m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size());
}

void mitk::VtkLayerController::SetEraseForAllRenderers(int i)
{
  this->m_RenderWindow->SetErase(i);

  RendererVectorType::iterator it;
  for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
    (*it)->SetErase(i);

  for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
    (*it)->SetErase(i);

  for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
    (*it)->SetErase(i);
}
