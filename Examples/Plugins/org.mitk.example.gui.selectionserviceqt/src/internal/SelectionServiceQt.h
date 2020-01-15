/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef SELECTIONSERVICEQT_H_
#define SELECTIONSERVICEQT_H_

// berry includes
#include <berryIApplication.h>

// Qt includes
#include <QObject>
#include <QScopedPointer>

class SelectionServiceQtWorkbenchAdvisor;

/**
 * \ingroup org_mitk_example_gui_selectionserviceqt
 *
 * \brief This BlueBerry plugin is part of the BlueBerry example
 * "Selection service QT". It creates a perspective with two views
 * that demonstrate the Qt-based selection service.
 */
class SelectionServiceQt : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  SelectionServiceQt();
  ~SelectionServiceQt() override;

  QVariant Start(berry::IApplicationContext *context) override;
  void Stop() override;

private:
  QScopedPointer<SelectionServiceQtWorkbenchAdvisor> wbAdvisor;
};

#endif /*SELECTIONSERVICEQT_H_*/
