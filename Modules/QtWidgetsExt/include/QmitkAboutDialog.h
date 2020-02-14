/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAboutDialog_h
#define QmitkAboutDialog_h

#include "MitkQtWidgetsExtExports.h"
#include <ui_QmitkAboutDialogGUI.h>

class MITKQTWIDGETSEXT_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkAboutDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
  ~QmitkAboutDialog() override;

  QString GetAboutText() const;
  QString GetCaptionText() const;
  QString GetRevisionText() const;

  void SetAboutText(const QString &text);
  void SetCaptionText(const QString &text);
  void SetRevisionText(const QString &text);

protected slots:
  void ShowModules();

private:
  Ui::QmitkAboutDialog m_GUI;
};

#endif
