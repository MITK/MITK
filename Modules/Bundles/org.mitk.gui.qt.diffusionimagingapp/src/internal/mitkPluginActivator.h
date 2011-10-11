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

#ifndef QMITKDIFFUSIONIMAGINGAPPAPPLICATIONPLUGIN_H_
#define QMITKDIFFUSIONIMAGINGAPPAPPLICATIONPLUGIN_H_

#include <QString>
#include <berryAbstractUICTKPlugin.h>
#include <berryIBundleContext.h>

#include <berryQCHPluginListener.h>
#include <mitkExportMacros.h>

class MITK_LOCAL mitkPluginActivator :
    public QObject, public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

      mitkPluginActivator();
      ~mitkPluginActivator();

      static mitkPluginActivator* GetDefault();

      ctkPluginContext* GetPluginContext() const;

      void start(ctkPluginContext*);

      QString GetQtHelpCollectionFile() const;

    private:

      static mitkPluginActivator* inst;

      ctkPluginContext* context;
      berry::QCHPluginListener* pluginListener;

      mutable QString helpCollectionFile;


}; // QmitkDiffusionImagingAppApplicationPlugin

#endif // QMITKDIFFUSIONIMAGINGAPPAPPLICATIONPLUGIN_H_
