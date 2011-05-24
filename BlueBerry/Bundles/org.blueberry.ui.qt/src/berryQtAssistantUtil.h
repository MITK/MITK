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
  static bool RegisterQCHFiles(const QString& collectionFile, const std::vector<IBundle::Pointer>& bundles);
  static bool RegisterQCHFiles(const QString& collectionFile, const std::vector<IBundle::Pointer>& bundles,
                               const QList<QSharedPointer<ctkPlugin> >& plugins);

  static bool RegisterQCHFiles(const QString& collectionFile, const QList<QSharedPointer<ctkPlugin> >& plugins);

  static void SetHelpColletionFile(const QString& file);
  static void SetDefaultHelpUrl(const QString& defaultUrl);

private:

  static QProcess* assistantProcess;
  static QString helpCollectionFile;
  static QString defaultHelpUrl;

  static QStringList registeredBundles;

  static QString GetAssistantExecutable();
  static QStringList ExtractQCHFiles(const std::vector<IBundle::Pointer>& bundles);
  static QStringList ExtractQCHFiles(const QList<QSharedPointer<ctkPlugin> >& plugins);
  static bool CallQtAssistant(const QString& collectionFile, const QStringList& qchFiles);

};

}

#endif /* BERRYQTASSISTANTUTIL_H_ */
