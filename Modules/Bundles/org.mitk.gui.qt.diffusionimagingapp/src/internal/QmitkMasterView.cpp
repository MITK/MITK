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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryIWorkbench.h>
#include <internal/berryQtOpenPerspectiveAction.h>

// Qmitk
#include "QmitkMasterView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>


const std::string QmitkMasterView::VIEW_ID = "org.mitk.views.masterview";

QmitkMasterView::QmitkMasterView()
    : QmitkFunctionality()
{
}

QmitkMasterView::~QmitkMasterView()
{
}

void QmitkMasterView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  CreateConnections();

  berry::IPerspectiveRegistry* perspRegistry = GetSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->GetPerspectiveRegistry();
  std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
  for (std::vector<berry::IPerspectiveDescriptor::Pointer>::iterator perspIt = perspectives.begin();
      perspIt != perspectives.end(); ++perspIt)
  {
    std::string perpid=((*perspIt)->GetId());
    MITK_INFO << perpid;

  }
}

void QmitkMasterView::CreateConnections()
{
  connect( (QObject*)(m_Controls.m_OpenDicomImport), SIGNAL(clicked()),(QObject*) this, SLOT(OpenDicomImport()));
  connect( (QObject*)(m_Controls.m_OpenPreprocessingReconstruction), SIGNAL(clicked()),(QObject*) this, SLOT(OpenPreprocessingReconstruction()));
  connect( (QObject*)(m_Controls.m_OpenQuantification), SIGNAL(clicked()),(QObject*) this, SLOT(OpenQuantification()));
  connect( (QObject*)(m_Controls.m_OpenTractography), SIGNAL(clicked()),(QObject*) this, SLOT(OpenTractography()));
  connect( (QObject*)(m_Controls.m_OpenTBSS), SIGNAL(clicked()),(QObject*) this, SLOT(OpenTBSS()));
  connect( (QObject*)(m_Controls.m_OpenConnectomics), SIGNAL(clicked()),(QObject*) this, SLOT(OpenConnectomics()));
  connect( (QObject*)(m_Controls.m_OpenIVIM), SIGNAL(clicked()),(QObject*) this, SLOT(OpenIVIM()));
  connect( (QObject*)(m_Controls.m_OpenVolumeVisualization), SIGNAL(clicked()),(QObject*) this, SLOT(OpenVolumeVisualization()));
  connect( (QObject*)(m_Controls.m_OpenScreenshotsMovies), SIGNAL(clicked()),(QObject*) this, SLOT(OpenScreenshotsMovies()));
}

void QmitkMasterView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{ 
}

void QmitkMasterView::OpenPerspective(std::string id)
{
  try
  {
    GetSite()->GetPage()->GetWorkbenchWindow()->GetWorkbench()->ShowPerspective(id, berry::IWorkbenchWindow::Pointer(GetSite()->GetPage()->GetWorkbenchWindow()));
  }
  catch (...)
  {
    QMessageBox::critical(0, "Opening Perspective Failed", QString("The perspective \"") + id.c_str() + "\" could not be opened.\nSee the log for details.");
  }
}

void QmitkMasterView::OpenDicomImport()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.dicomimport");
}

void QmitkMasterView::OpenPreprocessingReconstruction()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.preprocessingreconstruction");
}

void QmitkMasterView::OpenQuantification()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.quantification");
}

void QmitkMasterView::OpenTractography()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.tractography");
}

void QmitkMasterView::OpenTBSS()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.tbss");
}

void QmitkMasterView::OpenConnectomics()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.connectomics");
}

void QmitkMasterView::OpenIVIM()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.ivim");
}

void QmitkMasterView::OpenVolumeVisualization()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.volumevisualization");
}

void QmitkMasterView::OpenScreenshotsMovies()
{
  OpenPerspective("org.mitk.diffusionimagingapp.perspectives.screenshotsmovies");
}

