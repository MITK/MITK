/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/
#include "QmitknnUNetGPU.h"

#include <QProcess>
#include <iostream>
#include <string>
#include <vector>

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
    //spec.id = id;
    spec.memoryFree = gpuDetails.at(1).split(" ")[0].toInt();
    this->gpus.push_back(spec);
  }
}

int QmitkGPULoader::GetGPUCount()
{
  return static_cast<int>(gpus.size());
}

int QmitkGPULoader::GetTotalGPUs()
{
#if defined(__APPLE__) || defined(MACOSX) || defined(linux) || defined(__linux__)
  QProcess process1, process2;
  process1.setStandardOutputProcess(&process2);
  process1.start("nvidia-smi -L");
  process2.start("wc -l");
  process1.waitForFinished(-1);
  process2.waitForFinished(-1);
  QString nGpus = process2.readAll();
  return nGpus.toInt();
#elif defined(_WIN32)
  QProcess process;
  QStringList nGpus;
  process.setReadChannel(QProcess::StandardOutput);
  process.start("cmd",
                QStringList() << "/c"
                              << "nvidia-smi -L");
  process.waitForFinished(-1);
  while (process.canReadLine())
  {
    nGpus << QString(process.readLine());
  }
  return nGpus.size();
#endif
}
