/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnUNetGPU_h
#define QmitknnUNetGPU_h

#include <vector>
#include <QString>
#include <MitkSegmentationUIExports.h>

/**
 * @brief Struct to store GPU info.
 *
 */
struct QmitkGPUSpec
{
  QString name;
  QString memory;
  unsigned int id;
};

/**
 * @brief Class to load and save GPU information
 * for further validation
 */
class MITKSEGMENTATIONUI_EXPORT QmitkGPULoader
{
private:
  std::vector<QmitkGPUSpec> m_Gpus;

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
  int GetGPUCount() const;

  /**
   * @brief Returns all the parsed GPU  information
   * 
   * @return std::vector<QmitkGPUSpec>
   */
  std::vector<QmitkGPUSpec> GetAllGPUSpecs();
};

#endif
