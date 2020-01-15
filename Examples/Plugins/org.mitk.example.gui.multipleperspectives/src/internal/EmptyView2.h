/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  ~EmptyView2() override;

  void CreateQtPartControl(QWidget *parent) override;

protected:
  void SetFocus() override;

private:
  Ui::EmptyView2Controls m_Controls;

  QWidget *m_Parent;
};

#endif /*EMPTYVIEW2_H_*/
