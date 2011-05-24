/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "QmitkExtApplicationPlugin.h"
#include "QmitkExtDefaultPerspective.h"
#include "../QmitkExtApplication.h"

#include <mitkVersion.h>

#include <QFileInfo>
#include <QDateTime>
#include <QtPlugin>

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::inst = 0;

QmitkExtApplicationPlugin::QmitkExtApplicationPlugin()
{
  inst = this;
}

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkExtApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);
  
  this->context = context;
  
  BERRY_REGISTER_EXTENSION_CLASS(QmitkExtDefaultPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkExtApplication, context);
}

ctkPluginContext* QmitkExtApplicationPlugin::GetPluginContext() const
{
  return context;
}

QString QmitkExtApplicationPlugin::GetQtHelpCollectionFile() const
{
  QString collectionFilename;
  QString na("n/a");
  if (na != MITK_REVISION)
    collectionFilename = "MitkExtQtHelpCollection_" MITK_REVISION ".qhc";
  else
    collectionFilename = "MitkExtQtHelpCollection.qhc";

  QFileInfo collectionFileInfo = context->getDataFile(collectionFilename);
  QFileInfo pluginFileInfo = QFileInfo(QUrl(context->getPlugin()->getLocation()).toLocalFile());

  if (!collectionFileInfo.exists() ||
      pluginFileInfo.lastModified() > collectionFileInfo.lastModified())
  {
    // extract the qhc file from the plug-in
    QByteArray content = context->getPlugin()->getResource(collectionFilename);
    if (content.isEmpty())
    {
      BERRY_WARN << "Could not get plug-in resource: " << collectionFilename.toStdString();
    }
    else
    {
      QFile file(collectionFileInfo.canonicalFilePath());
      file.open(QIODevice::WriteOnly);
      file.write(content);
    }
  }

  if (collectionFileInfo.exists())
  {
    return collectionFileInfo.absoluteFilePath();
  }

  return QString();
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_extapplication, QmitkExtApplicationPlugin)
