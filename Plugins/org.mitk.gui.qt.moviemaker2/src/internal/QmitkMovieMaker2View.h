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

#ifndef QmitkMovieMaker2View_h
#define QmitkMovieMaker2View_h

#include <QmitkAbstractView.h>
#include <ui_QmitkMovieMaker2ViewControls.h>

class QmitkMovieMaker2View : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMovieMaker2View();
  ~QmitkMovieMaker2View();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

private:
  Ui::QmitkMovieMaker2ViewControls m_Controls;
};

#endif
