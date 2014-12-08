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

#include "QmitkDTIAtlasAppApplicationPlugin.h"
#include <QtPlugin>
#include <mitkVersion.h>
#include <berryQtAssistantUtil.h>

#include "src/QmitkDTIAtlasAppApplication.h"

#include "src/internal/QmitkDTIAtlasAppIntroPart.h"
#include "src/internal/QmitkDTIAtlasAppPerspective.h"
#include "src/internal/QmitkWelcomePerspective.h"

#include <QFileInfo>
#include <QDateTime>

QmitkDTIAtlasAppApplicationPlugin* QmitkDTIAtlasAppApplicationPlugin::inst = 0;

QmitkDTIAtlasAppApplicationPlugin::QmitkDTIAtlasAppApplicationPlugin()
    : pluginListener(0)
{
  inst = this;
}

QmitkDTIAtlasAppApplicationPlugin::~QmitkDTIAtlasAppApplicationPlugin()
{
  delete pluginListener;
}

QmitkDTIAtlasAppApplicationPlugin* QmitkDTIAtlasAppApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkDTIAtlasAppApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDTIAtlasAppApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDTIAtlasAppIntroPart, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDTIAtlasAppPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkWelcomePerspective, context)

//  QString collectionFile = GetQtHelpCollectionFile();

//  berry::QtAssistantUtil::SetHelpCollectionFile(collectionFile);
//  berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.dtiatlasapp/bundle/index.html");

  delete pluginListener;
  pluginListener = new berry::QCHPluginListener(context);
  context->connectPluginListener(pluginListener, SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // register all QCH files from all the currently installed plugins
  pluginListener->processPlugins();


}

QString QmitkDTIAtlasAppApplicationPlugin::GetQtHelpCollectionFile() const
{

    if (!helpCollectionFile.isEmpty())
    {
      return helpCollectionFile;
    }

    QString collectionFilename;
    QString na("n/a");
//    if (na != MITK_REVISION)
//      collectionFilename = "MitkDTIAtlasAppQtHelpCollection_" MITK_REVISION ".qhc";
//    else
      collectionFilename = "MitkDTIAtlasAppQtHelpCollection.qhc";

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
        QFile file(collectionFileInfo.absoluteFilePath());
        file.open(QIODevice::WriteOnly);
        file.write(content);
        file.close();
      }
    }

    if (QFile::exists(collectionFileInfo.absoluteFilePath()))
    {
      helpCollectionFile = collectionFileInfo.absoluteFilePath();
    }

    return helpCollectionFile;

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_dtiatlasapp, QmitkDTIAtlasAppApplicationPlugin)
#endif
