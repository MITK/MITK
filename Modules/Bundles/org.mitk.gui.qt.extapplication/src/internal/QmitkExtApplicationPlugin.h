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


#ifndef QMITKEXTAPPLICATIONPLUGIN_H_
#define QMITKEXTAPPLICATIONPLUGIN_H_

#include <berryAbstractUIPlugin.h>

#include <QString>

class QmitkExtApplicationPlugin : public berry::AbstractUIPlugin
{
public:

  QmitkExtApplicationPlugin();

  static QmitkExtApplicationPlugin* GetDefault();

  berry::IBundleContext::Pointer GetBundleContext() const;

  void Start(berry::IBundleContext::Pointer);

  QString GetQtHelpCollectionFile() const;

private:

  static QmitkExtApplicationPlugin* inst;

  berry::IBundleContext::Pointer context;
};

#endif /* QMITKEXTAPPLICATIONPLUGIN_H_ */
