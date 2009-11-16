/*=========================================================================
 
 Program:   openCherry Platform
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


#ifndef CHERRYQTASSISTANTUTIL_H_
#define CHERRYQTASSISTANTUTIL_H_

#include <QStringList>
#include <QProcess>

#include <vector>
#include <cherryIBundle.h>

#include "cherryUiQtDll.h"

namespace cherry {

class CHERRY_UI_QT QtAssistantUtil
{

public:

  static void OpenAssistant(const QString& collectionFile, const QString& startPage);

  static bool RegisterQCHFiles(const QString& collectionFile, const std::vector<IBundle::Pointer>& bundles);

private:

  static QProcess* assistantProcess;

};

}

#endif /* CHERRYQTASSISTANTUTIL_H_ */
