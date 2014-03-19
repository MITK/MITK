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

#ifndef QmitkLineEdit_h
#define QmitkLineEdit_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCommon.h>
#include <QLineEdit>

/**
 * \deprecatedSince{2014_03} Use QLineEdit instead
 */
class MitkQtWidgetsExt_EXPORT QmitkLineEdit : public QLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString DefaultText READ GetDefaultText WRITE SetDefaultText FINAL)

public:

  /**
   * \deprecatedSince{2014_03} Use QLineEdit instead
   */
  DEPRECATED(explicit QmitkLineEdit(QWidget* parent = NULL));
  /**
   * \deprecatedSince{2014_03} Use QLineEdit instead
   */
  DEPRECATED(explicit QmitkLineEdit(const QString& defaultText, QWidget* parent = NULL));
  ~QmitkLineEdit();

  QString GetDefaultText() const;
  void SetDefaultText(const QString& defaultText);

protected:
  void focusInEvent(QFocusEvent* event);
  void focusOutEvent(QFocusEvent* event);

private:
  void Initialize();
  void ShowDefaultText(bool show);

signals:
  void FocusChanged(bool hasFocus);

private slots:
  void OnFocusChanged(bool hasFocus);
  void OnTextChanged(const QString& text);

private:
  QString m_DefaultText;
  QPalette m_DefaultPalette;
};

#endif
