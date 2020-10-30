/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "DicomView.h"

#include "org_mitk_example_gui_customviewer_views_Activator.h"

#include "mitkIOUtil.h"
#include "mitkIDataStorageService.h"
#include "mitkImage.h"

#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>

#include "QDockWidget"

const std::string DicomView::VIEW_ID = "org.mitk.customviewer.views.dicomview";

DicomView::DicomView() : m_Parent(nullptr)
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

  // remove unused widgets
  QPushButton *downloadButton = parent->findChild<QPushButton *>("downloadButton");
  downloadButton->setVisible(false);

  connect(m_Controls.importButton, SIGNAL(clicked()), m_Controls.widget, SLOT(OnFolderCDImport()));
  connect(m_Controls.widget,
          SIGNAL(SignalDicomToDataManager(const QHash<QString, QVariant> &)),
          this,
          SLOT(AddDataNodeFromDICOM(const QHash<QString, QVariant> &)));

  m_Parent->setEnabled(true);
}
// //! [DicomViewCreatePartControl]

// //! [DicomViewCreateAddDataNodeInformation]
void DicomView::AddDataNodeFromDICOM(QHash<QString, QVariant> eventProperties)
{
  QStringList listOfFilesForSeries;
  std::vector<std::string> seriesToLoad;

  listOfFilesForSeries = eventProperties["FilesForSeries"].toStringList();

  if (!listOfFilesForSeries.isEmpty())
  {
    QStringListIterator it(listOfFilesForSeries);

    while (it.hasNext())
    {
      seriesToLoad.push_back(it.next().toStdString());
    }

    auto results = mitk::IOUtil::Load(seriesToLoad, *(this->GetDataStorage().GetPointer()));

    if (results->empty())
    {
      MITK_ERROR << "Error loading Dicom series";
    }
    // //! [DicomViewCreateAddDataNodeLoadSeries]
    else
    {
      // //! [DicomViewCreateAddDataNode]
      mitk::DataStorage::Pointer ds = this->GetDataStorage();
      // //! [DicomViewCreateAddDataNode]
      mitk::RenderingManager::GetInstance()->SetDataStorage(ds);
      auto geometry = ds->ComputeBoundingGeometry3D(ds->GetAll());
      mitk::RenderingManager::GetInstance()->InitializeViews(geometry);

      // //! [DicomViewCreateAddDataNodeActivatePersp]
      berry::IWorkbenchWindow::Pointer window = this->GetSite()->GetWorkbenchWindow();
      QString perspectiveId = "org.mitk.example.viewerperspective";
      window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      // //! [DicomViewCreateAddDataNodeActivatePersp]
    }
    // //! [DicomViewCreateAddDataNodeLoadSeries]
  }
}
// //! [DicomViewCreateAddDataNodeInformation]

void DicomView::SetFocus()
{
}
