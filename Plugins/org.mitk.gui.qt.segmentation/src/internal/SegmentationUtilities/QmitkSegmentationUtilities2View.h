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

#ifndef QmitkSegmentationUtilities2View_h
#define QmitkSegmentationUtilities2View_h

#include <QmitkAbstractView.h>
#include <ui_QmitkSegmentationUtilities2ViewControls.h>

class QmitkSegmentationUtilities2View : public QmitkAbstractView
{
  Q_OBJECT

public:
  QmitkSegmentationUtilities2View();
  ~QmitkSegmentationUtilities2View();

  void CreateQtPartControl(QWidget* parent);
  void SetFocus();

private:
  void AddUtilityWidget(QWidget* widget, const QIcon& icon, const QString& text);

  Ui::QmitkSegmentationUtilities2ViewControls m_Controls;
};

#endif
