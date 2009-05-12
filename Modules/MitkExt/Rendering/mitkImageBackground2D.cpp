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

#include "mitkImageBackground2D.h"
// MITK includes
#include "mitkVtkLayerController.h"

// VTK includes
#include "vtkSystemIncludes.h"
#include "vtkRenderer.h"
#include "vtkMapper.h"
#include "vtkObjectFactory.h"
#include "vtkImageActor.h"
#include "vtkRenderWindow.h"
#include "vtkImageImport.h"
#include "vtkCommand.h"
#include "vtkCamera.h"

#include <iostream>

// VTK CALLBACK for automatic resize of the video
class vtkVideoSizeCallback : public vtkCommand
{
public:
  static vtkVideoSizeCallback *New(){ return new vtkVideoSizeCallback; }
  
  vtkRenderer * m_ImageRenderer;
  int m_ImageWidth, m_ImageHeight;

  void SetVtkVideoRenderer(vtkRenderer* r)
  {
    m_ImageRenderer = r;
  }
  void SetVideoDimensions(int x, int y)
  {
    m_ImageWidth = x; m_ImageHeight = y;
  }

  virtual void Execute(vtkObject* /*caller*/, unsigned long, void*)
  {
    //vtkRenderWindow * RenderWindow = reinterpret_cast<vtkRenderWindow*>(caller);
    //m_ImageRenderer->ResetCameraClippingRange();
  }
};

mitk::ImageBackground2D::ImageBackground2D()
{
  m_ImageWidth              = 0;
  m_ImageHeight             = 0;
  m_ImageScalarComponents   = 3;
  m_ParallelScale           = 0;
  m_RenderWindow = NULL;
  
  m_Actor             = vtkImageActor::New();
  m_ImageRenderer     = vtkRenderer::New();
  m_VtkImageImport    = vtkImageImport::New();
  
  m_ImageData         = NULL;
}

void mitk::ImageBackground2D::InitVtkImageImport()
{ 
  /*m_VtkImageImport->Delete();
  m_VtkImageImport  = vtkImageImport::New();*/
  m_VtkImageImport->SetDataScalarTypeToUnsignedChar();
  m_VtkImageImport->SetNumberOfScalarComponents(m_ImageScalarComponents);
  m_VtkImageImport->SetWholeExtent(0,m_ImageWidth-1,0,m_ImageHeight-1,0,1-1);
  m_VtkImageImport->SetDataExtentToWholeExtent();  
}

mitk::ImageBackground2D::~ImageBackground2D()
{
  if ( m_RenderWindow != NULL )
    if ( this->IsEnabled() )
      this->Disable();
  if ( m_Actor!=NULL )
    m_Actor->Delete();
  if ( m_ImageRenderer != NULL )
    m_ImageRenderer->Delete();
  if ( m_VtkImageImport != NULL)
    m_VtkImageImport->Delete();
}
/**
 * Sets the renderwindow, in which the Video background
 * will be shown. Make sure, you have called this function
 * before calling Enable()
 */
void mitk::ImageBackground2D::SetRenderWindow(vtkRenderWindow* renderWindow )
{
  m_RenderWindow = renderWindow;

  /*m_SizeCallback = vtkVideoSizeCallback::New();
  m_RenderWindow->GetVtkRenderWindow()->AddObserver(vtkCommand::ModifiedEvent,m_SizeCallback);*/
  
}

void mitk::ImageBackground2D::SetParallelScale(int scale)
{
  m_ParallelScale = scale;
  m_ImageRenderer->GetActiveCamera()->SetParallelScale(m_ParallelScale);
}
int mitk::ImageBackground2D::GetParallelScale()
{
  // TODO someone who knows this, check if double->int could cause problems
  return static_cast<int>(m_ImageRenderer->GetActiveCamera()->GetParallelScale());
}
/**
 * Enables drawing of the color Video background.
 * If you want to disable it, call the Disable() function.
 */
void mitk::ImageBackground2D::Enable()
{
  m_ImageRenderer = vtkRenderer::New();

  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertBackgroundRenderer(m_ImageRenderer,true);
     
  char * c = 0;
  Update(c);
  m_Actor->SetInput(m_VtkImageImport->GetOutput());
 
  m_ImageRenderer->AddActor2D(m_Actor);
  m_ImageRenderer->ResetCamera();
  m_ImageRenderer->InteractiveOff();
  m_ImageRenderer->GetActiveCamera()->ParallelProjectionOn();
  if(m_ParallelScale == 0)
    m_ImageRenderer->GetActiveCamera()->SetParallelScale(m_ImageHeight/2);
 
  //m_SizeCallback->SetVtkVideoRenderer(m_ImageRenderer);
  //m_SizeCallback->SetVideoDimensions(m_ImageWidth, m_ImageHeight);
   
}

/**
 * Disables drawing of the color Video background.
 * If you want to enable it, call the Enable() function.
 */
void mitk::ImageBackground2D::Disable()
{
  if ( this->IsEnabled() )
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->RemoveRenderer(m_ImageRenderer);        
}
/**
 * Checks, if the Video background is currently
 * enabled (visible)
 */
bool mitk::ImageBackground2D::IsEnabled()
{
  if ( mitk::VtkLayerController::GetInstance(m_RenderWindow)->IsRendererInserted(m_ImageRenderer)) 
      return true;
  else
      return false;    
}

void mitk::ImageBackground2D::Update(char * dataPointer)
{
  if(!IsEnabled())
    return;

  // VTK import image data must be allocated before import (with correct parameters)
  if(m_ImageData == NULL)
    m_ImageData = new unsigned char[m_ImageHeight*m_ImageWidth*m_ImageScalarComponents];

  if(!dataPointer)
  {// no valid image pointer provided -> initialize black image
    unsigned char* b = m_ImageData;
    for ( int textCounter = 0; textCounter < (m_ImageHeight*m_ImageWidth*m_ImageScalarComponents); textCounter++ )
      *b++ = 0;
  }
  
  m_VtkImageImport->SetImportVoidPointer(m_ImageData);
  m_VtkImageImport->Modified();
  m_VtkImageImport->Update();
  m_RenderWindow->Render();
   
}

void mitk::ImageBackground2D::Update(char * dataPointer, int width, int height, int imageScalarComponents)
{
  // no image-backround layer is rendered
  if(!IsEnabled())
    return;

  // image contains no data, OR image scalar components missmatch (no RGB, no greyscale)
  if(!dataPointer || (imageScalarComponents != 1 && imageScalarComponents != 3))
  {
    // no valid image pointer provided -> we initialize black image
    if(m_ImageData == NULL)
      m_ImageData = new unsigned char[m_ImageHeight*m_ImageWidth*m_ImageScalarComponents];
    
    unsigned char* b = m_ImageData;
    for ( int textCounter = 0; textCounter < (m_ImageHeight*m_ImageWidth*m_ImageScalarComponents); textCounter++ )
      *b++ = 0;
  }
  else
  {
    if(m_ImageWidth == 0)
    { // VTK Image Import has to be initialized properly
      m_ImageHeight = height;
      m_ImageWidth  = width;
      m_ImageScalarComponents = imageScalarComponents;
      InitVtkImageImport();
    }
    

    // VTK import image data must be allocated before import (with correct parameters)
    if(m_ImageData == NULL)
      m_ImageData = new unsigned char[m_ImageHeight*m_ImageWidth*m_ImageScalarComponents];
    
    int column, row;
    unsigned char* tex  = m_ImageData;
    char*          data = dataPointer;

    // PREPARE image Data for VTKImageImport Filter //
    
     
    if(imageScalarComponents == 1)
    {
      m_VtkImageImport->SetNumberOfScalarComponents(m_ImageScalarComponents);

      unsigned char g;
      for (column = 0; column < m_ImageHeight; column++)
        for (row = 0; row < m_ImageWidth; row++)
        {
              g   = *data++;
              *tex++  = g;
        }      
    }
    else if(imageScalarComponents == 3)
    {
      // flip image, change color channels
      unsigned char r, g, b;
      for (column = 0; column < m_ImageHeight; column++)
        for (row = 0; row < m_ImageWidth; row++)
        {   //change r with b
            b = *data++;
            g = *data++;
            r = *data++;
            *tex++ = r;
            *tex++ = g;
            *tex++ = b;
          }        
    } // else if(imageScalarComponents == 3)
    
  } // end else image data present


  // Initialize VTK-ImageImport Filter //
  m_VtkImageImport->SetImportVoidPointer(m_ImageData);
  m_VtkImageImport->Modified();
  m_VtkImageImport->Update();
  m_RenderWindow->Render();
   
}


