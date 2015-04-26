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
#include <ctkXnatProject.h>
#include <ctkXnatSubject.h>
#include <ctkXnatExperiment.h>

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
      path = ":/xnat/project.ico";
    }
    else if(dynamic_cast<ctkXnatSubject*>(xnatObject))
    {
      path = ":/xnat/subject.ico";
    }
    else if(dynamic_cast<ctkXnatExperiment*>(xnatObject))
    {
      path = ":/xnat/experiment.ico";
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
    ctkXnatObject* parentObj = this->xnatObject(parent);
    emit ResourceDropped(droppedNodes, parentObj);
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
    ctkXnatProject* xnatProj = dynamic_cast<ctkXnatProject*>(this->xnatObject(index));

    // No dropping at project level allowed
    if (xnatProj == NULL)
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
