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

#ifndef QmitkRemeshingView_h
#define QmitkRemeshingView_h

#include <QmitkAbstractView.h>
#include <ui_QmitkRemeshingViewControls.h>

class QmitkRemeshingView : public QmitkAbstractView
{
  Q_OBJECT

public:
  QmitkRemeshingView();
  ~QmitkRemeshingView();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

private:
  Ui::QmitkRemeshingViewControls m_Controls;
};

#endif
