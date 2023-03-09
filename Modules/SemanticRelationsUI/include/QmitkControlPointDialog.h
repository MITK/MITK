/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkControlPointDialog_h
#define QmitkControlPointDialog_h

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

  void SetCurrentDate(mitk::SemanticTypes::ControlPoint currentControlPoint);
  QDate GetCurrentDate() const;

private:

  QDateEdit* m_DateEdit;
};

#endif
