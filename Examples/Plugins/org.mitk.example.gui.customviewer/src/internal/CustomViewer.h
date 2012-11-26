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
  ~CustomViewer();

/** Starts the application.*/
  int Start();

/** Exits the application.*/
  void Stop();

private:

/** The WorkbenchAdvisor for the CustomViewer application.*/
  QScopedPointer<CustomViewerWorkbenchAdvisor> wbAdvisor;
};

#endif /*CUSTOMVIEWER_H_*/
