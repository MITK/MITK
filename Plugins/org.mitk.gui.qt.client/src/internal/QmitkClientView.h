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

#ifndef QmitkClientView_h
#define QmitkClientView_h

#include <QmitkAbstractView.h>
#include "cpprest/uri.h"
#include <QString>
namespace Ui
{
  class QmitkClientView;
}

namespace mitk
{
  class RESTManager;
}

class QmitkClientView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkClientView();
  ~QmitkClientView() override;

  void CreateQtPartControl(QWidget *parent) override;

signals:
  void UpdateProgressBar();
  void UpdateLabel(QString);
private slots:
  void OnGetMultipleButtonClicked();
  void OnGetSingleButtonClicked();
  void OnGetSaveButtonClicked();
  void OnPutButtonClicked();
  void OnPostButtonClicked();
  void OnUpdateProgressBar();
  void OnUpdateLabel(QString text);

private:
  void SetFocus() override;
  Ui::QmitkClientView *m_Ui;
};

#endif
