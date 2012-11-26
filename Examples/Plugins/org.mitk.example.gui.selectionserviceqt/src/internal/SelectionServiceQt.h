/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  ~SelectionServiceQt();

  int Start();
  void Stop();

private:

  QScopedPointer<SelectionServiceQtWorkbenchAdvisor> wbAdvisor;
};

#endif /*SELECTIONSERVICEQT_H_*/
