/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetWorker_h
#define QmitknnUNetWorker_h

#include <QMutex>
#include <QObject>
#include <mitknnUnetTool.h>

// Q_DECLARE_METATYPE(mitk::DeepLearningSegmentationTool*)
// Q_DECLARE_METATYPE(mitk::LabelSetImage*)
// Q_DECLARE_METATYPE(std::vector<mitk::LabelSetImage::Pointer>)

class nnUNetModel
{
public:
  std::vector<mitk::ModelParams> requestQ;
  mitk::LabelSetImage::ConstPointer outputImage;
  size_t hashCode = 0;

  size_t GetUniqueHash()
  {
    if (hashCode == 0)
    {
      for (mitk::ModelParams request : requestQ)
      {
        // sum of individual hash is the final hash
        hashCode += request.generateHash();
      }
    }
    return hashCode;
  }
};

/**
 * @class SegmentationWorker
 * @brief Class to execute some functions (mainly segmentation) from the Segmentation Plugin in a seperate thread
 */
class nnUNetSegmentationWorker : public QObject
{
  Q_OBJECT

public slots:
  /**
   * @brief execute segmentation with the correct segmentation tool
   *
   * @param segTool the Segmentation Tool for running the segmentation
   * @param resultSetter the SegmentationResultHandler which sets the result in the GUI after the segmentation
   * @param networkPath the path to the trained network which is needed for the segmentation
   */
  void DoWork(mitk::nnUNetTool *, nnUNetModel*);
  /**
   * @brief if a segmentation is executed when the tool is started,
   *        wait for the segmentation to finish and emit a signal (PreviouesSegmentationFinished) afterwards.
   *
   * @param tool the Segmentation Tool to check, if the segmentation is still running.
   */
  // void WaitForSegmentationToFinish(mitk::nnUNetTool *segTool);

signals:
  /**
   * @brief the signal emitted when a segmentation process finished successful
   *
   * @param result the resulting segmentation
   * @param segTool the Segmentation Tool for running the segmentation
   */
  void Finished(mitk::nnUNetTool *, nnUNetModel*);
  /**
   * @brief the signal emitted when a multilabel segmentation process finished successful
   *
   * @param result the resulting segmentation
   * @param segTool the Segmentation Tool for running the segmentation
   */
  // void FinishedMultilabel(std::vector<mitk::LabelSetImage::Pointer> result, mitk::DeepLearningSegmentationTool
  // *segTool);
  /**
   * @brief the signal emitted when a segmentation process failed
   */
  // void Failed();
  /**
   * @brief the signal emitted when a segmentation, which ran when the tool was started, finished
   */
  // void PreviousSegmentationFinished();

private:
  QMutex mutex;
};

#endif
