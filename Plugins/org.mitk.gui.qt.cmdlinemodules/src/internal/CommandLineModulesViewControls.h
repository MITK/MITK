/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

  CommandLineModulesViewControls(QWidget *parent = 0);
  virtual ~CommandLineModulesViewControls();

  void SetAdvancedWidgetsVisible(const bool& isVisible);

protected:

private:

};

#endif // CommandLineModulesViewControls_h
