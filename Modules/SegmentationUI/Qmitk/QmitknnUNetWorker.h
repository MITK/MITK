/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnUNetWorker_h
#define QmitknnUNetWorker_h

#include "mitkProcessExecutor.h"
#include <QMutex>
#include <QObject>

Q_DECLARE_METATYPE(mitk::ProcessExecutor::Pointer);
Q_DECLARE_METATYPE(mitk::ProcessExecutor::ArgumentListType);

/**
 * @brief Class to execute some functions from the Segmentation Plugin in a seperate thread
 */
class nnUNetDownloadWorker : public QObject
{
  Q_OBJECT

public slots:
  /**
   * @brief Starts the download process worker thread.
   *
   */
  void DoWork(QString, QString, mitk::ProcessExecutor::Pointer, mitk::ProcessExecutor::ArgumentListType);

signals:

  /**
   * @brief the signal emitted when a download process is finshed; success or failed
   *
   * @param exitCode
   * @param message
   */
  void Exit(bool exitCode, const QString message);

private:
  QMutex mutex;
};

#endif
