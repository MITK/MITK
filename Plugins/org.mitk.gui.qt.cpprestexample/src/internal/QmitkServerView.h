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

#ifndef QmitkServerView_h
#define QmitkServerView_h

#include <QmitkAbstractView.h>
#include <mitkIRESTObserver.h>
#include "cpprest/uri.h"

namespace Ui
{
  class QmitkServerView;
}

namespace mitk
{
  class RESTManager;
}

class QmitkServerView : public QmitkAbstractView, public mitk::IRESTObserver
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkServerView();
  ~QmitkServerView() override;

  void CreateQtPartControl(QWidget *parent) override;
  web::json::value Notify(const web::uri &uri, const web::json::value &data) override;

private slots:
  void OnStopAllButtonClicked();
  void OnTestListenCheckBoxClicked();
  void OnExampleListenCheckBoxClicked();
  void OnPort8090ListenCheckBoxClicked();

private:
  void SetFocus() override;
  void StartListening(web::uri);
  void StopListening(web::uri);
  Ui::QmitkServerView *m_Ui;
};

#endif
