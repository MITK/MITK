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
#include <QmitkIGTCommonHelper.h>
#include <QSettings>
#include <QFileInfo>

const std::string QmitkIGTCommonHelper::VIEW_ID = "org.mitk.views.igtcommonhelper";

void QmitkIGTCommonHelper::SetLastFileSavePath(const QString &str)
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(QmitkIGTCommonHelper::VIEW_ID));
  settings.setValue("LastFileSavePath",QVariant(str));
  settings.endGroup();
}

void QmitkIGTCommonHelper::SetLastFileSavePathByFileName(const QString &str)
{
  QFileInfo file(str);
  SetLastFileSavePath(file.absolutePath());
}

const QString QmitkIGTCommonHelper::GetLastFileSavePath()
{
  QString path = "";
  QSettings settings;
  settings.beginGroup(QString::fromStdString(QmitkIGTCommonHelper::VIEW_ID));
  path = settings.value("LastFileSavePath",QString("")).toString();
  settings.endGroup();

  return path;
}

void QmitkIGTCommonHelper::SetLastFileLoadPath(const QString &str)
{
  QSettings settings;
  settings.beginGroup(QString::fromStdString(QmitkIGTCommonHelper::VIEW_ID));
  settings.setValue("LastFileLoadPath",QVariant(str));
  settings.endGroup();
}

void QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(const QString &str)
{
  QFileInfo file(str);
  SetLastFileLoadPath(file.absolutePath());
}

const QString QmitkIGTCommonHelper::GetLastFileLoadPath()
{
  QString path = "";
  QSettings settings;
  settings.beginGroup(QString::fromStdString(QmitkIGTCommonHelper::VIEW_ID));
  path = settings.value("LastFileLoadPath",QString("")).toString();
  settings.endGroup();

  return path;
}
