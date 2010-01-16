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


#ifndef BERRYQTASSISTANTUTIL_H_
#define BERRYQTASSISTANTUTIL_H_

#include <QStringList>
#include <QProcess>

#include <vector>
#include <berryIBundle.h>

#include "berryUiQtDll.h"

namespace berry {

class BERRY_UI_QT QtAssistantUtil
{

public:

  static void OpenAssistant(const QString& startPage = "");
  static void CloseAssistant();

   /**
     * @brief With this method you can open the help-page of the active bundle. 
     */
  static void OpenActivePartHelp();

  static bool RegisterQCHFiles(const QString& collectionFile, const std::vector<IBundle::Pointer>& bundles);

  static void SetHelpColletionFile(const QString& file);
  static void SetDefaultHelpUrl(const QString& defaultUrl);

private:

  static QProcess* assistantProcess;
  static QString helpCollectionFile;
  static QString defaultHelpUrl;

  static QStringList registeredBundles;

  static QString GetAssistantExecutable();

};

}

#endif /* BERRYQTASSISTANTUTIL_H_ */
