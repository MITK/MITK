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


#ifndef EMPTYVIEW2_H_
#define EMPTYVIEW2_H_

// berry includes
#include <berryQtViewPart.h>

// ui includes
#include "ui_EmptyView2Controls.h"

class EmptyView2 : public berry::QtViewPart
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  EmptyView2();

  virtual ~EmptyView2();

  virtual void CreateQtPartControl(QWidget *parent);

protected:

  void SetFocus();

private:

  Ui::EmptyView2Controls m_Controls;

  QWidget* m_Parent;

};

#endif /*EMPTYVIEW2_H_*/
