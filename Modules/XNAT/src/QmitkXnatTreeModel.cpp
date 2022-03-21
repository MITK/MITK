/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatTreeModel.h"
#include <QmitkHttpStatusCodeHandler.h>

#include <QmitkMimeTypes.h>

#include <QIcon>

#include <ctkXnatDataModel.h>
#include <ctkXnatExperiment.h>
#include <ctkXnatFile.h>
#include <ctkXnatProject.h>
#include <ctkXnatResource.h>
#include <ctkXnatResourceCatalogXmlParser.h>
#include <ctkXnatResourceFolder.h>
#include <ctkXnatScan.h>
#include <ctkXnatScanFolder.h>
#include <ctkXnatSubject.h>

#include <iostream>

QmitkXnatTreeModel::QmitkXnatTreeModel() : ctkXnatTreeModel()
{
}

QModelIndexList QmitkXnatTreeModel::match(
  const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
  QModelIndexList result;
  uint matchType = flags & 0x0F;
  Qt::CaseSensitivity cs = flags & Qt::MatchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  bool recurse = flags & Qt::MatchRecursive;
  bool wrap = flags & Qt::MatchWrap;
  bool allHits = (hits == -1);
  QString text; // only convert to a string if it is needed
  QModelIndex p = parent(start);
  int from = start.row();
  int to = rowCount(p);

  // iterates twice if wrapping
  for (int i = 0; (wrap && i < 2) || (!wrap && i < 1); ++i)
  {
    for (int r = from; (r < to) && (allHits || result.count() < hits); ++r)
    {
      QModelIndex idx = index(r, start.column(), p);
      if (!idx.isValid())
        continue;
      QVariant v = data(idx, role);
      // QVariant based matching
      if (matchType == Qt::MatchExactly)
      {
        if (value != v)
          result.append(idx);
      }
      else
      {                     // QString based matching
        if (text.isEmpty()) // lazy conversion
          text = value.toString();
        QString t = v.toString();
        switch (matchType)
        {
          case Qt::MatchRegExp:
            if (!QRegExp(text, cs).exactMatch(t))
              result.append(idx);
            break;
          case Qt::MatchWildcard:
            if (!QRegExp(text, cs, QRegExp::Wildcard).exactMatch(t))
              result.append(idx);
            break;
          case Qt::MatchStartsWith:
            if (!t.startsWith(text, cs))
              result.append(idx);
            break;
          case Qt::MatchEndsWith:
            if (!t.endsWith(text, cs))
              result.append(idx);
            break;
          case Qt::MatchFixedString:
            if (t.compare(text, cs) != 0)
              result.append(idx);
            break;
          case Qt::MatchContains:
          default:
            if (!t.contains(text, cs))
              result.append(idx);
        }
      }
      if (recurse && hasChildren(idx))
      { // search the hierarchy
        result += match(index(0, idx.column(), idx),
                        role,
                        (text.isEmpty() ? value : text),
                        (allHits ? -1 : hits - result.count()),
                        flags);
      }
    }
    // prepare for the next iteration
    from = 0;
    to = start.row();
  }
  return result;
}

void QmitkXnatTreeModel::fetchMore(const QModelIndex &index)
{
  try
  {
    ctkXnatTreeModel::fetchMore(index);
  }
  catch (const ctkRuntimeException& e)
  {
    QmitkHttpStatusCodeHandler::HandleErrorMessage(e.what());
    emit Error(index);
  }
}

QVariant QmitkXnatTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::DecorationRole)
  {
    ctkXnatObject *xnatObject = this->xnatObject(index);
    QString path;

    if (dynamic_cast<ctkXnatDataModel *>(xnatObject))
    {
      path = ":/xnat-module/xnat-server.png";
    }
    else if (dynamic_cast<ctkXnatProject *>(xnatObject))
    {
      path = ":/xnat-module/xnat-project.png";
    }
    else if (dynamic_cast<ctkXnatSubject *>(xnatObject))
    {
      path = ":/xnat-module/xnat-subject.png";
    }
    else if (dynamic_cast<ctkXnatExperiment *>(xnatObject))
    {
      path = ":/xnat-module/xnat-experiment.png";
    }
    else if (dynamic_cast<ctkXnatResourceFolder *>(xnatObject))
    {
      path = ":/xnat-module/xnat-folder.png";
    }
    else if (dynamic_cast<ctkXnatResource *>(xnatObject))
    {
      path = ":/xnat-module/xnat-resource.png";
    }
    else if (dynamic_cast<ctkXnatScanFolder *>(xnatObject))
    {
      path = ":/xnat-module/xnat-folder.png";
    }
    else if (dynamic_cast<ctkXnatScan *>(xnatObject))
    {
      path = ":/xnat-module/xnat-scan.png";
    }
    else if (dynamic_cast<ctkXnatFile *>(xnatObject))
    {
      path = ":/xnat-module/xnat-file.png";
    }
    return QIcon(path);
  }
  return ctkXnatTreeModel::data(index, role);
}

bool QmitkXnatTreeModel::dropMimeData(
  const QMimeData *data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  // Return true if data can be handled
  bool returnVal(false);

  if (data->hasFormat(QmitkMimeTypes::DataNodePtrs))
  {
    returnVal = true;
    QList<mitk::DataNode *> droppedNodes = QmitkMimeTypes::ToDataNodePtrList(data);
    ctkXnatObject *parentXnatObj = this->xnatObject(parent);
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
    bool droppingAllowed = dynamic_cast<ctkXnatSubject *>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatExperiment *>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatResource *>(this->xnatObject(index)) != nullptr;
    droppingAllowed |= dynamic_cast<ctkXnatResourceFolder *>(this->xnatObject(index)) != nullptr;

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

ctkXnatObject *QmitkXnatTreeModel::InternalGetXnatObjectFromUrl(const QString &xnatObjectType,
                                                                const QString &url,
                                                                ctkXnatObject *parent)
{
  // 1. Find project
  int start = url.lastIndexOf(xnatObjectType);
  if (start == -1)
    return nullptr;

  start += xnatObjectType.length();

  parent->fetch();
  QList<ctkXnatObject *> children = parent->children();
  foreach (ctkXnatObject *child, children)
  {
    if (url.indexOf(child->resourceUri()) != -1)
    {
      return child;
    }
  }
  return nullptr;
}

ctkXnatObject *QmitkXnatTreeModel::GetXnatObjectFromUrl(const QString &url)
{
  QModelIndex index = this->index(0, 0, QModelIndex());
  ctkXnatObject *currentXnatObject = nullptr;
  currentXnatObject = this->xnatObject(index);
  if (currentXnatObject != nullptr)
  {
    // 1. Find project
    ctkXnatObject *project = nullptr;
    project = this->InternalGetXnatObjectFromUrl("projects/", url, currentXnatObject);

    // 2. Find subject
    ctkXnatObject *subject = nullptr;
    if (project != nullptr)
    {
      currentXnatObject = project;
      subject = this->InternalGetXnatObjectFromUrl("subjects/", url, project);
    }

    // 3. Find experiment
    ctkXnatObject *experiment = nullptr;
    if (subject != nullptr)
    {
      currentXnatObject = subject;
      experiment = this->InternalGetXnatObjectFromUrl("experiments/", url, subject);
    }

    // 4. Find scan
    ctkXnatObject *scan = nullptr;
    if (experiment != nullptr)
    {
      currentXnatObject = experiment;
      scan = this->InternalGetXnatObjectFromUrl("scans/", url, experiment);
    }

    if (scan != nullptr)
    {
      scan->fetch();
      QList<ctkXnatObject *> scans = scan->children();
      foreach (ctkXnatObject *child, scans)
      {
        if (url.indexOf(child->resourceUri()) != -1)
        {
          return child;
        }
      }
    }

    currentXnatObject->fetch();
    QList<ctkXnatObject *> bla = currentXnatObject->children();
    foreach (ctkXnatObject *child, bla)
    {
      if (child->name() == "Resources")
        return child;
    }
  }
  return nullptr;
}
