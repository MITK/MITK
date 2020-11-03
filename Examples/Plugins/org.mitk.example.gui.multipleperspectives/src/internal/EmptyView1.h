/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef EMPTYVIEW1_H_
#define EMPTYVIEW1_H_

// berry includes
#include <berryQtViewPart.h>

// ui includes
#include "ui_EmptyView1Controls.h"

class EmptyView1 : public berry::QtViewPart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  EmptyView1();

  ~EmptyView1() override;

  void CreateQtPartControl(QWidget *parent) override;

protected:
  void SetFocus() override;

private:
  Ui::EmptyView1Controls m_Controls;

  QWidget *m_Parent;
};

#endif /*EMPTYVIEW1_H_*/
