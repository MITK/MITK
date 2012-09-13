/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
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
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QTextBrowser>
#include <QByteArray>

// CTK
#include <ctkCmdLineModuleFuture.h>
#include <ctkCmdLineModuleFutureWatcher.h>
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleFrontend.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCmdLineModuleParameter.h>
#include <ctkCollapsibleGroupBox.h>

// MITK
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <QmitkCommonFunctionality.h>
#include <QmitkCustomVariants.h>
#include "QmitkCmdLineModuleGui.h"

//-----------------------------------------------------------------------------
QmitkCmdLineModuleProgressWidget::QmitkCmdLineModuleProgressWidget(QWidget *parent)
  : QWidget(parent)
, m_ModuleManager(NULL)
, m_DataStorage(NULL)
, m_TemporaryDirectoryName("")
, m_UI(new Ui::QmitkCmdLineModuleProgressWidget)
, m_Layout(NULL)
, m_ModuleFrontEnd(NULL)
, m_FutureWatcher(NULL)
{
  m_UI->setupUi(this);
  m_UI->m_RemoveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));

  m_Layout = new QVBoxLayout();
  m_Layout->setContentsMargins(0,0,0,0);
  m_Layout->setSpacing(0);
  m_UI->m_ParametersGroupBox->setLayout(m_Layout);

  qRegisterMetaType<mitk::DataNode::Pointer>();
  qRegisterMetaType<ctkCmdLineModuleReference>();

  connect(m_UI->m_RemoveButton, SIGNAL(clicked()), this, SLOT(OnRemoveButtonClicked()));

  // Due to Qt bug 12152, we cannot listen to the "paused" signal because it is
  // not emitted directly when the QFuture is paused. Instead, it is emitted after
  // resuming the future, after the "resume" signal has been emitted... we use
  // a polling approach instead.
  connect(&m_PollPauseTimer, SIGNAL(timeout()), SLOT(OnCheckModulePaused()));
  m_PollPauseTimer.setInterval(300);
  m_PollPauseTimer.start();
}


//-----------------------------------------------------------------------------
QmitkCmdLineModuleProgressWidget::~QmitkCmdLineModuleProgressWidget()
{
  if (m_ModuleFrontEnd != NULL)
  {
    delete m_ModuleFrontEnd;
  }

  this->ClearUpTemporaryFiles();

  delete m_UI;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::SetManager(ctkCmdLineModuleManager* manager)
{
  this->m_ModuleManager = manager;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  this->m_DataStorage = dataStorage;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::SetTemporaryDirectory(const QString& directoryName)
{
  this->m_TemporaryDirectoryName = directoryName;
}


//-----------------------------------------------------------------------------
QString QmitkCmdLineModuleProgressWidget::GetTitle()
{
  assert(m_ModuleFrontEnd);

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  return description.title();
}


//-----------------------------------------------------------------------------
QString QmitkCmdLineModuleProgressWidget::GetFullName() const
{
  assert(m_ModuleFrontEnd);

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  return description.categoryDotTitle();
}


//-----------------------------------------------------------------------------
bool QmitkCmdLineModuleProgressWidget::IsStarted() const
{
  bool isStarted = false;
  if (m_FutureWatcher != NULL && m_FutureWatcher->isStarted())
  {
    isStarted = true;
  }
  return isStarted;
}

//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnCheckModulePaused()
{
  if (!this->IsStarted())
  {
    return;
  }

  if (this->m_FutureWatcher->future().isPaused())
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


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnPauseButtonToggled(bool toggled)
{
  this->m_FutureWatcher->setPaused(toggled);

  if (toggled)
  {
    this->m_UI->m_ProgressTitle->setText(this->GetTitle() + ": paused");
  }
  else
  {
    this->m_UI->m_ProgressTitle->setText(this->GetTitle() + ": resumed");
  }
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnRemoveButtonClicked()
{
  this->deleteLater();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleStarted()
{
  this->m_UI->m_ProgressBar->setMaximum(0);

  QString message = "started.";
  this->PublishMessage(message);

  emit started();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleCanceled()
{
  QString message = "cancelling.";
  this->PublishMessage(message);

  this->m_UI->m_PauseButton->setEnabled(false);
  this->m_UI->m_PauseButton->setChecked(false);
  this->m_UI->m_CancelButton->setEnabled(false);
  this->m_UI->m_RemoveButton->setEnabled(true);

  this->m_UI->m_ParametersGroupBox->setCollapsed(true);
  this->m_UI->m_ConsoleGroupBox->setCollapsed(true);
  this->m_UI->m_ProgressTitle->setText(this->GetTitle() + ": cancelled");

  message = "cancelled.";
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleFinished()
{
  this->m_UI->m_PauseButton->setEnabled(false);
  this->m_UI->m_PauseButton->setChecked(false);
  this->m_UI->m_CancelButton->setEnabled(false);
  this->m_UI->m_RemoveButton->setEnabled(true);

  if (!this->m_FutureWatcher->isCanceled())
  {
    QString message = "finishing.";
    this->PublishMessage(message);

    // If no incremental results from stdout, try getting hold of the whole buffer and printing it.
    if (m_OutputCount == 0)
    {
      message = "Output channel is:";
      this->PublishMessage(message);
      this->PublishByteArray(this->m_FutureWatcher->readAllOutputData());
    }

    // If no incremental results from stderr, try getting hold of the whole buffer and printing it.
    if (m_ErrorCount == 0)
    {
      message = "Error channel is:";
      this->PublishMessage(message);
      this->PublishByteArray(this->m_FutureWatcher->readAllErrorData());
    }

    this->m_UI->m_ProgressTitle->setText(this->GetTitle() + ": finished");

    this->LoadOutputData();
    this->ClearUpTemporaryFiles();

    message = "finished.";
    this->PublishMessage(message);

  }

  emit finished();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleResumed()
{
  this->m_UI->m_PauseButton->setChecked(false);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleProgressRangeChanged(int progressMin, int progressMax)
{
  this->m_UI->m_ProgressBar->setMinimum(progressMin);
  this->m_UI->m_ProgressBar->setMaximum(progressMax);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleProgressTextChanged(const QString& progressText)
{
  this->m_UI->m_Console->appendPlainText(progressText);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnModuleProgressValueChanged(int progressValue)
{
  this->m_UI->m_ProgressBar->setValue(progressValue);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnOutputDataReady()
{
  m_OutputCount++;
  this->PublishByteArray(this->m_FutureWatcher->readPendingOutputData());
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::OnErrorDataReady()
{
  m_ErrorCount++;
  this->PublishByteArray(this->m_FutureWatcher->readPendingErrorData());
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::PublishMessage(const QString& message)
{
  QString prefix = ""; // Can put additional prefix here if needed.
  QString outputMessage = prefix + message;

  qDebug() << outputMessage;
  this->m_UI->m_Console->appendPlainText(outputMessage);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::PublishByteArray(const QByteArray& array)
{
  QString message = array.data();
  this->PublishMessage(message);
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
void QmitkCmdLineModuleProgressWidget::SetFrontend(QmitkCmdLineModuleGui* frontEnd)
{
  assert(frontEnd);
  assert(m_ModuleManager);
  assert(m_DataStorage);

  // We are assuming that this method is ONLY EVER CALLED ONCE.
  assert(!m_ModuleFrontEnd);

  // Assign the frontEnd to the member variable.
  m_ModuleFrontEnd = frontEnd;

  // We put the new GUI into the layout.
  m_Layout->insertWidget(0, m_ModuleFrontEnd->getGui());

  // And configure a few other niceties.
  m_UI->m_ProgressTitle->setText(this->GetTitle());
  m_UI->m_ConsoleGroupBox->setCollapsed(true);     // We basically call SetFrontend then Run
  m_UI->m_ParametersGroupBox->setCollapsed(true);  // so in practice the user will only want the progress bar.
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleProgressWidget::Run()
{
  assert(m_ModuleManager);
  assert(m_DataStorage);
  assert(m_ModuleFrontEnd);

  m_OutputDataToLoad.clear();

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  QString message = "Saving image data to temporary storage...";
  this->PublishMessage(message);

  // Sanity check we have actually specified some input:
  QString parameterName;
  QList<ctkCmdLineModuleParameter> parameters;

  // For each input image, write a temporary file as a Nifti image,
  // and then save the full path name back on the parameter.
  parameters = m_ModuleFrontEnd->parameters("image", ctkCmdLineModuleFrontend::Input);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();

    QVariant tmp = m_ModuleFrontEnd->value(parameterName, ctkCmdLineModuleFrontend::UserRole);
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
        m_ModuleFrontEnd->setValue(parameterName, temporaryStorageFileName);

        message = "Saved " + temporaryStorageFileName;
        this->PublishMessage(message);
      } // end if image
    } // end if node
  } // end foreach input image

  // For each output image or file, store the filename, so we can auto-load it once the process finishes.
  parameters = m_ModuleFrontEnd->parameters("image", ctkCmdLineModuleFrontend::Output);
  parameters << m_ModuleFrontEnd->parameters("file", ctkCmdLineModuleFrontend::Output);

  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();
    QString outputFileName = m_ModuleFrontEnd->value(parameterName, ctkCmdLineModuleFrontend::DisplayRole).toString();

    if (!outputFileName.isEmpty())
    {
      m_OutputDataToLoad.push_back(outputFileName);

      message = "Registered " + outputFileName + " to auto load upon completion.";
      this->PublishMessage(message);
    }
  }

  m_OutputCount = 0;
  m_ErrorCount = 0;

  // Now we run stuff.
  message = "starting.";
  this->PublishMessage(message);

  if (m_FutureWatcher == NULL)
  {
    m_FutureWatcher = new ctkCmdLineModuleFutureWatcher();

    connect(m_FutureWatcher, SIGNAL(started()), SLOT(OnModuleStarted()));
    connect(m_FutureWatcher, SIGNAL(canceled()), SLOT(OnModuleCanceled()));
    connect(m_FutureWatcher, SIGNAL(finished()), SLOT(OnModuleFinished()));
    connect(m_FutureWatcher, SIGNAL(resumed()), SLOT(OnModuleResumed()));
    connect(m_FutureWatcher, SIGNAL(progressRangeChanged(int,int)), SLOT(OnModuleProgressRangeChanged(int,int)));
    connect(m_FutureWatcher, SIGNAL(progressTextChanged(QString)), SLOT(OnModuleProgressTextChanged(QString)));
    connect(m_FutureWatcher, SIGNAL(progressValueChanged(int)), SLOT(OnModuleProgressValueChanged(int)));
    connect(m_FutureWatcher, SIGNAL(outputDataReady()), SLOT(OnOutputDataReady()));
    connect(m_FutureWatcher, SIGNAL(errorDataReady()), SLOT(OnErrorDataReady()));

    connect(m_UI->m_CancelButton, SIGNAL(clicked()), m_FutureWatcher, SLOT(cancel()));
    connect(m_UI->m_PauseButton, SIGNAL(toggled(bool)), this, SLOT(OnPauseButtonToggled(bool)));

  }
  ctkCmdLineModuleFuture future = m_ModuleManager->run(m_ModuleFrontEnd);
  m_FutureWatcher->setFuture(future);

  m_UI->m_PauseButton->setEnabled(future.canPause());
  m_UI->m_CancelButton->setEnabled(future.canCancel());
  m_UI->m_RemoveButton->setEnabled(!future.isRunning());

  // Lock parameters, as once the module is running the user can't change them.
  m_ModuleFrontEnd->lockGui();

  // Give some immediate indication that we are running.
  m_UI->m_ProgressTitle->setText(description.title() + ": running");
}
