#include "QmitknnUNetWorker.h"

#include "mitkRenderingManager.h"
#include <QMutexLocker>

void nnUNetSegmentationWorker::DoWork(mitk::nnUNetTool *tool)
{
  MITK_INFO << "in nnUNet Worker";
  try
  {
    QMutexLocker locker(&mutex);
    tool->UpdatePreview();
    if (tool->GetOutputBuffer() == nullptr)
    {
      mitkThrow() << "An error occured while calling nnUNet process.";
    }
    emit Finished(tool);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    emit Failed();
  }
}
