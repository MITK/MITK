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


#ifndef BERRYQCHPLUGINLISTENER_P_H
#define BERRYQCHPLUGINLISTENER_P_H

#include <QObject>
#include <QSharedPointer>

#include <ctkPluginEvent.h>

#include <berryIExtensionPointService.h>

#include <org_blueberry_ui_qt_Export.h>

class ctkPlugin;

namespace berry {

/**
 * A listener for CTK plugin events. When plugins come and go we look to see
 * if there are any extensions or extension points and update the legacy BlueBerry registry accordingly.
 * Using a Synchronous listener here is important. If the
 * plugin activator code tries to access the registry to get its extension
 * points, we need to ensure that they are in the registry before the
 * plugin start is called. By listening sync we are able to ensure that
 * happens.
 *
 * \deprecated Use the org.blueberry.ui.qt.help plug-in instead.
 * \see org_blueberry_ui_qt_help
 */
class BERRY_UI_QT QCHPluginListener : public QObject {

  Q_OBJECT

public:

  QCHPluginListener(ctkPluginContext* context);

  void processPlugins();

public slots:

  void pluginChanged(const ctkPluginEvent& event);

private:

  bool isPluginResolved(QSharedPointer<ctkPlugin> plugin);

  void removePlugin(QSharedPointer<ctkPlugin> plugin);

  void addPlugin(QSharedPointer<ctkPlugin> plugin);

  bool delayRegistration;
  ctkPluginContext* context;

};

}

#endif // BERRYQCHPLUGINLISTENER_P_H
