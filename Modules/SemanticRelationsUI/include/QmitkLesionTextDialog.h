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

#ifndef QMITKLESIONTEXTDIALOG_H
#define QMITKLESIONTEXTDIALOG_H

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

  virtual ~QmitkLesionTextDialog();

  void SetLineEditText(const std::string& lineEditText);
  QString GetLineEditText() const;

  QLineEdit* GetLineEdit() const;

private:

  QLineEdit* m_LineEdit;

};

#endif // QMITKLESIONTEXTDIALOG_H
