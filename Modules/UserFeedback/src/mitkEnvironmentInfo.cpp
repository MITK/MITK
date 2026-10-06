/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkEnvironmentInfo.h>

#include <itkConfigure.h>
#include <mitkVersion.h>
#include <vtkVersionMacros.h>

#include <QLocale>
#include <QSysInfo>
#include <QtGlobal>

mitk::EnvironmentInfo mitk::CollectEnvironmentInfo()
{
  EnvironmentInfo info;

  info.mitkVersion = MITK_VERSION_STRING;
  info.mitkRevision = MITK_REVISION;
  info.mitkRevisionDescription = MITK_REVISION_DESC;
#ifdef NDEBUG
  info.buildType = "Release";
#else
  info.buildType = "Debug";
#endif
  info.qtVersion = QT_VERSION_STR;
  info.vtkVersion = VTK_VERSION;
  info.itkVersion = std::to_string(ITK_VERSION_MAJOR) + '.' + std::to_string(ITK_VERSION_MINOR) + '.' +
                    std::to_string(ITK_VERSION_PATCH);
  info.os = QSysInfo::prettyProductName().toStdString();
  info.kernelType = QSysInfo::kernelType().toStdString();
  info.kernelVersion = QSysInfo::kernelVersion().toStdString();
  info.cpuArchitecture = QSysInfo::currentCpuArchitecture().toStdString();
  info.uiLocale = QLocale::system().name().toStdString();

  return info;
}

std::vector<std::pair<std::string, std::string>> mitk::ToLabeledFields(const EnvironmentInfo& info)
{
  const std::pair<const char*, const std::string*> fields[] = {
    { "MITK version", &info.mitkVersion },
    { "Revision", &info.mitkRevision },
    { "Revision description", &info.mitkRevisionDescription },
    { "Build type", &info.buildType },
    { "Qt version", &info.qtVersion },
    { "VTK version", &info.vtkVersion },
    { "ITK version", &info.itkVersion },
    { "Operating system", &info.os },
    { "Kernel type", &info.kernelType },
    { "Kernel version", &info.kernelVersion },
    { "CPU architecture", &info.cpuArchitecture },
    { "UI locale", &info.uiLocale },
    { "GPU renderer", &info.gpuRenderer }
  };

  std::vector<std::pair<std::string, std::string>> labeled;

  for (const auto& [label, value] : fields)
  {
    if (!value->empty())
      labeled.emplace_back(label, *value);
  }

  return labeled;
}
