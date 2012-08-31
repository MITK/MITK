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

//#include <QMessageBox>

#include "org_mitk_example_gui_views_Activator.h"

#include "mitkIDataStorageService.h"
#include "mitkDicomSeriesReader.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>

const std::string DicomView::VIEW_ID = "org.mitk.views.dicomview"; 

DicomView::DicomView()
  : m_Parent(0)
{
}

DicomView::~DicomView()
{
}

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

void DicomView::AddDataNodeFromDICOM(const QStringList& Properties)
{
  QString seriesUID = Properties.at(3);
  QString path = Properties.at(5);
  
  std::list<std::string> qualifiedUIDs;
  mitk::DicomSeriesReader::StringContainer seriesToLoad;
  std::size_t found;

  mitk::DicomSeriesReader::UidFileNamesMap dicomSeriesMap = mitk::DicomSeriesReader::GetSeries(path.toStdString(),false);
  mitk::DicomSeriesReader::UidFileNamesMap::const_iterator qualifiedSeriesInstanceUIDIterator;

  for(qualifiedSeriesInstanceUIDIterator = dicomSeriesMap.begin();
      qualifiedSeriesInstanceUIDIterator != dicomSeriesMap.end();
      ++qualifiedSeriesInstanceUIDIterator)
  {
      found = qualifiedSeriesInstanceUIDIterator->first.find(seriesUID.toStdString());
      if(found!= qualifiedSeriesInstanceUIDIterator->first.npos)
      {
          qualifiedUIDs.push_back(qualifiedSeriesInstanceUIDIterator->first); 
          seriesToLoad = qualifiedSeriesInstanceUIDIterator->second;
      }
  }

  mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad);
  if (node.IsNull())
  {
      MITK_ERROR << "Could not load series: " << seriesUID.toStdString();
  }
  else
  {        
      this->GetDataStorage()->Add(node);

      ctkServiceReference serviceReference = org_mitk_example_gui_views_Activator::GetPluginContext()->getServiceReference<mitk::IDataStorageService>();
      mitk::IDataStorageService* storageService = org_mitk_example_gui_views_Activator::GetPluginContext()->getService<mitk::IDataStorageService>(serviceReference);
      mitk::RenderingManager::GetInstance()->SetDataStorage(storageService->GetActiveDataStorage().GetPointer()->GetDataStorage());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      berry::IWorkbenchWindow::Pointer window = this->GetSite()->GetWorkbenchWindow();
      std::string perspectiveId = "org.mitk.example.viewerperspective";
      window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
  }
}

void DicomView::SetFocus ()
{
}