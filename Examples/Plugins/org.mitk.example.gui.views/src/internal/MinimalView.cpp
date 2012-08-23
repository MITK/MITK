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

#include "MinimalView.h"

#include <QMessageBox>

#include "org_mitk_example_gui_testapplication_Activator.h"

#include "mitkIDataStorageService.h"
#include "mitkDicomSeriesReader.h"

const std::string MinimalView::VIEW_ID = "org.mitk.views.minimalview"; 

//MinimalView::MinimalView()
//  : m_Parent(0)
//  , m_SelectionListener(new berry::SelectionChangedAdapter<MinimalView>(this, &MinimalView::SelectionChanged))
//{
//}

MinimalView::MinimalView()
  : m_Parent(0)
{
}

MinimalView::~MinimalView()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //s->RemoveSelectionListener(m_SelectionListener);
}

void MinimalView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //ctkPluginContext* pluginContext = org_mitk_example_gui_testapplication_Activator::GetPluginContext();
  //ctkServiceReference serviceReference = pluginContext->getServiceReference<berry::idata>();

  ////always granted by org.blueberry.ui.qt
  //Q_ASSERT(serviceReference);
  //
  //berry::IQtStyleManager* styleManager = pluginContext->getService<berry::IQtStyleManager>(serviceReference);
  //Q_ASSERT(styleManager);

  ctkServiceReference serviceReference = org_mitk_example_gui_testapplication_Activator::GetPluginContext()->getServiceReference<mitk::IDataStorageService>();
  mitk::IDataStorageService* storageService = org_mitk_example_gui_testapplication_Activator::GetPluginContext()->getService<mitk::IDataStorageService>(serviceReference);

  storageService->GetActiveDataStorage().GetPointer()->GetDataStorage();

  connect(m_Controls.importButton, SIGNAL(clicked()), m_Controls.widget, SLOT(OnFolderCDImport()));
  connect(m_Controls.widget, SIGNAL(SignalDicomToDataManager(const QStringList&)), this, SLOT(AddDataNodeFromDICOM(const QStringList&)));
  
  // register selection listener
  //GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  m_Parent->setEnabled(true);
}

void MinimalView::AddDataNodeFromDICOM(const QStringList& Properties)
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

      ctkServiceReference serviceReference = org_mitk_example_gui_testapplication_Activator::GetPluginContext()->getServiceReference<mitk::IDataStorageService>();
      mitk::IDataStorageService* storageService = org_mitk_example_gui_testapplication_Activator::GetPluginContext()->getService<mitk::IDataStorageService>(serviceReference);
      mitk::RenderingManager::GetInstance()->SetDataStorage(storageService->GetActiveDataStorage().GetPointer()->GetDataStorage());
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


//void MinimalView::ToggleRadioMethod()
//{
//  bool buttonState = m_Controls.radioButton->isChecked();
//  if (buttonState) m_Controls.radioButton_2->toggle();
//  else m_Controls.radioButton->toggle();
//}

void MinimalView::SetFocus ()
{
}

//void MinimalView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
//                               berry::ISelection::ConstPointer selection)
//{
//  if (sourcepart != this && 
//      selection.Cast<const berry::IStructuredSelection>())
//  {
//    ToggleRadioMethod();
//    //DoSomething(selection.Cast<const berry::IStructuredSelection>());
//  }
//}