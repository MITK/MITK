/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  ~SelectionServiceMitk() override;

  QVariant Start(berry::IApplicationContext *context) override;
  void Stop() override;

private:
  QScopedPointer<SelectionServiceMITKWorkbenchAdvisor> wbAdvisor;
};

#endif /*SELECTIONSERVICEMITK_H_*/
