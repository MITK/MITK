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
#ifndef QMITKDILATETOOLGUI_H
#define QMITKDILATETOOLGUI_H

#include "QmitkMorphologicToolGUI.h"
#include "QmitkExtExports.h"

class /*QmitkExt_EXPORTS*/ QmitkDilateToolGUI : public QmitkMorphologicToolGUI
{
  Q_OBJECT
public:

  mitkClassMacro(QmitkDilateToolGUI, QmitkMorphologicToolGUI);
  itkNewMacro(QmitkDilateToolGUI);

protected:

  virtual ~QmitkDilateToolGUI();
  QmitkDilateToolGUI();

};

#endif
