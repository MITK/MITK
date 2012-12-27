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

#ifndef QMITKLINEEDIT_H
#define QMITKLINEEDIT_H

#include <QLineEdit>

class QmitkLineEdit : public QLineEdit
{
  Q_OBJECT

  Q_PROPERTY(QString defaultText READ defaultText WRITE setDefaultText)

public:
  explicit QmitkLineEdit(QWidget *parent = NULL);
  explicit QmitkLineEdit(const QString &defaultText, QWidget *parent = NULL);
  ~QmitkLineEdit();

  QString defaultText() const;
  void setDefaultText(const QString &defaultText);

signals:
  void focusChanged(bool hasFocus);

protected:
  void focusInEvent(QFocusEvent *event);
  void focusOutEvent(QFocusEvent *event);

private slots:
  void onFocusChanged(bool hasFocus);
  void onTextChanged(const QString &text);

private:
  void initialize();
  void showDefaultText(bool show);

  QString m_DefaultText;
  QPalette m_Palette;
  QPalette m_DefaultPalette;
};

#endif
