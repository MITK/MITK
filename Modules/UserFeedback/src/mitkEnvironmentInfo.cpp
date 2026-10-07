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

std::vector<std::pair<mitk::EnvironmentField, std::string>> mitk::ToFields(const EnvironmentInfo& info)
{
  const std::pair<EnvironmentField, const std::string*> fields[] = {
    { EnvironmentField::MitkVersion, &info.mitkVersion },
    { EnvironmentField::Revision, &info.mitkRevision },
    { EnvironmentField::RevisionDescription, &info.mitkRevisionDescription },
    { EnvironmentField::BuildType, &info.buildType },
    { EnvironmentField::QtVersion, &info.qtVersion },
    { EnvironmentField::VtkVersion, &info.vtkVersion },
    { EnvironmentField::ItkVersion, &info.itkVersion },
    { EnvironmentField::OperatingSystem, &info.os },
    { EnvironmentField::KernelType, &info.kernelType },
    { EnvironmentField::KernelVersion, &info.kernelVersion },
    { EnvironmentField::CpuArchitecture, &info.cpuArchitecture },
    { EnvironmentField::UiLocale, &info.uiLocale },
    { EnvironmentField::GpuRenderer, &info.gpuRenderer }
  };

  std::vector<std::pair<EnvironmentField, std::string>> result;

  for (const auto& [field, value] : fields)
  {
    if (!value->empty())
      result.emplace_back(field, *value);
  }

  return result;
}
