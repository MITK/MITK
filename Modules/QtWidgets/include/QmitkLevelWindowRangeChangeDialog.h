/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLevelWindowRangeChangeDialog_h
#define QmitkLevelWindowRangeChangeDialog_h

#include <MitkQtWidgetsExports.h>

#include "ui_QmitkLevelWindowRangeChange.h"

#include <QDialog>

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkLevelWindowRangeChangeDialog : public QDialog, public Ui::QmitkLevelWindowRangeChange
{
  Q_OBJECT

public:
  QmitkLevelWindowRangeChangeDialog(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  double getLowerLimit();

  double getUpperLimit();

  void setLowerLimit(double rangeMin);

  void setUpperLimit(double rangeMax);

protected slots:

  void inputValidator();
};

#endif
