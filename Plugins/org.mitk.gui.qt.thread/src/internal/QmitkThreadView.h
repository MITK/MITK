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
#include "cpprest/uri.h"

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
  web::json::value Notify(web::json::value &data, const web::uri &uri) override;

private slots:
  void OnStopAllButtonClicked();
  void OnTestListenCheckBoxClicked();
  void OnExampleListenCheckBoxClicked();
  void OnPort8090ListenCheckBoxClicked();

private:
  void SetFocus() override;
  void StartListening(web::uri);
  void StopListening(web::uri);
  Ui::QmitkThreadView *m_Ui;
};

#endif
