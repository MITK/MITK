/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CommandLineModulesViewControls_h
#define CommandLineModulesViewControls_h

#include "ui_CommandLineModulesViewControls.h"

class QHBoxLayout;

/**
 * \class CommandLineModulesViewControls
 * \brief Class derived from Ui_CommandLineModulesViewControls to provide access to GUI widgets.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
class CommandLineModulesViewControls : public QWidget, public Ui_CommandLineModulesViewControls
{
  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  CommandLineModulesViewControls(QWidget *parent = nullptr);
  ~CommandLineModulesViewControls() override;

  void SetAdvancedWidgetsVisible(const bool& isVisible);

protected:

private:

};

#endif // CommandLineModulesViewControls_h
