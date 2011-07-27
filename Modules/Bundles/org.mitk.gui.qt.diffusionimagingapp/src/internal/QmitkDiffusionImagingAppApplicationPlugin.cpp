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

#include "QmitkDiffusionImagingAppApplicationPlugin.h"
#include <QtPlugin>
#include <mitkVersion.h>
#include <berryQtAssistantUtil.h>

#include "src/QmitkDiffusionImagingAppApplication.h"

#include "src/internal/QmitkDiffusionImagingAppIntroPart.h"
#include "src/internal/QmitkDiffusionImagingAppPerspective.h"
#include "src/internal/QmitkWelcomePerspective.h"

#include <QFileInfo>
#include <QDateTime>

QmitkDiffusionImagingAppApplicationPlugin* QmitkDiffusionImagingAppApplicationPlugin::inst = 0;

QmitkDiffusionImagingAppApplicationPlugin::QmitkDiffusionImagingAppApplicationPlugin()
    : pluginListener(0)
{
  inst = this;
}

QmitkDiffusionImagingAppApplicationPlugin::~QmitkDiffusionImagingAppApplicationPlugin()
{
  delete pluginListener;
}

QmitkDiffusionImagingAppApplicationPlugin* QmitkDiffusionImagingAppApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkDiffusionImagingAppApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppIntroPart, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkWelcomePerspective, context)

//  QString collectionFile = GetQtHelpCollectionFile();

//  berry::QtAssistantUtil::SetHelpCollectionFile(collectionFile);
//  berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.diffusionimagingapp/bundle/index.html");

  delete pluginListener;
  pluginListener = new berry::QCHPluginListener(context);
  context->connectPluginListener(pluginListener, SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // register all QCH files from all the currently installed plugins
  pluginListener->processPlugins();


}

QString QmitkDiffusionImagingAppApplicationPlugin::GetQtHelpCollectionFile() const
{

    if (!helpCollectionFile.isEmpty())
    {
      return helpCollectionFile;
    }

    QString collectionFilename;
    QString na("n/a");
//    if (na != MITK_REVISION)
//      collectionFilename = "MitkDiffusionImagingAppQtHelpCollection_" MITK_REVISION ".qhc";
//    else
      collectionFilename = "MitkDiffusionImagingAppQtHelpCollection.qhc";

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

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimagingapp, QmitkDiffusionImagingAppApplicationPlugin)


