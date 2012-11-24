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


#ifndef MINIMALVIEW_H_
#define MINIMALVIEW_H_

/// Berry
#include <berryQtViewPart.h>

#include <QString>

#include "ui_MinimalViewControls.h"


class MinimalView : public berry::QtViewPart
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  MinimalView();

  virtual ~MinimalView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void ChangeExtensionLabelText(QString s);

private:


protected:

  void SetFocus();

  Ui::MinimalViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*MINIMALVIEW_H_*/
