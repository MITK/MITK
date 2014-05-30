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

#ifndef QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H
#define QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H

#include <QWidget>
#include "mitkNavigationDataPlayer.h"
#include "MitkIGTUIExports.h"

class QTimer;

namespace Ui {
  class QmitkNavigationDataPlayerControlWidget;
}

class MitkIGTUI_EXPORT QmitkNavigationDataPlayerControlWidget : public QWidget
{
  Q_OBJECT

signals:
  void SignalUpdate();
  void SignalEndReached();

public slots:
  void OnStop();
  void OnPlayPause();
  void OnRestart();

protected slots:
  void OnUpdate();

public:
  explicit QmitkNavigationDataPlayerControlWidget(QWidget *parent = 0);
  ~QmitkNavigationDataPlayerControlWidget();

  void SetPlayer(mitk::NavigationDataPlayer::Pointer player);

private:
  void ResetPlayerDisplay();

  mitk::NavigationDataPlayer::Pointer m_Player;

  QTimer*                             m_UpdateTimer;

  Ui::QmitkNavigationDataPlayerControlWidget *ui;
};

#endif // QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H
