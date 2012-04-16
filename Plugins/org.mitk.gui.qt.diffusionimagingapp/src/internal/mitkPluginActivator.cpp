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

#include "mitkPluginActivator.h"
#include <QtPlugin>
#include <mitkVersion.h>
#include <berryQtAssistantUtil.h>

#include "src/QmitkDiffusionImagingAppApplication.h"

#include "QmitkWelcomePerspective.h"
#include "src/internal/QmitkDiffusionImagingAppIntroPart.h"
#include "src/internal/QmitkDiffusionImagingAppPerspective.h"

#include "src/internal/QmitkDIAppConnectomicsPerspective.h"
#include "src/internal/QmitkDIAppDicomImportPerspective.h"
#include "src/internal/QmitkDIAppFiberTractographyPerspective.h"
#include "src/internal/QmitkDIAppIVIMPerspective.h"
#include "src/internal/QmitkDIAppPreprocessingReconstructionPerspective.h"
#include "src/internal/QmitkDIAppQuantificationPerspective.h"
#include "src/internal/QmitkDIAppScreenshotsMoviesPerspective.h"
#include "src/internal/QmitkDIAppTBSSPerspective.h"
#include "src/internal/QmitkDIAppVolumeVisualizationPerspective.h"

#include <QFileInfo>
#include <QDateTime>

mitkPluginActivator* mitkPluginActivator::inst = 0;

mitkPluginActivator::mitkPluginActivator()
    : pluginListener(0)
{
  inst = this;
}

mitkPluginActivator::~mitkPluginActivator()
{
  delete pluginListener;
}

mitkPluginActivator* mitkPluginActivator::GetDefault()
{
  return inst;
}

void mitkPluginActivator::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppIntroPart, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkWelcomePerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppConnectomicsPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppDicomImportPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppFiberTractographyPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppIVIMPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppPreprocessingReconstructionPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppQuantificationPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppScreenshotsMoviesPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppTBSSPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppVolumeVisualizationPerspective, context)

  delete pluginListener;
  pluginListener = new berry::QCHPluginListener(context);
  context->connectPluginListener(pluginListener, SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // register all QCH files from all the currently installed plugins
  pluginListener->processPlugins();


}

QString mitkPluginActivator::GetQtHelpCollectionFile() const
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

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimagingapp, mitkPluginActivator)


