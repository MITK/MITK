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


#ifndef EMPTYVIEW1_H_
#define EMPTYVIEW1_H_

// berry Includes
#include <berryQtViewPart.h>

// ui Includes
#include "ui_EmptyView1Controls.h"


class EmptyView1 : public berry::QtViewPart
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  EmptyView1();

  virtual ~EmptyView1();

  virtual void CreateQtPartControl(QWidget *parent);

private:


protected:

  void SetFocus();

  Ui::EmptyView1Controls m_Controls;

  QWidget* m_Parent;

};

#endif /*EMPTYVIEW1_H_*/
