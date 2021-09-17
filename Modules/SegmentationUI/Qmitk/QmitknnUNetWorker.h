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

/**
 * @class nnUNetSegmentationWorker
 * @brief Class to execute some functions (mainly segmentation) from the nnUNet Tool in a seperate thread
 */
class nnUNetSegmentationWorker : public QObject
{
  Q_OBJECT

public slots:
  /**
   * @brief execute segmentation with the correct segmentation tool
   */
  void DoWork(mitk::nnUNetTool *);
signals:
  /**
   * @brief the signal emitted when a segmentation process finished successful
   */
  void Finished(mitk::nnUNetTool *);
  /**
   * @brief the signal emitted when a segmentation process failed
   */
  void Failed();

private:
  QMutex mutex;
};

#endif
