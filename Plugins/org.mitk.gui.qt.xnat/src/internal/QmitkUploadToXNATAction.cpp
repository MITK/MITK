/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkUploadToXNATAction.h"

//needed for qApp
#include <QCoreApplication>

#include "org_mitk_gui_qt_xnatinterface_Activator.h"
#include <QmitkSelectXnatUploadDestinationDialog.h>
#include <mitkIDataStorageService.h>
#include <mitkIOUtil.h>
#include <mitkNodePredicateProperty.h>

#include <ctkXnatDefaultSchemaTypes.h>
#include <ctkXnatFile.h>
#include <ctkXnatObject.h>
#include <ctkXnatResource.h>
#include <ctkServiceTracker.h>
#include <ctkXnatSession.h>

#include <QDir>
#include <QMessageBox>
#include <QString>

void ShowInfoMessage()
{
  QMessageBox infoBox;
  infoBox.setIcon(QMessageBox::Information);
  infoBox.setText("You are not connected to a XNAT server!");
  infoBox.setInformativeText("Please use the 'Connect' button in the Preferences.");
  infoBox.exec();
}

QmitkUploadToXNATAction::QmitkUploadToXNATAction()
{
}

QmitkUploadToXNATAction::~QmitkUploadToXNATAction()
{
}

void QmitkUploadToXNATAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  if (selectedNodes.size() != 1)
  {
    QMessageBox infoBox;
    infoBox.setIcon(QMessageBox::Information);
    infoBox.setText("Please select only one data node for upload");
    infoBox.exec();
  }

  // Try to get the XNAT session
  us::ServiceReference<ctkXnatSession> modServiceRef = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext()->GetServiceReference<ctkXnatSession>();

  if (!modServiceRef)
  {
    ShowInfoMessage();
    return;
  }

  us::ModuleContext* xnatModuleContext = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatModuleContext();

  if (xnatModuleContext == nullptr)
  {
    ShowInfoMessage();
    return;
  }

  ctkXnatSession *session(nullptr);
  session = xnatModuleContext->GetService(modServiceRef);
  if (session == nullptr || !session->isOpen())
  {
    ShowInfoMessage();
    return;
  }

  mitk::DataNode* selectedNode = selectedNodes.at(0);

  if (selectedNode == nullptr)
    return;

  ctkServiceTracker<mitk::IDataStorageService*> dataStorageServiceTracker (mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext());
  dataStorageServiceTracker.open();
  mitk::IDataStorageService* dsService = dataStorageServiceTracker.getService();
  mitk::DataStorage::Pointer dataStorage = dsService->GetDataStorage()->GetDataStorage();

  mitk::NodePredicateProperty::Pointer pred = mitk::NodePredicateProperty::New("xnat.url");
  mitk::DataStorage::SetOfObjects::ConstPointer result = dataStorage->GetSources(selectedNode, pred);
  mitk::DataStorage::SetOfObjects::ConstIterator it = result->Begin();

  QList<ctkXnatObject*> resourceFolders;
  QStringList resourceNames;
  QString url;
  for (;it != result->End(); ++it)
  {
    mitk::DataNode::Pointer node = it->Value();

    std::string xnatUrl("");
    node->GetStringProperty("xnat.url", xnatUrl);
    url = QString::fromStdString(xnatUrl);

    int start = url.lastIndexOf("resources/") + 10; //length of "resources/"
    url = url.left(start);

    QUuid uid = session->httpGet(url);
    resourceFolders = session->httpResults(uid, ctkXnatDefaultSchemaTypes::XSI_RESOURCE);

    foreach (ctkXnatObject* obj, resourceFolders)
    {
      resourceNames << obj->name();
    }
  }

  // Dialog for selecting the upload destination
  QmitkSelectXnatUploadDestinationDialog dialog(session, resourceNames);
  dialog.setWindowTitle("Select XNAT upload destination");
  dialog.SetXnatResourceFolderUrl(url);
  int returnValue = dialog.exec();

  if (returnValue == QDialog::Accepted)
  {
    // Save node
    QString fileName (QString::fromStdString(ReplaceSpecialChars(selectedNode->GetName())));

    if (dynamic_cast<mitk::Image*>(selectedNode->GetData()))
    {
      fileName.append(".nrrd");
    }
    else if (dynamic_cast<mitk::Surface*>(selectedNode->GetData()))
    {
      fileName.append(".vtk");
    }
    else if (dynamic_cast<mitk::PointSet*>(selectedNode->GetData()))
    {
      fileName.append(".mps");
    }
    else
    {
      MITK_WARN << "Could not upload file! File-type not supported";
      QMessageBox msgbox;
      msgbox.setText("Could not upload file! File-type not supported");
      msgbox.setIcon(QMessageBox::Critical);
      msgbox.exec();
      return;
    }

    QString xnatFolder = "XNAT_UPLOADS";
    QDir dir(mitk::org_mitk_gui_qt_xnatinterface_Activator::GetContext()->getDataFile("").absoluteFilePath());
    dir.mkdir(xnatFolder);

    fileName = dir.path().append("/" + fileName);
    mitk::IOUtil::Save (selectedNode->GetData(), fileName.toStdString());

    // Upload the file to XNAT
    ctkXnatObject* uploadDestination = dialog.GetUploadDestination();
    if (uploadDestination != nullptr)
    {
      ctkXnatFile* file = new ctkXnatFile(uploadDestination);
      file->setLocalFilePath(fileName);
      QFileInfo fileInfo (fileName);
      file->setName(fileInfo.fileName());
      file->save();
    }
  }
  dataStorageServiceTracker.close();
}

void QmitkUploadToXNATAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
}

void QmitkUploadToXNATAction::SetFunctionality(berry::QtViewPart* /*functionality*/)
{
}

void QmitkUploadToXNATAction::SetSmoothed(bool /*smoothed*/)
{
}

void QmitkUploadToXNATAction::SetDecimated(bool /*smoothed*/)
{
}

std::string QmitkUploadToXNATAction::ReplaceSpecialChars(const std::string& input) const
{
  QString convertedString = QString(QUrl::toPercentEncoding(QString::fromStdString(input)));
  return  convertedString.toStdString();
}
