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

#include <ui_QmitkAboutDialogGUI.h>
#include "MitkQtWidgetsExtExports.h"

class MitkQtWidgetsExt_EXPORT QmitkAboutDialog : public QDialog
{
  Q_OBJECT

public:
  QmitkAboutDialog(QWidget* parent = 0, Qt::WindowFlags f = Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
  virtual ~QmitkAboutDialog();

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

