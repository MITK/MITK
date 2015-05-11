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

#ifndef QMITKLEVELWINDOWRANGECHANGEDIALOG_H_
#define QMITKLEVELWINDOWRANGECHANGEDIALOG_H_

#include <MitkQtWidgetsExports.h>

#include "ui_QmitkLevelWindowRangeChange.h"

#include <QDialog>

/// \ingroup QmitkModule
class MITKQTWIDGETS_EXPORT QmitkLevelWindowRangeChangeDialog : public QDialog, public Ui::QmitkLevelWindowRangeChange
{
  Q_OBJECT

public:

  QmitkLevelWindowRangeChangeDialog(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

  int getLowerLimit();

  int getUpperLimit();

  void setLowerLimit( int rangeMin );

  void setUpperLimit( int rangeMax );

protected slots:

  void inputValidator();

};

#endif /*QMITKLEVELWINDOWRANGECHANGEDIALOG_H_*/
