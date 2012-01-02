/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 17:34:48 +0200 (Wed, 31 Mar 2010) $
Version:   $Revision: 21985 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRenderingTestHelper.h"
#include "mitkStandaloneDataStorage.h"

#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkRenderLargeImage.h>
#include <mitkRenderWindow.h>

#include <mitkGlobalInteraction.h>
#include <vtkRenderWindowInteractor.h>
#include <mitkSliceNavigationController.h>
#include <mitkNodePredicateDataType.h>
#include <mitkLevelWindowProperty.h>

mitkRenderingTestHelper::mitkRenderingTestHelper(int width, int height, mitk::DataStorage *ds)
{
  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  m_RenderWindow = mitk::RenderWindow::New();
  m_RenderWindow->GetRenderer()->SetDataStorage(ds);
  m_RenderWindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
  this->GetVtkRenderWindow()->SetSize( width, height );

  mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow())->GetSliceNavigationController()->SetDefaultViewDirection(mitk::SliceNavigationController::Transversal);
  mitk::TimeSlicedGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());

  //Level Window Property
//  ds->GetNode(mitk::NodePredicateDataType::New("Image"))->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(254.0, 1.0) ) );

  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(m_RenderWindow->GetVtkRenderWindow());

//use this to actually show the iamge in a renderwindow
//  this->GetVtkRenderWindow()->Render();
//  this->GetVtkRenderWindow()->GetInteractor()->Start();
}

mitkRenderingTestHelper::~mitkRenderingTestHelper()
{
}

vtkRenderer* mitkRenderingTestHelper::GetVtkRenderer()
{
  return m_RenderWindow->GetRenderer()->GetVtkRenderer();
}

vtkRenderWindow* mitkRenderingTestHelper::GetVtkRenderWindow()
{
  return m_RenderWindow->GetVtkRenderWindow();
}

//method to save a screenshot of the renderwindow (e.g. create a reference screenshot)
void mitkRenderingTestHelper::SaveAsPNG(std::string fileName)
{
  vtkSmartPointer<vtkRenderer> renderer = this->GetVtkRenderer();
  bool doubleBuffering( renderer->GetRenderWindow()->GetDoubleBuffer() );
  renderer->GetRenderWindow()->DoubleBufferOff();

  vtkSmartPointer<vtkRenderLargeImage> magnifier = vtkSmartPointer<vtkRenderLargeImage>::New();
  magnifier->SetInput(renderer);
  magnifier->SetMagnification(1.0);

  vtkSmartPointer<vtkImageWriter> fileWriter = vtkSmartPointer<vtkPNGWriter>::New();
  fileWriter->SetInput(magnifier->GetOutput());
  fileWriter->SetFileName(fileName.c_str());

  fileWriter->Write();
  renderer->GetRenderWindow()->SetDoubleBuffer(doubleBuffering);
}

