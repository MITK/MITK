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
      path = ":/xnat-module/xnat-server.png";
    }
    else if(dynamic_cast<ctkXnatProject*>(xnatObject))
    {
      path = ":/xnat-module/xnat-project.png";
    }
    else if(dynamic_cast<ctkXnatSubject*>(xnatObject))
    {
      path = ":/xnat-module/xnat-subject.png";
    }
    else if(dynamic_cast<ctkXnatExperiment*>(xnatObject))
    {
      path = ":/xnat-module/xnat-experiment.png";
    }
    else if (dynamic_cast<ctkXnatResourceFolder*>(xnatObject))
    {
      path = ":/xnat-module/xnat-folder.png";
    }
    else if (dynamic_cast<ctkXnatResource*>(xnatObject))
    {
      path = ":/xnat-module/xnat-resource.png";
    }
    else if (dynamic_cast<ctkXnatScanFolder*>(xnatObject))
    {
      path = ":/xnat-module/xnat-folder.png";
    }
    else if (dynamic_cast<ctkXnatScan*>(xnatObject))
    {
      path = ":/xnat-module/xnat-scan.png";
    }
    else if (dynamic_cast<ctkXnatFile*>(xnatObject))
    {
      path = ":/xnat-module/xnat-file.png";
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

ctkXnatObject* QmitkXnatTreeModel::InternalGetXnatObjectFromUrl(const QString & xnatObjectType, const QString & url,
                                                                ctkXnatObject* parent)
{
  // 1. Find project
  int start = url.lastIndexOf(xnatObjectType);
  if (start == -1)
    return nullptr;

  start += xnatObjectType.length();
  int length = url.indexOf("/",start);
  length -= start;

  parent->fetch();
  QList<ctkXnatObject*> children = parent->children();
  foreach (ctkXnatObject* child, children)
  {
    if(url.indexOf(child->resourceUri()) != -1)
    {
      return child;
    }
  }
  return nullptr;
}

ctkXnatObject* QmitkXnatTreeModel::GetXnatObjectFromUrl(const QString& url)
{
  QModelIndex index = this->index(0,0,QModelIndex());
  ctkXnatObject* currentXnatObject = nullptr;
  currentXnatObject = this->xnatObject(index);
  if (currentXnatObject != nullptr)
  {
    // 1. Find project
    ctkXnatObject* project = nullptr;
    project = this->InternalGetXnatObjectFromUrl("projects/", url, currentXnatObject);

    // 2. Find subject
    ctkXnatObject* subject = nullptr;
    if (project != nullptr)
    {
      currentXnatObject = project;
      subject = this->InternalGetXnatObjectFromUrl("subjects/", url, project);
    }

    // 3. Find experiment
    ctkXnatObject* experiment = nullptr;
    if (subject != nullptr)
    {
      currentXnatObject = subject;
      experiment = this->InternalGetXnatObjectFromUrl("experiments/", url, subject);
    }

    // 4. Find scan
    ctkXnatObject* scan = nullptr;
    if (experiment != nullptr)
    {
      currentXnatObject = experiment;
      scan = this->InternalGetXnatObjectFromUrl("scans/", url, experiment);
    }

    if (scan != nullptr)
    {
      scan->fetch();
      QList<ctkXnatObject*> scans = scan->children();
      foreach (ctkXnatObject* child, scans)
      {
        if (url.indexOf(child->resourceUri()) != -1)
        {
          return child;
        }
      }
    }

    currentXnatObject->fetch();
    QList<ctkXnatObject*> bla = currentXnatObject->children();
    foreach (ctkXnatObject* child, bla)
    {
      if (child->name() == "Resources")
        return child;
    }
  }
  return nullptr;
}
