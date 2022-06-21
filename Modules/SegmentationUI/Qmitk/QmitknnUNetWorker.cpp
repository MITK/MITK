#include "QmitknnUNetWorker.h"

#include "mitkRenderingManager.h"
#include <QMutexLocker>
#include <itksys/SystemTools.hxx>

void nnUNetDownloadWorker::DoWork(QString resultsFolder,
                                  QString pythonPath,
                                  mitk::ProcessExecutor::Pointer spExec,
                                  mitk::ProcessExecutor::ArgumentListType args)
{
  MITK_INFO << "in nnUNet Worker";
  try
  {
    QMutexLocker locker(&mutex);
    std::string resultsFolderEnv = "RESULTS_FOLDER=" + resultsFolder.toStdString();
    itksys::SystemTools::PutEnv(resultsFolderEnv.c_str());
    spExec->Execute(pythonPath.toStdString(), "nnUNet_download_pretrained_model", args);
    MITK_INFO << "in nnUNet Worker: download finished";
    emit Exit(true, QString("Download completed successfully."));
  }
  catch (const mitk::Exception &e)
  {
    emit Exit(false, QString::fromStdString(e.GetDescription()));
  }
}
