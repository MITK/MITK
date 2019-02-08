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

#ifndef QmitkThreadView_h
#define QmitkThreadView_h

#include <QmitkAbstractView.h>
#include <mitkIRESTObserver.h>


namespace Ui
{
  class QmitkThreadView;
}

namespace mitk
{
  class RESTManager;
}

class QmitkThreadView : public QmitkAbstractView, public mitk::IRESTObserver
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkThreadView();
  ~QmitkThreadView() override;

  void CreateQtPartControl(QWidget *parent) override;
  web::json::value Notify(web::json::value data) override;

private slots:
  void OnStartButtonClicked();
  void OnStopButtonClicked();

private:
  void SetFocus() override;

  Ui::QmitkThreadView *m_Ui;
};

#endif
