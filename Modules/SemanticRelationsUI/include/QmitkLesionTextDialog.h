/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLesionTextDialog_h
#define QmitkLesionTextDialog_h

#include <MitkSemanticRelationsUIExports.h>

// semantic relations module
#include "mitkSemanticTypes.h"

#include <QLineEdit>
#include <QDialog>

class MITKSEMANTICRELATIONSUI_EXPORT QmitkLesionTextDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkLesionTextDialog(QWidget *parent = nullptr);

  void SetLineEditText(const std::string& lineEditText);
  QString GetLineEditText() const;

  QLineEdit* GetLineEdit() const;

private:

  QLineEdit* m_LineEdit;

};

#endif
