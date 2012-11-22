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

#ifndef SELECTIONSERVICEMITK_H_
#define SELECTIONSERVICEMITK_H_

// berry includes
#include <berryIApplication.h>

// Qt includes
#include <QObject>
#include <QScopedPointer>

class SelectionServiceMITKWorkbenchAdvisor;

/**
 * \ingroup org_mitk_example_gui_selectionservicemitk
 *
 * \brief This BlueBerry plugin is part of the BlueBerry example
 * "Selection service MITK". It creates a perspective with two views
 * that demonstrate the MitkDataNode-based selection service.
 */
class SelectionServiceMitk : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  SelectionServiceMitk();
  ~SelectionServiceMitk();

  int Start();
  void Stop();

private:

  QScopedPointer<SelectionServiceMITKWorkbenchAdvisor> wbAdvisor;
};

#endif /*SELECTIONSERVICEMITK_H_*/
