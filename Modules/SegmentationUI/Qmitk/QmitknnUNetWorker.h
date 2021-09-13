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

Q_DECLARE_METATYPE(mitk::nnUNetTool*)
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
   */
  void DoWork(mitk::nnUNetTool *, nnUNetModel *);
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
  void Finished(mitk::nnUNetTool *, nnUNetModel *);
  /**
   * @brief the signal emitted when a segmentation process failed
   */
  void Failed();

private:
  QMutex mutex;
};

#endif
