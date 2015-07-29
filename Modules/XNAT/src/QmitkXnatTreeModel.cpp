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

#include "QmitkXnatTreeModel.h"

#include <QmitkMimeTypes.h>

#include <QIcon>

#include <ctkXnatDataModel.h>
#include <ctkXnatExperiment.h>
#include <ctkXnatProject.h>
#include <ctkXnatResource.h>
#include <ctkXnatResourceFolder.h>
#include <ctkXnatSubject.h>
#include <ctkXnatFile.h>
#include <ctkXnatResourceCatalogXmlParser.h>
#include <ctkXnatScan.h>
#include <ctkXnatScanFolder.h>

QmitkXnatTreeModel::QmitkXnatTreeModel ()
  : ctkXnatTreeModel()
{

}

QVariant QmitkXnatTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::DecorationRole)
  {
    ctkXnatObject* xnatObject = this->xnatObject(index);
    QString path;

    if(dynamic_cast<ctkXnatDataModel*>(xnatObject))
    {
      path = ":/xnat/server.ico";
    }
    else if(dynamic_cast<ctkXnatProject*>(xnatObject))
    {
      path = ":/xnat/project.png";
    }
    else if(dynamic_cast<ctkXnatSubject*>(xnatObject))
    {
      path = ":/xnat/subject.ico";
    }
    else if(dynamic_cast<ctkXnatExperiment*>(xnatObject))
    {
      path = ":/xnat/experiment.ico";
    }
    else if (dynamic_cast<ctkXnatResourceFolder*>(xnatObject))
    {
      path = ":/xnat/folder.ico";
    }
    else if (dynamic_cast<ctkXnatResource*>(xnatObject))
    {
      path = ":/xnat/resource.png";
    }
    else if (dynamic_cast<ctkXnatScanFolder*>(xnatObject))
    {
      path = ":/xnat/folder.ico";
    }
    else if (dynamic_cast<ctkXnatScan*>(xnatObject))
    {
      path = ":/xnat/scan.png";
    }
    else if (dynamic_cast<ctkXnatFile*>(xnatObject))
    {
      path = ":/xnat/file.png";
    }
    return QIcon(path);
  }
  return ctkXnatTreeModel::data(index, role);
}

bool QmitkXnatTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  // Return true if data can be handled
  bool returnVal (false);

  if (data->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    returnVal = true;
    QList<mitk::DataNode*> droppedNodes = QmitkMimeTypes::ToDataNodePtrList(data);
    ctkXnatObject* parentXnatObj = this->xnatObject(parent);
    emit ResourceDropped(droppedNodes, parentXnatObj, parent);
  }
  return returnVal;
}

Qt::DropActions QmitkXnatTreeModel::supportedDropActions()
{
  return Qt::CopyAction;
}

Qt::ItemFlags QmitkXnatTreeModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags defaultFlags = ctkXnatTreeModel::flags(index);


  if (index.isValid())
  {
    bool droppingAllowed = dynamic_cast<ctkXnatSubject*>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatExperiment*>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatResource*>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatResourceFolder*>(this->xnatObject(index)) != nullptr;

    // No dropping at project, session or data model level allowed
    if (droppingAllowed)
    {
      return Qt::ItemIsDropEnabled | defaultFlags;
    }
    else
    {
      return defaultFlags;
    }
  }
  else
    return defaultFlags;
}
