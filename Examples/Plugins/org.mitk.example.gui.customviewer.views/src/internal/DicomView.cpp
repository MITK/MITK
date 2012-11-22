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

#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

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
  QDockWidget* searchWidget = parent->findChild<QDockWidget*>("ExternalSearchDockWidget");
  searchWidget->setVisible(false);

  connect(m_Controls.importButton, SIGNAL(clicked()), m_Controls.widget, SLOT(OnFolderCDImport()));
  connect(m_Controls.widget, SIGNAL(SignalDicomToDataManager(const QStringList&)), this, SLOT(AddDataNodeFromDICOM(const QStringList&)));

  m_Parent->setEnabled(true);
}
// //! [DicomViewCreatePartControl]

// //! [DicomViewCreateAddDataNodeInformation]
void DicomView::AddDataNodeFromDICOM(const QStringList& Properties)
{
  QString seriesUID = Properties.at(3);
  QString path = Properties.at(5);
// //! [DicomViewCreateAddDataNodeInformation]
// //! [DicomViewCreateAddDataNodeLoadSeries]
  mitk::DicomSeriesReader::StringContainer seriesToLoad;
  std::size_t found;

  mitk::DicomSeriesReader::FileNamesGrouping dicomSeriesMap = mitk::DicomSeriesReader::GetSeries(path.toStdString(),false);
  mitk::DicomSeriesReader::FileNamesGrouping::const_iterator qualifiedSeriesInstanceUIDIterator;

  for(qualifiedSeriesInstanceUIDIterator = dicomSeriesMap.begin();
      qualifiedSeriesInstanceUIDIterator != dicomSeriesMap.end();
      ++qualifiedSeriesInstanceUIDIterator)
  {
      found = qualifiedSeriesInstanceUIDIterator->second.GetSeriesInstanceUID().find(seriesUID.toStdString());
      if(found != std::string::npos)
      {
          seriesToLoad = qualifiedSeriesInstanceUIDIterator->second.GetFilenames();
      }
  }

  mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
// //! [DicomViewCreateAddDataNodeLoadSeries]
  if (node.IsNull())
  {
      MITK_ERROR << "Could not load series: " << seriesUID.toStdString();
  }
  else
  {
// //! [DicomViewCreateAddDataNode]
      this->GetDataStorage()->Add(node);
// //! [DicomViewCreateAddDataNode]
      mitk::RenderingManager::GetInstance()->SetDataStorage(this->GetDataStorage());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

// //! [DicomViewCreateAddDataNodeActivatePersp]
      berry::IWorkbenchWindow::Pointer window = this->GetSite()->GetWorkbenchWindow();
      std::string perspectiveId = "org.mitk.example.viewerperspective";
      window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
// //! [DicomViewCreateAddDataNodeActivatePersp]
  }
}

void DicomView::SetFocus ()
{
}
