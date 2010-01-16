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

#include <mitkVersion.h>

#include <QFileInfo>

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::inst = 0;

QmitkExtApplicationPlugin::QmitkExtApplicationPlugin()
{
  inst = this;
}

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkExtApplicationPlugin::Start(berry::IBundleContext::Pointer context)
{
  this->context = context;
}

berry::IBundleContext::Pointer QmitkExtApplicationPlugin::GetBundleContext() const
{
  return context;
}

QString QmitkExtApplicationPlugin::GetQtHelpCollectionFile() const
{
  Poco::Path collectionPath = context->GetThisBundle()->GetPath();
  collectionPath.pushDirectory("resources");

  std::string collectionFilename;
  std::string na = "n/a";
  if (na != MITK_SVN_REVISION)
    collectionFilename = "MitkExtQtHelpCollection_" MITK_SVN_REVISION ".qhc";
  else
    collectionFilename = "MitkExtQtHelpCollection.qhc";

  collectionPath.setFileName(collectionFilename);

  QString collectionFile = QString::fromStdString(collectionPath.toString());
  if (QFileInfo(collectionFile).exists())
    return collectionFile;
  return QString("");
}
