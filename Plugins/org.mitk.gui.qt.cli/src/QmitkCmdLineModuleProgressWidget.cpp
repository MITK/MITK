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

#include <ctkCmdLineModuleFuture.h>

#include "QmitkCmdLineModuleProgressWidget.h"
#include "ui_QmitkCmdLineModuleProgressWidget.h"


QmitkCmdLineModuleProgressWidget::QmitkCmdLineModuleProgressWidget(QWidget *parent)
  : QWidget(parent)
  , m_UI(new Ui::QmitkCmdLineModuleProgressWidget)
{
  m_UI->setupUi(this);

  m_UI->m_RemoveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));

  // Due to Qt bug 12152, we cannot listen to the "paused" signal because it is
  // not emitted directly when the QFuture is paused. Instead, it is emitted after
  // resuming the future, after the "resume" signal has been emitted... we use
  // a polling aproach instead.
  m_PollPauseTimer.setInterval(300);
  connect(&m_PollPauseTimer, SIGNAL(timeout()), SLOT(checkModulePaused()));

  connect(&m_FutureWatcher, SIGNAL(started()), SLOT(moduleStarted()));
  connect(&m_FutureWatcher, SIGNAL(canceled()), SLOT(moduleCanceled()));
  connect(&m_FutureWatcher, SIGNAL(finished()), SLOT(moduleFinished()));
  connect(&m_FutureWatcher, SIGNAL(resumed()), SLOT(moduleResumed()));
  connect(&m_FutureWatcher, SIGNAL(progressRangeChanged(int,int)), SLOT(moduleProgressRangeChanged(int,int)));
  connect(&m_FutureWatcher, SIGNAL(progressTextChanged(QString)), SLOT(moduleProgressTextChanged(QString)));
  connect(&m_FutureWatcher, SIGNAL(progressValueChanged(int)), SLOT(moduleProgressValueChanged(int)));

  connect(m_UI->m_CancelButton, SIGNAL(clicked()), &this->m_FutureWatcher, SLOT(cancel()));

  m_PollPauseTimer.start();
}

QmitkCmdLineModuleProgressWidget::~QmitkCmdLineModuleProgressWidget()
{
  delete m_UI;
}

void QmitkCmdLineModuleProgressWidget::setFuture(const ctkCmdLineModuleFuture &future)
{
  m_UI->m_PauseButton->setEnabled(future.canPause());
  m_UI->m_CancelButton->setEnabled(future.canCancel());
  m_UI->m_RemoveButton->setEnabled(!future.isRunning());

  m_FutureWatcher.setFuture(future);
}

void QmitkCmdLineModuleProgressWidget::setTitle(const QString &title)
{
  m_UI->m_ProgressTitle->setText(title);
}


void QmitkCmdLineModuleProgressWidget::mouseReleaseEvent(QMouseEvent*)
{
  emit clicked();
}

void QmitkCmdLineModuleProgressWidget::on_PauseButton_toggled(bool toggled)
{
  this->m_FutureWatcher.setPaused(toggled);
}

void QmitkCmdLineModuleProgressWidget::on_RemoveButton_clicked()
{
  this->deleteLater();
}

void QmitkCmdLineModuleProgressWidget::moduleStarted()
{
  this->m_UI->m_ProgressBar->setMaximum(0);
}

void QmitkCmdLineModuleProgressWidget::moduleCanceled()
{
  this->m_UI->m_PauseButton->setEnabled(false);
  this->m_UI->m_PauseButton->setChecked(false);
  this->m_UI->m_CancelButton->setEnabled(false);
  this->m_UI->m_RemoveButton->setEnabled(true);
}

void QmitkCmdLineModuleProgressWidget::moduleFinished()
{
  this->m_UI->m_PauseButton->setEnabled(false);
  this->m_UI->m_PauseButton->setChecked(false);
  this->m_UI->m_CancelButton->setEnabled(false);
  this->m_UI->m_RemoveButton->setEnabled(true);
}

void QmitkCmdLineModuleProgressWidget::checkModulePaused()
{
  if (this->m_FutureWatcher.future().isPaused())
  {
    if (!m_UI->m_PauseButton->isChecked())
    {
      m_UI->m_PauseButton->setChecked(true);
    }
  }
  else
  {
    if (m_UI->m_PauseButton->isChecked())
    {
      m_UI->m_PauseButton->setChecked(false);
    }
  }
}

void QmitkCmdLineModuleProgressWidget::moduleResumed()
{
  this->m_UI->m_PauseButton->setChecked(false);
}

void QmitkCmdLineModuleProgressWidget::moduleProgressRangeChanged(int progressMin, int progressMax)
{
  this->m_UI->m_ProgressBar->setMinimum(progressMin);
  this->m_UI->m_ProgressBar->setMaximum(progressMax);
}

void QmitkCmdLineModuleProgressWidget::moduleProgressTextChanged(const QString& progressText)
{
  //m_UI->m_ProgressText->setText(progressText);
}

void QmitkCmdLineModuleProgressWidget::moduleProgressValueChanged(int progressValue)
{
  m_UI->m_ProgressBar->setValue(progressValue);
}
