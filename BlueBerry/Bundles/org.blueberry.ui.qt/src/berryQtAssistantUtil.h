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
#include <ctkPlugin.h>

#include <QSharedPointer>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * \deprecated Use the org.blueberry.ui.qt.help plug-in instead.
 * \see org_blueberry_ui_qt_help
 *
 * This class is deprecated. Please use the org.blueberry.ui.qt.help
 * plug-in if you want to access help contents in your application.
 */
class BERRY_UI_QT QtAssistantUtil
{

public:

  static void OpenAssistant(const QString& startPage = "");
  static void CloseAssistant();

   /**
     * @brief With this method you can open the help-page of the active bundle. 
     */
  static void OpenActivePartHelp();

  // for legacy BlueBerry bundle support
  static bool RegisterQCHFiles(const std::vector<IBundle::Pointer>& bundles);

  static bool RegisterQCHFiles(const QStringList& qchFiles);
  static bool UnregisterQCHFiles(const QStringList& qchFiles);

  static void SetHelpCollectionFile(const QString& file);
  static QString GetHelpCollectionFile();
  static void SetDefaultHelpUrl(const QString& defaultUrl);

private:

  static QProcess* assistantProcess;
  static QString helpCollectionFile;
  static QString defaultHelpUrl;

  static QSet<QString> registeredBundles;

  static QString GetAssistantExecutable();
  static QStringList ExtractQCHFiles(const std::vector<IBundle::Pointer>& bundles);
  static bool CallQtAssistant(const QStringList& qchFiles, bool registerFile = true);

};

}

#endif /* BERRYQTASSISTANTUTIL_H_ */
