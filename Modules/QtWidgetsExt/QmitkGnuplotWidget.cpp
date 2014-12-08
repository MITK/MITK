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

#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <ui_QmitkGnuplotWidget.h>
#include "QmitkGnuplotWidget.h"

QmitkGnuplotWidget::QmitkGnuplotWidget(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkGnuplotWidget),
    m_ContextMenu(NULL),
    m_CopyPlotAction(NULL),
    m_CopyScriptAction(NULL),
    m_Process(new QProcess(this))
{
  m_Ui->setupUi(this);

  connect(m_Process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(OnProcessStateChanged(QProcess::ProcessState)));
  connect(m_Process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnProcessError(QProcess::ProcessError)));
  connect(m_Process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnProcessFinished(int, QProcess::ExitStatus)));

  this->CreateContextMenu();
}

QmitkGnuplotWidget::~QmitkGnuplotWidget()
{
}

void QmitkGnuplotWidget::CreateContextMenu()
{
  m_CopyPlotAction = new QAction("Copy &Plot", this);
  connect(m_CopyPlotAction, SIGNAL(triggered()), this, SLOT(OnCopyPlot()));

  m_CopyScriptAction = new QAction("Copy &Script", this);
  connect(m_CopyScriptAction, SIGNAL(triggered()), this, SLOT(OnCopyScript()));

  m_ContextMenu = new QMenu(this);

  m_ContextMenu->addActions(QList<QAction*>()
    << m_CopyPlotAction
    << m_CopyScriptAction);
}

void QmitkGnuplotWidget::contextMenuEvent(QContextMenuEvent* event)
{
  const QPixmap* plot = m_Ui->label->pixmap();

  m_CopyPlotAction->setEnabled(plot != NULL && !plot->isNull());
  m_CopyScriptAction->setEnabled(!m_Commands.empty());

  m_ContextMenu->popup(event->globalPos());

  event->accept();
}

void QmitkGnuplotWidget::OnCopyPlot()
{
  const QPixmap* plot = m_Ui->label->pixmap();

  if (plot != NULL && !plot->isNull())
    QApplication::clipboard()->setPixmap(*plot);
}

void QmitkGnuplotWidget::OnCopyScript()
{
  if (m_Commands.empty())
    return;

  QString script = this->CreateSetTermCommand();

  Q_FOREACH(const QString& command, m_Commands)
  {
    script += command + "\n";
  }

  QApplication::clipboard()->setText(script);
}

void QmitkGnuplotWidget::resizeEvent(QResizeEvent*)
{
  m_ModifiedTime.Modified();

  if (m_Process->isOpen() || m_Commands.isEmpty() || m_GnuplotPath.isEmpty())
    return;

  this->Update();
}

QString QmitkGnuplotWidget::GetGnuplotPath() const
{
  return m_GnuplotPath;
}

void QmitkGnuplotWidget::SetGnuplotPath(const QString &path)
{
  m_GnuplotPath = path;
  m_ModifiedTime.Modified();
}

QStringList QmitkGnuplotWidget::GetCommands() const
{
  return m_Commands;
}

void QmitkGnuplotWidget::SetCommands(const QStringList& commands)
{
  m_Commands = commands;
  m_ModifiedTime.Modified();
}

void QmitkGnuplotWidget::Update()
{
  if (m_UpdateTime < m_ModifiedTime)
    m_Process->start(m_GnuplotPath, QStringList() << "-");
}

QSize QmitkGnuplotWidget::sizeHint() const
{
  return QSize(400, 300);
}

QString QmitkGnuplotWidget::CreateSetTermCommand() const
{
  return QString("set term pngcairo size %1,%2 enhanced font '%3,%4'\n")
    .arg(this->width())
    .arg(this->height())
    .arg(this->font().family())
    .arg(this->font().pointSize());
}

void QmitkGnuplotWidget::OnProcessStateChanged(QProcess::ProcessState state)
{
  if (state == QProcess::Running)
  {
    m_UpdateTime = m_ModifiedTime;

    m_Process->write(this->CreateSetTermCommand().toLatin1());

    Q_FOREACH(const QString& command, m_Commands)
    {
      m_Process->write(QString("%1\n").arg(command).toLatin1());
    }

    m_Process->write("exit\n");
    m_Process->closeWriteChannel();
  }
}

void QmitkGnuplotWidget::OnProcessError(QProcess::ProcessError error)
{
  switch (error)
  {
  case QProcess::FailedToStart:
    m_Ui->label->setText("Gnuplot failed to start!");
    break;

  case QProcess::Crashed:
    m_Ui->label->setText("Gnuplot crashed!");
    break;

  case QProcess::Timedout:
    m_Ui->label->setText("Gnuplot timed out!");
    break;

  case QProcess::WriteError:
    m_Ui->label->setText("Could not write to gnuplot!");
    break;

  case QProcess::ReadError:
    m_Ui->label->setText("Could not read from gnuplot!");
    break;

  default:
    m_Ui->label->setText("An unknown error occurred!");
    break;
  }
}

void QmitkGnuplotWidget::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  bool needUpdate = false;

  if (exitStatus != QProcess::CrashExit)
  {
    if (exitCode == 0)
    {
      if (m_UpdateTime < m_ModifiedTime)
      {
        needUpdate = true;
      }
      else
      {
        m_Ui->label->setPixmap(QPixmap::fromImage(QImage::fromData(m_Process->readAllStandardOutput(), "PNG")));
      }
    }
    else
    {
      m_Ui->label->setText(QString("Gnuplot exit code: %1!").arg(exitCode));
    }
  }

  m_Process->close();

  if (needUpdate)
    this->Update();
}
