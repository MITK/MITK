/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkHotkeyLineEdit_h
#define QmitkHotkeyLineEdit_h

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

  void keyPressEvent(QKeyEvent* event) override;
  void Init();

  QKeySequence m_KeySequence;

};

#endif
