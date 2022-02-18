/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetToolGPU_h_Included
#define QmitknnUNetToolGPU_h_Included

#include <QObject>
#include <QProcess>
#include <QString>
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
  std::vector<QmitkGPUSpec> m_Gpus;

public:
  /**
   * @brief Construct a new Qmitk GPU Loader object.
   * Parses GPU info using `nvidia-smi` command and saves it as QmitkGPUSpec objects.
   */
  QmitkGPULoader()
  {
    QProcess process;
    process.start("nvidia-smi --query-gpu=name,memory.free --format=csv");
    process.waitForFinished(-1);
    QStringList infoStringList;
    while (process.canReadLine())
    {
      QString line = process.readLine();
      if (!line.startsWith("name"))
      {
        infoStringList << line;
      }
    }
    foreach (QString infoString, infoStringList)
    {
      QmitkGPUSpec spec;
      QStringList gpuDetails;
      gpuDetails = infoString.split(",");
      spec.name = gpuDetails.at(0);
      // spec.id = id;
      spec.memoryFree = gpuDetails.at(1).split(" ")[0].toInt();
      this->m_Gpus.push_back(spec);
    }
  }
  ~QmitkGPULoader() = default;

  /**
   * @brief Returns the number of GPUs parsed and saved as QmitkGPUSpec objects.
   *
   * @return int
   */
  int GetGPUCount() { return static_cast<int>(m_Gpus.size()); }
};

#endif
