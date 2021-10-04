/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/
#include "QmitknnUNetGPU.h"
#include <QProcess>

QmitkGPULoader::QmitkGPULoader()
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

int QmitkGPULoader::GetGPUCount()
{
  return static_cast<int>(m_Gpus.size());
}
