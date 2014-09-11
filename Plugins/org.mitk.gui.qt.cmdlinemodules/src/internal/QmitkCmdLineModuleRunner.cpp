/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCmdLineModuleRunner.h"
#include "ui_QmitkCmdLineModuleProgressWidget.h"

// Qt
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QTextBrowser>
#include <QByteArray>
#include <QApplication>
#include <QRegExp>

// CTK
#include <ctkCmdLineModuleFuture.h>
#include <ctkCmdLineModuleFutureWatcher.h>
#include <ctkCmdLineModuleManager.h>
#include <ctkCmdLineModuleFrontend.h>
#include <ctkCmdLineModuleDescription.h>
#include <ctkCollapsibleGroupBox.h>

// MITK
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>
#include <mitkExceptionMacro.h>
#include <QmitkCustomVariants.h>
#include "QmitkCmdLineModuleGui.h"


//-----------------------------------------------------------------------------
QmitkCmdLineModuleRunner::QmitkCmdLineModuleRunner(QWidget *parent)
  : QWidget(parent)
, m_ModuleManager(NULL)
, m_DataStorage(NULL)
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
QmitkCmdLineModuleRunner::~QmitkCmdLineModuleRunner()
{
  if (m_ModuleFrontEnd != NULL)
  {
    delete m_ModuleFrontEnd;
  }

  this->ClearUpTemporaryFiles();

  delete m_UI;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::SetManager(ctkCmdLineModuleManager* manager)
{
  this->m_ModuleManager = manager;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::SetDataStorage(mitk::DataStorage* dataStorage)
{
  this->m_DataStorage = dataStorage;
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::SetOutputDirectory(const QString& directoryName)
{
  this->m_OutputDirectoryName = directoryName;
}


//-----------------------------------------------------------------------------
QString QmitkCmdLineModuleRunner::GetTitle()
{
  assert(m_ModuleFrontEnd);

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  return description.title();
}


//-----------------------------------------------------------------------------
QString QmitkCmdLineModuleRunner::GetFullName() const
{
  assert(m_ModuleFrontEnd);

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  return description.categoryDotTitle();
}


//-----------------------------------------------------------------------------
QString QmitkCmdLineModuleRunner::GetValidNodeName(const QString& nodeName) const
{
  QString outputName = nodeName;

  // We will allow A-Z, a-z, 0-9, period, hyphen and underscore in the output file name.
  // This method is parsing a node name, and other bits of code add on a file extension .nii.
  // So, in the output string from this function, we should not allow period, so that
  // the second recommendation on this page:
  // http://www.boost.org/doc/libs/1_43_0/libs/filesystem/doc/portability_guide.htm
  // is still true.

  QRegExp rx("[A-Z|a-z|0-9|-|_]{1,1}");

  QString singleLetter;

  for (int i = 0; i < outputName.size(); i++)
  {
    if (i == 0 && outputName[i] == '-')
    {
      outputName[i] = '_';
    }

    singleLetter = outputName[i];

    if (!rx.exactMatch(singleLetter))
    {
      outputName[i] = '-';
    }
  }
  return outputName;
}


//-----------------------------------------------------------------------------
bool QmitkCmdLineModuleRunner::IsStarted() const
{
  bool isStarted = false;
  if (m_FutureWatcher != NULL && m_FutureWatcher->isStarted())
  {
    isStarted = true;
  }
  return isStarted;
}

//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnCheckModulePaused()
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
void QmitkCmdLineModuleRunner::OnPauseButtonToggled(bool toggled)
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
void QmitkCmdLineModuleRunner::OnRemoveButtonClicked()
{
  this->deleteLater();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnModuleStarted()
{
  this->m_UI->m_ProgressBar->setMaximum(0);

  QString message = "started.";
  this->PublishMessage(message);

  emit started();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnModuleCanceled()
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
void QmitkCmdLineModuleRunner::OnModuleFinished()
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
void QmitkCmdLineModuleRunner::OnModuleResumed()
{
  this->m_UI->m_PauseButton->setChecked(false);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnModuleProgressRangeChanged(int progressMin, int progressMax)
{
  this->m_UI->m_ProgressBar->setMinimum(progressMin);
  this->m_UI->m_ProgressBar->setMaximum(progressMax);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnModuleProgressTextChanged(const QString& progressText)
{
  this->m_UI->m_Console->appendPlainText(progressText);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnModuleProgressValueChanged(int progressValue)
{
  this->m_UI->m_ProgressBar->setValue(progressValue);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnOutputDataReady()
{
  m_OutputCount++;
  this->PublishByteArray(this->m_FutureWatcher->readPendingOutputData());
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::OnErrorDataReady()
{
  m_ErrorCount++;
  this->PublishByteArray(this->m_FutureWatcher->readPendingErrorData());
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::PublishMessage(const QString& message)
{
  QString prefix = ""; // Can put additional prefix here if needed.
  QString outputMessage = prefix + message;

  qDebug() << outputMessage;
  this->m_UI->m_Console->appendPlainText(outputMessage);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::PublishByteArray(const QByteArray& array)
{
  QString message = array.data();
  this->PublishMessage(message);
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::ClearUpTemporaryFiles()
{
  QString message;
  QString fileName;

  foreach (QTemporaryFile* file, m_TemporaryFiles)
  {
    assert(file != NULL);

    fileName = file->fileName();
    message = QObject::tr("removing %1").arg(fileName);
    this->PublishMessage(message);

    delete file;
  }

  m_TemporaryFiles.clear();
}


//-----------------------------------------------------------------------------
void QmitkCmdLineModuleRunner::LoadOutputData()
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
void QmitkCmdLineModuleRunner::SetFrontend(QmitkCmdLineModuleGui* frontEnd)
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
void QmitkCmdLineModuleRunner::Run()
{
  assert(m_ModuleManager);
  assert(m_DataStorage);
  assert(m_ModuleFrontEnd);

  m_OutputDataToLoad.clear();

  QString parameterName;
  QString message;
  QList<ctkCmdLineModuleParameter> parameters;

  ctkCmdLineModuleReference reference = m_ModuleFrontEnd->moduleReference();
  ctkCmdLineModuleDescription description = reference.description();

  // Check we have valid output. If at all possible, they should be somewhere writable.

  parameters = m_ModuleFrontEnd->parameters("image", ctkCmdLineModuleFrontend::Output);
  parameters << m_ModuleFrontEnd->parameters("file", ctkCmdLineModuleFrontend::Output);
  parameters << m_ModuleFrontEnd->parameters("geometry", ctkCmdLineModuleFrontend::Output);

  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();
    QString outputFileName = m_ModuleFrontEnd->value(parameterName, ctkCmdLineModuleFrontend::DisplayRole).toString();

    // Try to make sure we are not running in the application installation folder,
    // as more likely than not, it should not have write access, and you certainly
    // don't want users output files dumped there.
    //
    // eg. C:/Program Files (Windows), /Applications (Mac), /usr/local (Linux) etc.

    QFileInfo outputFileInfo(outputFileName);

    QString applicationDir = QApplication::applicationDirPath();
    QString outputDir = outputFileInfo.dir().absolutePath();

    if (applicationDir == outputDir)
    {
      qDebug() << "QmitkCmdLineModuleRunner::Run(), output folder = application folder, so will swap to defaultOutputDir, specified in CLI module preferences";

      QFileInfo newOutputFileInfo(m_OutputDirectoryName, outputFileInfo.fileName());
      QString newOutputFileAbsolutePath = newOutputFileInfo.absoluteFilePath();

      qDebug() << "QmitkCmdLineModuleRunner::Run(), swapping " << outputFileName << " to " << newOutputFileAbsolutePath;

      QMessageBox msgBox;
        msgBox.setText("The output directory is the same as the application installation directory");
        msgBox.setInformativeText(tr("Output file:\n%1\n\nwill be swapped to\n%2").arg(outputFileName).arg(newOutputFileAbsolutePath));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

      m_ModuleFrontEnd->setValue(parameterName, newOutputFileAbsolutePath, ctkCmdLineModuleFrontend::DisplayRole);
    }
  }

  // For each output image or file, store the filename, so we can auto-load it once the process finishes.
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

  // For each input image, write a temporary file as a Nifti image (TODO - iterate through list of file formats).
  // and then save the full path name back on the parameter.

  message = "Saving image data to temporary storage...";
  this->PublishMessage(message);

  parameters = m_ModuleFrontEnd->parameters("image", ctkCmdLineModuleFrontend::Input);
  foreach (ctkCmdLineModuleParameter parameter, parameters)
  {
    parameterName = parameter.name();

    QVariant tmp = m_ModuleFrontEnd->value(parameterName, ctkCmdLineModuleFrontend::UserRole);
    mitk::DataNode::Pointer node = tmp.value<mitkDataNodePtr>();

    if (node.IsNotNull())
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (image != NULL)
      {
        QString errorMessage;
        QTemporaryFile* tempFile = this->SaveTemporaryImage(parameter, node.GetPointer(), errorMessage);

        if(tempFile == NULL)
        {
          QMessageBox::warning(this, "Saving temporary file failed", errorMessage);
          return;
        }

        m_TemporaryFiles.push_back(tempFile);
        m_ModuleFrontEnd->setValue(parameterName, tempFile->fileName());

        message = "Saved " + tempFile->fileName();
        this->PublishMessage(message);

      } // end if image
    } // end if node
  } // end foreach input image

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

  // Give some immediate indication that we are running.
  m_UI->m_ProgressTitle->setText(description.title() + ": running");
}


//-----------------------------------------------------------------------------
QTemporaryFile* QmitkCmdLineModuleRunner::SaveTemporaryImage(const ctkCmdLineModuleParameter &parameter, mitk::DataNode::ConstPointer node, QString& errorMessage) const
{
  // Don't call this if node is null or node is not an image.
  assert(node.GetPointer());
  mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
  assert(image);

  QString intermediateError;
  QString intermediateErrors;

  QTemporaryFile *returnedFile = NULL;
  QString name = this->GetValidNodeName(QString::fromStdString(node->GetName()));
  QString fileNameTemplate = name + "_XXXXXX";

  // If no file extensions are specified, we default to .nii
  QStringList fileExts = parameter.fileExtensions();
  if (fileExts.isEmpty())
  {
    fileExts.push_back(".nii");
  }

  // Try each extension until we get a good one.
  foreach (QString extension, fileExts)
  {
    // File extensions may or may not include the leading dot, so add one if necessary.
    if (!extension.startsWith("."))
    {
      extension.prepend(".");
    }
    fileNameTemplate = fileNameTemplate + extension;

    try
    {
      QTemporaryFile *tempFile = new QTemporaryFile(QDir::tempPath() + QDir::separator() + fileNameTemplate);
      if (tempFile->open())
      {
        tempFile->close();
        if (mitk::IOUtil::SaveBaseData( image, tempFile->fileName().toStdString() ))
        {
          returnedFile = tempFile;
          break;
        }
        else
        {
          intermediateError = QObject::tr("Tried %1, failed to save image:\n%2\n").arg(extension).arg(tempFile->fileName());
        }
      }
      else
      {
        intermediateError = QObject::tr("Tried %1, failed to open file:\n%2\n").arg(extension).arg(tempFile->fileName());
      }
    }
    catch(const mitk::Exception &e)
    {
      intermediateError = QObject::tr("Tried %1, caught MITK Exception:\nDescription: %2\nFilename: %3\nLine: %4\n")
                          .arg(extension).arg(e.GetDescription()).arg(e.GetFile()).arg(e.GetLine());
    }
    catch(const std::exception& e)
    {
      intermediateError = QObject::tr("Tried %1, caught exception:\nDescription: %2\n")
                          .arg(extension).arg(e.what());
    }
    intermediateErrors += intermediateError;
  }
  errorMessage = intermediateErrors;
  return returnedFile;
}
