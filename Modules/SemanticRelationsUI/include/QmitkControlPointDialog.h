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

#ifndef QMITKCONTROLPOINTDIALOG_H
#define QMITKCONTROLPOINTDIALOG_H

#include <MitkSemanticRelationsUIExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"

#include <QDateEdit>
#include <QDialog>

class MITKSEMANTICRELATIONSUI_EXPORT QmitkControlPointDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkControlPointDialog(QWidget *parent = nullptr);

  virtual ~QmitkControlPointDialog();

  void SetCurrentDate(mitk::SemanticTypes::ControlPoint currentControlPoint);
  QDate GetCurrentDate() const;

private:

  QDateEdit* m_DateEdit;
};

#endif // QMITKCONTROLPOINTDIALOG_H
