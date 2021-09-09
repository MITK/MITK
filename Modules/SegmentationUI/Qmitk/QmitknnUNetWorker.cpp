#include "QmitknnUNetWorker.h"
#include "mitkRenderingManager.h"


void nnUNetSegmentationWorker::DoWork(mitk::nnUNetTool *tool, nnUNetModel* request)
{
  // connect signals/slots with the result setter which sets the result in the main thread afterwards
  // connect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
  // connect(this, &SegmentationWorker::FinishedMultilabel, resultSetter,
  // &SegmentationResultHandler::SetMultilabelResult); connect(this, &SegmentationWorker::Failed, resultSetter,
  // &SegmentationResultHandler::SegmentationProcessFailed);
  std::cout << "in do work" << std::endl;
  try
  {
    mutex.lock();
    tool->UpdatePreview();
    mutex.unlock();
    MITK_INFO << "Back in Worker";
    // emit Failed();
    emit Finished(tool, request);

    // disconnect from result setter. Otherwise, the result is set twice after second execution,
    // three times after third execution,...
    // disconnect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
    // disconnect(this, &SegmentationWorker::FinishedMultilabel, resultSetter,
    // &SegmentationResultHandler::SetMultilabelResult); disconnect(this, &SegmentationWorker::Failed, resultSetter,
    // &SegmentationResultHandler::SegmentationProcessFailed);
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    // emit Failed();
    // disconnect from result setter. Otherwise, the result is set twice after second execution,
    // three times after third execution,...
    // disconnect(this, &SegmentationWorker::Finished, resultSetter, &SegmentationResultHandler::SetResult);
    // disconnect(this, &SegmentationWorker::FinishedMultilabel, resultSetter,
    // &SegmentationResultHandler::SetMultilabelResult); disconnect(this, &SegmentationWorker::Failed, resultSetter,
    // &SegmentationResultHandler::SegmentationProcessFailed);
  }
}
