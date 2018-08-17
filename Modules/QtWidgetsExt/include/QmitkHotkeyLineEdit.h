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

#ifndef QMITKHOTKEYLINEEDIT_H
#define QMITKHOTKEYLINEEDIT_H

#include "MitkQtWidgetsExtExports.h"

// qt
#include <QKeySequence>
#include <QLineEdit>

class MITKQTWIDGETSEXT_EXPORT QmitkHotkeyLineEdit : public QLineEdit
{
  Q_OBJECT

public:

  static const std::string TOOLTIP;

  QmitkHotkeyLineEdit(QWidget* parent = nullptr);
  QmitkHotkeyLineEdit(const QKeySequence& qKeySequence, QWidget* parent = nullptr);
  QmitkHotkeyLineEdit(const QString&  qQString, QWidget* parent = nullptr);

  virtual void SetKeySequence(const QKeySequence& qKeySequence);
  virtual void SetKeySequence(const QString& qKeySequenceAsString);
  virtual QKeySequence GetKeySequence();
  virtual QString GetKeySequenceAsString();
  bool Matches(QKeyEvent *event);

protected Q_SLOTS:

  void LineEditTextChanged(const QString&);

protected:

  virtual void keyPressEvent(QKeyEvent* event) override;
  void Init();

  QKeySequence m_KeySequence;

};

#endif // QMITKHOTKEYLINEEDIT_H
