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
#include <vtkCamera.h>

//MITK
#include <mitkRenderingTestHelper.h>
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

//VTK Testing to compare the rendered image pixel-wise against a reference screen shot
#include "vtkTesting.h"

mitk::RenderingTestHelper::RenderingTestHelper(int width, int height, mitk::BaseRenderer::RenderingMode::Type renderingMode)
  : m_AutomaticallyCloseRenderWindow(true)
{
  this->Initialize(width, height, renderingMode );
}

mitk::RenderingTestHelper::RenderingTestHelper(int width, int height, int argc, char* argv[], mitk::BaseRenderer::RenderingMode::Type renderingMode)
  : m_AutomaticallyCloseRenderWindow(true)
{
  this->Initialize(width, height, renderingMode);
  this->SetInputFileNames(argc, argv);
}

void mitk::RenderingTestHelper::Initialize(int width, int height, mitk::BaseRenderer::RenderingMode::Type renderingMode)
{
  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  m_RenderWindow = mitk::RenderWindow::New(NULL ,"unnamed renderer" , NULL, renderingMode);

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

  //Prints the glinfo after creation of the vtkrenderwindow, we always want to do this for debugging.
  this->PrintGLInfo();
}

mitk::RenderingTestHelper::~RenderingTestHelper()
{
}

bool  mitk::RenderingTestHelper::IsAdvancedOpenGL()
{
  const GLubyte *version = glGetString(GL_VERSION);
  if(!version)
    return false;
  return *version >= '2';
}

void mitk::RenderingTestHelper::PrintGLInfo()
{
  GLint maxTextureSize;

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);;

  MITK_INFO << "OpenGL Render Context Information: \n"
            << "- GL_VENDOR: "<< glGetString(GL_VENDOR) << "\n"
            << "- GL_RENDERER: "<< glGetString(GL_RENDERER) << "\n"
            << "- GL_VERSION: "<< glGetString(GL_VERSION)   << "\n"
            << "- GL_MAX_TEXTURE_SIZE: "<< maxTextureSize << "\n"
            << "- GL_EXTENSIONS: "<< glGetString(GL_EXTENSIONS);
}

void mitk::RenderingTestHelper::SetMapperID( mitk::BaseRenderer::StandardMapperSlot id)
{
  m_RenderWindow->GetRenderer()->SetMapperID(id);
}

void mitk::RenderingTestHelper::SetMapperIDToRender3D()
{
  this->SetMapperID(mitk::BaseRenderer::Standard3D);
   mitk::RenderingManager::GetInstance()->InitializeViews( this->GetDataStorage()->ComputeBoundingGeometry3D( this->GetDataStorage()->GetAll() ) );
}

void mitk::RenderingTestHelper::SetMapperIDToRender2D()
{
  this->SetMapperID(mitk::BaseRenderer::Standard2D);
}

void mitk::RenderingTestHelper::Render()
{
  //if the datastorage is initialized and at least 1 image is loaded render it
  if(m_DataStorage.IsNotNull() || m_DataStorage->GetAll()->Size() >= 1 )
  {
    //Prepare the VTK camera before rendering.
    m_RenderWindow->GetRenderer()->PrepareRender();

    this->GetVtkRenderWindow()->Render();
    if(m_AutomaticallyCloseRenderWindow == false)
    {
      //Use interaction to stop the test
      this->GetVtkRenderWindow()->GetInteractor()->Start();
    }
  }
  else
  {
    MITK_ERROR << "No images loaded in data storage!";
  }
}

mitk::DataStorage::Pointer mitk::RenderingTestHelper::GetDataStorage()
{
  return m_DataStorage;
}

void mitk::RenderingTestHelper::SetInputFileNames(int argc, char* argv[])
{
  //i is set 1, because 0 is the testname as string
  //parse parameters
  for (int i = 1; i < argc; ++i)
  {
    //add everything to a list but -T and -V
    std::string tmp = argv[i];
    if((tmp.compare("-T")) && (tmp.compare("-V")))
    {
      this->AddToStorage(tmp);
    }
    else
    {
      break;
    }
  }
}

void mitk::RenderingTestHelper::SetViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection)
{
  mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow())->GetSliceNavigationController()->SetDefaultViewDirection(viewDirection);
  mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
}

void mitk::RenderingTestHelper::ReorientSlices(mitk::Point3D origin, mitk::Vector3D rotation)
{
  mitk::SliceNavigationController::Pointer sliceNavigationController =
      mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow())->GetSliceNavigationController();
  sliceNavigationController->ReorientSlices(origin, rotation);
}

vtkRenderer* mitk::RenderingTestHelper::GetVtkRenderer()
{
  return m_RenderWindow->GetRenderer()->GetVtkRenderer();
}

void mitk::RenderingTestHelper::SetImageProperty(const char *propertyKey, mitk::BaseProperty* property )
{
  this->m_DataStorage->GetNode(mitk::NodePredicateDataType::New("Image"))->SetProperty(propertyKey, property);
}

vtkRenderWindow* mitk::RenderingTestHelper::GetVtkRenderWindow()
{
  return m_RenderWindow->GetVtkRenderWindow();
}

bool mitk::RenderingTestHelper::CompareRenderWindowAgainstReference(int argc, char* argv[], double threshold)
{
  this->Render();
  m_RenderWindow->GetRenderer()->ForceImmediateUpdate();
  //retVal meanings: (see VTK/Rendering/vtkTesting.h)
  //0 = test failed
  //1 = test passed
  //2 = test not run
  //3 = something with vtkInteraction
  if(vtkTesting::Test(argc, argv, this->GetVtkRenderWindow(), threshold) == 1)
    return true;
  else
    return false;
}

//method to save a screenshot of the renderwindow (e.g. create a reference screenshot)
void mitk::RenderingTestHelper::SaveAsPNG(std::string fileName)
{
  vtkSmartPointer<vtkRenderer> renderer = this->GetVtkRenderer();
  bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
  renderer->GetRenderWindow()->DoubleBufferOff();

  vtkSmartPointer<vtkRenderLargeImage> magnifier = vtkSmartPointer<vtkRenderLargeImage>::New();
  magnifier->SetInput(renderer);
  magnifier->SetMagnification(1);

  vtkSmartPointer<vtkImageWriter> fileWriter = vtkSmartPointer<vtkPNGWriter>::New();
  fileWriter->SetInputConnection(magnifier->GetOutputPort());
  fileWriter->SetFileName(fileName.c_str());

  fileWriter->Write();
  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

void mitk::RenderingTestHelper::SetAutomaticallyCloseRenderWindow(bool automaticallyCloseRenderWindow)
{
  m_AutomaticallyCloseRenderWindow = automaticallyCloseRenderWindow;
}

void mitk::RenderingTestHelper::SaveReferenceScreenShot(std::string fileName)
{
  this->SaveAsPNG(fileName);
}

void mitk::RenderingTestHelper::AddToStorage(const std::string &filename)
{
  try
  {
    mitk::IOUtil::Load(filename, *m_DataStorage.GetPointer());
    mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
  }
  catch ( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Failed loading test data '" << filename << "': " << e.what();
  }
}

void mitk::RenderingTestHelper::AddNodeToStorage(mitk::DataNode::Pointer node)
{
  this->m_DataStorage->Add(node);
  mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
}
