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

#include "mitkVtkLayerController.h"

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <algorithm>

typedef std::map<const vtkRenderWindow*,mitk::VtkLayerController*> vtkLayerControllerMapType;
static vtkLayerControllerMapType vtkLayerControllerMap;

mitk::VtkLayerController * mitk::VtkLayerController::GetInstance(vtkRenderWindow* renWin)
{
  for(vtkLayerControllerMapType::iterator mapit = vtkLayerControllerMap.begin(); 
      mapit != vtkLayerControllerMap.end(); mapit++)
  {
    if( (*mapit).first == renWin)
      return (*mapit).second;

  }
  return NULL;
}

void  mitk::VtkLayerController::AddInstance(vtkRenderWindow* renWin, vtkRenderer * mitkSceneRenderer)
{
  // ensure that no vtkRenderWindow is managed twice
  mitk::VtkLayerController::RemoveInstance(renWin);

  // instanciate controller, add it to the map
  mitk::VtkLayerController* ControllerInstance = new mitk::VtkLayerController(renWin);
  ControllerInstance->InsertSceneRenderer(mitkSceneRenderer);

  vtkLayerControllerMap.insert(vtkLayerControllerMapType::value_type(renWin,ControllerInstance));
}     

void  mitk::VtkLayerController::RemoveInstance(vtkRenderWindow* renWin)
{
  vtkLayerControllerMapType::iterator mapit = vtkLayerControllerMap.find(renWin);
  if(mapit != vtkLayerControllerMap.end())
    vtkLayerControllerMap.erase(mapit);
}

mitk::VtkLayerController::VtkLayerController(vtkRenderWindow* renderWindow)
{
  m_RenderWindow = renderWindow;
  m_BackgroundRenderers.clear();
  m_ForegroundRenderers.clear();
  m_SceneRenderers.clear();

}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered in the background.
 * With forceAbsoluteBackground set true a renderer can be placed at the absolute background of the scene.
 * Multiple calls with forceAbsoluteBackground set true will set the latest registered renderer as background.
 */
void mitk::VtkLayerController::InsertBackgroundRenderer(vtkRenderer* renderer, bool forceAbsoluteBackground)
{
  
  if(renderer == NULL)
    return;
  
  // Remove renderer if it already exists 
  RemoveRenderer(renderer);

  if(forceAbsoluteBackground)
  {
    RendererVectorType::iterator it = m_BackgroundRenderers.begin();
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
void mitk::VtkLayerController::InsertForegroundRenderer(vtkRenderer* renderer, bool forceAbsoluteForeground)
{
  
  if(renderer == NULL)
    return;
  
  // Remove renderer if it already exists 
  RemoveRenderer(renderer);

  if(forceAbsoluteForeground)
  {
    RendererVectorType::iterator it = m_ForegroundRenderers.begin();
    m_ForegroundRenderers.insert(it,renderer);
  }
  else
    m_ForegroundRenderers.push_back(renderer);

  UpdateLayers();
}
/**
 * Returns the Scene Renderer
 */
vtkRenderer* mitk::VtkLayerController::GetSceneRenderer()
{
  if(m_SceneRenderers.size() > 0)
  {
    RendererVectorType::iterator it = m_SceneRenderers.begin();
    return (*it);
  }
  else
    return NULL;
}
/**
 * Connects a VTK renderer with a vtk renderwindow. The renderer will be rendered between background renderers and
 * foreground renderers.
 */
void mitk::VtkLayerController::InsertSceneRenderer(vtkRenderer* renderer)
{
  
  if(renderer == NULL)
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
void mitk::VtkLayerController::RemoveRenderer(vtkRenderer* renderer)
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
void mitk::VtkLayerController::SetRenderWindow(vtkRenderWindow* renwin)
{
  if(renwin != NULL)
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
bool mitk::VtkLayerController::IsRendererInserted(vtkRenderer* renderer)
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
  vtkRendererCollection* v = m_RenderWindow->GetRenderers();
  v->RemoveAllItems();


  int numberOfLayers = m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size();
  int currentLayerNumber;
  bool traverseUpwards;

  #if ( VTK_MAJOR_VERSION >= 5 )
    currentLayerNumber  = 0;
    traverseUpwards     = true;
  #else
    currentLayerNumber  = numberOfLayers - 1;
    traverseUpwards     = false;
  #endif
  
  
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
int mitk::VtkLayerController::GetNumberOfRenderers()
{
  return m_BackgroundRenderers.size() + m_SceneRenderers.size() + m_ForegroundRenderers.size();
}

void mitk::VtkLayerController::SetEraseForAllRenderers(int i)
{
  #if ( VTK_MAJOR_VERSION >= 5 )
  
  this->m_RenderWindow->SetErase(i);

  RendererVectorType::iterator it;
  for(it = m_BackgroundRenderers.begin(); it != m_BackgroundRenderers.end(); it++)
    (*it)->SetErase(i);
  
  for(it = m_SceneRenderers.begin(); it != m_SceneRenderers.end(); it++)
   (*it)->SetErase(i);

  for(it = m_ForegroundRenderers.begin(); it != m_ForegroundRenderers.end(); it++)
   (*it)->SetErase(i);
  
  #endif
}

