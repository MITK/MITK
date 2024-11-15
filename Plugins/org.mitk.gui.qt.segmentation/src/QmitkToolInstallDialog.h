/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToolInstallDialog_h
#define QmitkToolInstallDialog_h

#include "ui_QmitkToolInstallDialog.h"
#include <QDialog>
#include <QCloseEvent>
#include <QTextEdit>


//Subclass to stop QDialog from closing
class QmitkToolInstallDialog : public QDialog
{
  Q_OBJECT
public:
  explicit QmitkToolInstallDialog(QWidget *parent = nullptr);
  ~QmitkToolInstallDialog() = default;

  /**
   * @brief Overrides CloseEvent function of QDialog to open Messagebox
   */
  void closeEvent(QCloseEvent *event) override;

  /**
   * @brief Overrides reject function of QDialog to block closing via esc-key
   */
  void reject() override {}

  bool m_IsInstalling = false;
  Ui::QmitkToolInstallDialog *m_Ui;
  static QTextEdit *m_ConsoleOut; // for installation process output
};

#endif
