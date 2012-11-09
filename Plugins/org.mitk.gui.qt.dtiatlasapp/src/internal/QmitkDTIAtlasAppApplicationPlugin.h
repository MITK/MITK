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

#ifndef QMITKDTIATLASAPPAPPLICATIONPLUGIN_H_
#define QMITKDTIATLASAPPAPPLICATIONPLUGIN_H_

#include <QString>
#include <berryAbstractUICTKPlugin.h>
#include <berryIBundleContext.h>

#include <berryQCHPluginListener.h>

class QmitkDTIAtlasAppApplicationPlugin :
    public QObject, public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

      QmitkDTIAtlasAppApplicationPlugin();
      ~QmitkDTIAtlasAppApplicationPlugin();

      static QmitkDTIAtlasAppApplicationPlugin* GetDefault();

      ctkPluginContext* GetPluginContext() const;

      void start(ctkPluginContext*);

      QString GetQtHelpCollectionFile() const;

    private:

      static QmitkDTIAtlasAppApplicationPlugin* inst;

      ctkPluginContext* context;
      berry::QCHPluginListener* pluginListener;

      mutable QString helpCollectionFile;


}; // QmitkDTIAtlasAppApplicationPlugin

#endif // QMITKDTIATLASAPPAPPLICATIONPLUGIN_H_
