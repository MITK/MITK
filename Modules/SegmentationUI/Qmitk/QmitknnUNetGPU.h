/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetToolGPU_h_Included
#define QmitknnUNetToolGPU_h_Included

#include <QProcess>
#include <string>
#include <vector>

/**
 * @brief Struct to store GPU info.
 * 
 */
struct QmitkGPUSpec
{
  QString name;
  int memoryFree;
  unsigned int id;
};

/**
 * @brief Class to load and save GPU information
 * for further validation 
 */
class QmitkGPULoader : public QObject
{
  Q_OBJECT

private:
  std::vector<QmitkGPUSpec> gpus;

public:
  /**
   * @brief Construct a new Qmitk GPU Loader object.
   * Parses GPU info using `nvidia-smi` command and saves it as QmitkGPUSpec objects.
   */
  QmitkGPULoader();
  ~QmitkGPULoader() = default;

  /**
   * @brief Returns the number of GPUs parsed and saved as QmitkGPUSpec objects.
   * 
   * @return int 
   */
  int GetGPUCount();
};

#endif
