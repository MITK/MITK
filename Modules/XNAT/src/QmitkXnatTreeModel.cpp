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
