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

#ifndef CommandLineModulesViewControls_h
#define CommandLineModulesViewControls_h

#include "ui_CommandLineModulesViewControls.h"

/**
 * \class CommandLineModulesViewControls
 * \brief Contains various utilities to remove the responsibility from CommandLineModulesView.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 */
class CommandLineModulesViewControls : public QWidget, public Ui_CommandLineModulesViewControls
{
  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

public:

  CommandLineModulesViewControls(QWidget *parent = 0);
  virtual ~CommandLineModulesViewControls();

signals:

protected slots:

protected:

private:

};

#endif // CommandLineModulesViewControls_h

