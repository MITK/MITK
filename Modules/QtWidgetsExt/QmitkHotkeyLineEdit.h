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


#ifndef QMITKHOTKEYLINEEDIT_H_
#define QMITKHOTKEYLINEEDIT_H_

#include <QLineEdit>
#include <QKeySequence>

#include "MitkQtWidgetsExtExports.h"

class MitkQtWidgetsExt_EXPORT QmitkHotkeyLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  static const std::string TOOLTIP;

  QmitkHotkeyLineEdit(QWidget* parent = 0);
  QmitkHotkeyLineEdit(const QKeySequence& _QKeySequence, QWidget* parent = 0);
  QmitkHotkeyLineEdit(const QString& _QString, QWidget* parent = 0);

  virtual void SetKeySequence(const QKeySequence& _QKeySequence);
  virtual void SetKeySequence(const QString& _QKeySequenceAsString);
  virtual QKeySequence GetKeySequence();
  virtual QString GetKeySequenceAsString();
  bool Matches( QKeyEvent * event );
protected slots:
  void LineEditTextChanged(const QString &);
protected:
  virtual void keyPressEvent ( QKeyEvent * event );
  void Init();
protected:
  QKeySequence m_KeySequence;

};

#endif

