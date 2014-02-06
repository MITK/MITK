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

//VTK
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkRenderLargeImage.h>
#include <vtkRenderWindowInteractor.h>

//MITK
#include <mitkInteractionTestHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkRenderWindow.h>
#include <mitkGlobalInteraction.h>
#include <mitkSliceNavigationController.h>
#include <mitkNodePredicateDataType.h>
#include <mitkIOUtil.h>

// include gl to read out properties
#include <vtkOpenGL.h>
#include <vtkOpenGLExtensionManager.h>


#if defined _MSC_VER
#if _MSC_VER >= 1700
#define RESIZE_WORKAROUND
#endif
#endif

#ifdef RESIZE_WORKAROUND
#include "vtkWin32OpenGLRenderWindow.h"
#endif


mitk::InteractionTestHelper::InteractionTestHelper(int width, int height, std::string interactionFilePath)
  : m_AutomaticallyCloseRenderWindow(true)
{
  this->Initialize(width, height, interactionFilePath);
}

void mitk::InteractionTestHelper::Initialize(int width, int height, std::string interactionFilePath)
{
  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  m_RenderWindow = mitk::RenderWindow::New();
  m_DataStorage = mitk::StandaloneDataStorage::New();

  m_RenderWindow->GetRenderer()->SetDataStorage(m_DataStorage);
  this->SetMapperIDToRender2D();
  this->GetVtkRenderWindow()->SetSize( width, height );

#ifdef RESIZE_WORKAROUND

  HWND hWnd = static_cast<vtkWin32OpenGLRenderWindow*>(this->GetVtkRenderWindow())->GetWindowId();

  RECT r;
  r.left      = 10;
  r.top       = 10;
  r.right     = r.left + width;
  r.bottom    = r.top + height;

  LONG style = GetWindowLong(hWnd, GWL_STYLE);
  AdjustWindowRect( &r, style, FALSE );

  MITK_INFO << "WANTED:";
  MITK_INFO << r.right-r.left;
  MITK_INFO << r.bottom-r.top;

  RECT rect;
if(GetWindowRect(hWnd, &rect))
{
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  MITK_INFO << "ACTUAL:";
  MITK_INFO << width;
  MITK_INFO <<height;
}

  SetWindowPos( hWnd
            , HWND_TOP, 0, 0, r.right-r.left, r.bottom-r.top,
                       SWP_NOZORDER);

  GetWindowRect(hWnd, &rect);

  int width2 = rect.right - rect.left;
  int height2 = rect.bottom - rect.top;

  MITK_INFO << "ACTUAL2:";
  MITK_INFO << width2;
  MITK_INFO << height2;

    SetWindowPos( hWnd
            , HWND_TOP, 0, 0, 2*(r.right-r.left) - width2 ,2*(r.bottom-r.top) - height2,
                        SWP_NOZORDER);

#endif

  m_RenderWindow->GetRenderer()->Resize( width, height);

  //load interaction pattern
  std::ifstream xmlStream(interactionFilePath.c_str());
  mitk::XML2EventParser parser(xmlStream);
  m_Events = parser.GetInteractions();
  xmlStream.close();
}

mitk::InteractionTestHelper::~InteractionTestHelper()
{
}


void mitk::InteractionTestHelper::SetMapperID( mitk::BaseRenderer::StandardMapperSlot id)
{
  m_RenderWindow->GetRenderer()->SetMapperID(id);
}


void mitk::InteractionTestHelper::SetMapperIDToRender3D()
{
  this->SetMapperID(mitk::BaseRenderer::Standard3D);
}


void mitk::InteractionTestHelper::SetMapperIDToRender2D()
{
  this->SetMapperID(mitk::BaseRenderer::Standard2D);
}


//void mitk::InteractionTestHelper::Render()
//{
//  //if the datastorage is initialized and at least 1 image is loaded render it
//  if(m_DataStorage.IsNotNull() || m_DataStorage->GetAll()->Size() >= 1 )
//  {
//    //Prepare the VTK camera before rendering.
//    m_RenderWindow->GetRenderer()->PrepareRender();

//    this->GetVtkRenderWindow()->Render();
//    if(m_AutomaticallyCloseRenderWindow == false)
//    {
//      //Use interaction to stop the test
//      this->GetVtkRenderWindow()->GetInteractor()->Start();
//    }
//  }
//  else
//  {
//    MITK_ERROR << "No images loaded in data storage!";
//  }
//}


mitk::DataStorage::Pointer mitk::InteractionTestHelper::GetDataStorage()
{
  return m_DataStorage;
}


void mitk::InteractionTestHelper::SetViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection)
{
  mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow())->GetSliceNavigationController()->SetDefaultViewDirection(viewDirection);
  mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
}


mitk::BaseRenderer* mitk::InteractionTestHelper::GetRenderer()
{
  return m_RenderWindow->GetRenderer();
}


mitk::RenderWindow* mitk::InteractionTestHelper::GetRenderWindow()
{
  return m_RenderWindow;
}


void mitk::InteractionTestHelper::SetAutomaticallyCloseRenderWindow(bool automaticallyCloseRenderWindow)
{
  m_AutomaticallyCloseRenderWindow = automaticallyCloseRenderWindow;
}


void mitk::InteractionTestHelper::AddToStorage(const std::string &filename)
{
  try
  {
    mitk::DataNode::Pointer node = mitk::IOUtil::LoadDataNode(filename);
    this->AddNodeToStorage(node);
  }
  catch ( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Failed loading test data '" << filename << "': " << e.what();
  }
}


void mitk::InteractionTestHelper::AddNodeToStorage(mitk::DataNode::Pointer node)
{
  this->m_DataStorage->Add(node);
  mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
}


void mitk::InteractionTestHelper::PlaybackInteraction()
{
  for (int i=0; i < m_Events.size(); ++i)
  {
    m_Events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(m_Events.at(i));
  }
}
