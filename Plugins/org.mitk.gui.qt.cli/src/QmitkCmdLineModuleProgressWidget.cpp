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

#include "QmitkCmdLineModuleProgressWidget.h"
#include "ui_QmitkCmdLineModuleProgressWidget.h"

// Qt
#include <QFile>

// CTK
#include <ctkCmdLineModuleFuture.h>
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleFrontend.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleParameter.h>

// MITK
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <QmitkCommonFunctionality.h>
#include <QmitkCustomVariants.h>

QmitkCmdLineModuleProgressWidget::QmitkCmdLineModuleProgressWidget(QWidget *parent)
  : QWidget(parent)
  , m_UI(new Ui::QmitkCmdLineModuleProgressWidget)
{
  qRegisterMetaType<mitk::DataNode::Pointer>();
  qRegisterMetaType<ctkCmdLineModuleReference>();

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
  this->ClearUpTemporaryFiles();
  delete m_UI;
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
  QString message = "finishing.";
  this->PublishMessage(message);

  this->m_UI->m_PauseButton->setEnabled(false);
  this->m_UI->m_PauseButton->setChecked(false);
  this->m_UI->m_CancelButton->setEnabled(false);
  this->m_UI->m_RemoveButton->setEnabled(true);

  this->LoadOutputData();
  this->ClearUpTemporaryFiles();

  message = "finished.";
  this->PublishMessage(message);
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

//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::PublishMessage(const QString& message)
{
  QString prefix = ""; // Can put additional prefix here if needed.
  qDebug() << prefix << message;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::ClearUpTemporaryFiles()
{
  QString message;
  QString fileName;

  foreach (fileName, m_TemporaryFileNames)
  {
    QFile file(fileName);
    if (file.exists())
    {
      message = QObject::tr("removing %1").arg(fileName);
      this->PublishMessage(message);

      bool success = file.remove();

      message = QObject::tr("removed %1, successfully=%2").arg(fileName).arg(success);
      this->PublishMessage(message);
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::LoadOutputData()
{
  assert(m_DataStorage);

  std::vector<std::string> fileNames;

  QString fileName;
  foreach (fileName, m_OutputDataToLoad)
  {
    QString message = QObject::tr("loading %1").arg(fileName);
    this->PublishMessage(message);

    fileNames.push_back(fileName.toStdString());
  }

  if (fileNames.size() > 0)
  {
    int numberLoaded = mitk::IOUtil::LoadFiles(fileNames, *(m_DataStorage));

    QString message = QObject::tr("loaded %1 files").arg(numberLoaded);
    this->PublishMessage(message);
  }
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::Run(ctkCmdLineModuleFrontend* moduleInstance)
{
  assert(m_ModuleManager);

  if (!moduleInstance)
  {
    qWarning() << "Invalid module instance";
    return;
  }

  m_OutputDataToLoad.clear();
  ctkCmdLineModuleReference reference = moduleInstance->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  QString message = "Saving image data to temporary storage...";
  this->PublishMessage(message);

  // Sanity check we have actually specified some input:
  QString parameterName;
  QList<ctkCmdLineModuleParameter> parameters;

  // For each input image, write a temporary file as a Nifti image,
  // and then save the full path name back on the parameter.
  parameters = moduleInstance->parameters("image", ctkCmdLineModuleFrontend::Input);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();

    QVariant tmp = moduleInstance->value(parameterName, ctkCmdLineModuleFrontend::UserRole);
    mitk::DataNode::Pointer node = tmp.value<mitk::DataNode::Pointer>();

    if (node.IsNotNull())
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image != NULL)
      {
        QString name = QString::fromStdString(node->GetName());
        int pid = QCoreApplication::applicationPid();
        int randomInt = qrand() % 1000000;

        QString fileName = m_TemporaryDirectoryName + "/" + name + QString::number(pid) + "." + QString::number(randomInt) + ".nii";

        message = "Saving " + fileName;
        this->PublishMessage(message);

        std::string tmpFN = CommonFunctionality::SaveImage(image, fileName.toStdString().c_str());
        QString temporaryStorageFileName = QString::fromStdString(tmpFN);

        m_TemporaryFileNames.push_back(temporaryStorageFileName);
        moduleInstance->setValue(parameterName, temporaryStorageFileName);

        message = "Saved " + temporaryStorageFileName;
        this->PublishMessage(message);
      } // end if image
    } // end if node
  } // end foreach input image

  // For each output image or file, store the filename, so we can auto-load it once the process finishes.
  parameters = moduleInstance->parameters("image", ctkCmdLineModuleFrontend::Output);
  parameters << moduleInstance->parameters("file", ctkCmdLineModuleFrontend::Output);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();
    QString outputFileName = moduleInstance->value(parameterName).toString();

    if (!outputFileName.isEmpty())
    {
      m_OutputDataToLoad.push_back(outputFileName);

      message = "Command Line Module ... Registered " + outputFileName + " to auto load upon completion.";
      this->PublishMessage(message);
    }
  }

  // Now we run stuff.
  message = "starting.";
  this->PublishMessage(message);

  ctkCmdLineModuleFuture future = m_ModuleManager->run(moduleInstance);
  m_FutureWatcher.setFuture(future);

  m_UI->m_PauseButton->setEnabled(future.canPause());
  m_UI->m_CancelButton->setEnabled(future.canCancel());
  m_UI->m_RemoveButton->setEnabled(!future.isRunning());
}
