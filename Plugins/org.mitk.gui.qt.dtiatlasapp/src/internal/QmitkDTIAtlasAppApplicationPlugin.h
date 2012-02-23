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
