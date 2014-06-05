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

#include "DicomView.h"

#include "org_mitk_example_gui_customviewer_views_Activator.h"

#include "mitkIDataStorageService.h"
#include "mitkDicomSeriesReader.h"
#include "mitkImage.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

#include "QDockWidget"

const std::string DicomView::VIEW_ID = "org.mitk.customviewer.views.dicomview";

DicomView::DicomView()
  : m_Parent(0)
{
}

DicomView::~DicomView()
{
}

// //! [DicomViewCreatePartControl]
void DicomView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //remove unused widgets
  QPushButton* downloadButton = parent->findChild<QPushButton*>("downloadButton");
  downloadButton->setVisible(false);

  connect(m_Controls.importButton, SIGNAL(clicked()), m_Controls.widget, SLOT(OnFolderCDImport()));
  connect(m_Controls.widget, SIGNAL(SignalDicomToDataManager(const QHash<QString,QVariant>&)), this, SLOT(AddDataNodeFromDICOM(const QHash<QString,QVariant>&)));

  m_Parent->setEnabled(true);
}
// //! [DicomViewCreatePartControl]

// //! [DicomViewCreateAddDataNodeInformation]
void DicomView::AddDataNodeFromDICOM( QHash<QString,QVariant> eventProperties)
{
  QStringList listOfFilesForSeries;
  mitk::DicomSeriesReader::StringContainer seriesToLoad;

  listOfFilesForSeries = eventProperties["FilesForSeries"].toStringList();

  if (!listOfFilesForSeries.isEmpty()){

      QStringListIterator it(listOfFilesForSeries);

      while (it.hasNext())
      {
          seriesToLoad.push_back(it.next().toStdString());
      }

      mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
      if (node.IsNull())
      {
          MITK_ERROR << "Error loading Dicom series";
      }
      // //! [DicomViewCreateAddDataNodeLoadSeries]
      else
      {
      // //! [DicomViewCreateAddDataNode]
            mitk::DataStorage::Pointer ds = this->GetDataStorage();
            ds->Add(node);
      // //! [DicomViewCreateAddDataNode]
            mitk::RenderingManager::GetInstance()->SetDataStorage(ds);
            mitk::TimeGeometry::Pointer geometry = ds->ComputeBoundingGeometry3D(ds->GetAll());
            mitk::RenderingManager::GetInstance()->InitializeViews(geometry);

      // //! [DicomViewCreateAddDataNodeActivatePersp]
            berry::IWorkbenchWindow::Pointer window = this->GetSite()->GetWorkbenchWindow();
            std::string perspectiveId = "org.mitk.example.viewerperspective";
            window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      // //! [DicomViewCreateAddDataNodeActivatePersp]
     }
  }
}

void DicomView::SetFocus ()
{
}
