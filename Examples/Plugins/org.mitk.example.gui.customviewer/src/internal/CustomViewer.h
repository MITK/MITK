/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CUSTOMVIEWER_H_
#define CUSTOMVIEWER_H_

#include <berryIApplication.h>

/// Qt
#include <QObject>
#include <QScopedPointer>

class CustomViewerWorkbenchAdvisor;

/** Documentation
*   \ingroup org_mitk_example_gui_customviewer
*
*   \brief A blueberry application class as an entry point for the custom viewer plug-in.
*
*   This class acts as an entry point application class for the dedicated custom viewer plug-in.
*/
class CustomViewer : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  /** Standard constructor.*/
  CustomViewer();

  /** Standard destructor.*/
  ~CustomViewer() override;

  /** Starts the application.*/
  QVariant Start(berry::IApplicationContext *context) override;

  /** Exits the application.*/
  void Stop() override;
};

#endif /*CUSTOMVIEWER_H_*/
