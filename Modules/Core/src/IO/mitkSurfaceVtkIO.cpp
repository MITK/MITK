/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceVtkIO.h>
#include "mitkSurfaceIOHelper.h"

#include <mitkDataNode.h>
#include <mitkLexicalCast.h>
#include <mitkSurface.h>
#include <mitkUtf8Util.h>

#include <vtkPolyData.h>

#include <itksys/Directory.hxx>
#include <itksys/SystemTools.hxx>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iterator>
#include <limits>
#include <locale>
#include <optional>
#include <regex>
#include <sstream>

namespace
{
  struct SeriesFileName
  {
    std::string SeriesName;
    std::string Extension;
    mitk::TimeStepType TimeStep = 0;
    mitk::TimeBounds TimeBounds;
  };

  struct SeriesFile
  {
    std::string Path;
    mitk::TimeStepType TimeStep = 0;
    mitk::TimeBounds TimeBounds;
  };

  std::optional<SeriesFileName> ParseSeriesFileName(const std::string &fileName)
  {
    static const std::regex pattern(R"((.+)_S([^_]+)_E([^_]+)_T(\d+))");

    const auto extension = itksys::SystemTools::GetFilenameLastExtension(fileName);
    const auto stem = fileName.substr(0, fileName.size() - extension.size());

    std::smatch match;

    if (!std::regex_match(stem, match, pattern))
      return std::nullopt;

    SeriesFileName result;
    result.SeriesName = match[1];
    result.Extension = extension;

    try
    {
      result.TimeBounds[0] = mitk::LexicalCast<mitk::TimePointType>(match[2]);
      result.TimeBounds[1] = mitk::LexicalCast<mitk::TimePointType>(match[3]);
      result.TimeStep = mitk::LexicalCast<mitk::TimeStepType>(match[4]);
    }
    catch (const mitk::BadLexicalCast &)
    {
      return std::nullopt;
    }

    return result;
  }

  /** \brief Like the extensions of mime types, file names are compared case-insensitively. */
  bool EqualsCaseInsensitive(const std::string &lhs, const std::string &rhs)
  {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char l, char r) {
      return std::tolower(static_cast<unsigned char>(l)) == std::tolower(static_cast<unsigned char>(r));
    });
  }

  /**
   * \brief The files of the time series in the directory of the location,
   * sorted by time step.
   *
   * \return Nothing if the directory cannot be listed, with the reason in
   *         \p errorMessage.
   */
  std::optional<std::vector<SeriesFile>> ListSeriesFiles(const std::string &location,
                                                         const SeriesFileName &series,
                                                         std::string &errorMessage)
  {
    using mitk::Utf8Util::Local8BitToUtf8;
    using mitk::Utf8Util::Utf8ToLocal8Bit;

    const auto utf8Location = Local8BitToUtf8(location);
    const auto fileName = itksys::SystemTools::GetFilenameName(utf8Location);

    // The directory is kept as spelled in the location, so that the paths of
    // the series compare equal to the paths of other selected files.
    const auto directory = location.substr(0, location.size() - Utf8ToLocal8Bit(fileName).size());

    auto utf8Directory = itksys::SystemTools::GetFilenamePath(utf8Location);

    if (utf8Directory.empty())
      utf8Directory = ".";

    itksys::Directory listing;

    if (const auto status = listing.Load(utf8Directory); !status)
    {
      errorMessage = status.GetString();
      return std::nullopt;
    }

    std::vector<SeriesFile> seriesFiles;

    for (unsigned long i = 0; i < listing.GetNumberOfFiles(); ++i)
    {
      const std::string entry = listing.GetFile(i);
      const auto file = ParseSeriesFileName(entry);

      if (file.has_value() && EqualsCaseInsensitive(file->SeriesName, series.SeriesName) &&
          EqualsCaseInsensitive(file->Extension, series.Extension))
      {
        seriesFiles.push_back({ directory + Utf8ToLocal8Bit(entry), file->TimeStep, file->TimeBounds });
      }
    }

    std::sort(seriesFiles.begin(), seriesFiles.end(), [](const SeriesFile &lhs, const SeriesFile &rhs) {
      return lhs.TimeStep < rhs.TimeStep;
    });

    return seriesFiles;
  }

  /**
   * \brief All files of the time series that the file at the location belongs
   * to, sorted by time step, or none if the file is to be read on its own.
   */
  std::vector<SeriesFile> FindSeriesFiles(const std::string &location)
  {
    const auto selectedFile = ParseSeriesFileName(itksys::SystemTools::GetFilenameName(mitk::Utf8Util::Local8BitToUtf8(location)));

    if (!selectedFile.has_value())
      return {};

    std::string errorMessage;
    const auto seriesFiles = ListSeriesFiles(location, *selectedFile, errorMessage);

    if (!seriesFiles.has_value())
    {
      MITK_WARN << "The time steps of the time series of \"" << location << "\" cannot be collected, since its "
                << "directory cannot be listed: " << errorMessage << ". The file is read on its own.";
      return {};
    }

    // A time series has several files, so a single file whose name merely
    // follows the pattern is read on its own.
    if (seriesFiles->size() < 2)
      return {};

    const auto duplicate = std::adjacent_find(seriesFiles->begin(), seriesFiles->end(), [](const SeriesFile &lhs, const SeriesFile &rhs) {
      return lhs.TimeStep == rhs.TimeStep;
    });

    if (duplicate != seriesFiles->end())
    {
      mitkThrow() << "The files \"" << duplicate->Path << "\" and \"" << std::next(duplicate)->Path
                  << "\" of the time series \"" << selectedFile->SeriesName << "\" both belong to time step "
                  << duplicate->TimeStep << ".";
    }

    return *seriesFiles;
  }

  std::string ToShortestString(mitk::TimePointType value)
  {
    using Limits = std::numeric_limits<mitk::TimePointType>;

    std::string result;

    for (auto precision = Limits::digits10; precision <= Limits::max_digits10; ++precision)
    {
      std::ostringstream stream;
      stream.imbue(std::locale::classic());
      stream << std::setprecision(precision) << value;
      result = stream.str();

      if (mitk::LexicalCast<mitk::TimePointType>(result) == value)
        break;
    }

    return result;
  }

  std::string GetTimeStepFileName(const std::string &location, const mitk::TimeBounds &bounds, mitk::TimeStepType timeStep)
  {
    using mitk::Utf8Util::Local8BitToUtf8;
    using mitk::Utf8Util::Utf8ToLocal8Bit;

    const auto extension = Utf8ToLocal8Bit(itksys::SystemTools::GetFilenameLastExtension(Local8BitToUtf8(location)));
    const auto stem = location.substr(0, location.size() - extension.size());

    return stem + "_S" + ToShortestString(bounds[0]) + "_E" + ToShortestString(bounds[1]) + "_T" +
           std::to_string(timeStep) + extension;
  }

  /**
   * \brief Throw if the directory holds files of an earlier time series of the
   * same name that writing the given files would not overwrite, as they would
   * be read as part of the new time series.
   */
  void ThrowOnFilesOfEarlierSeries(const std::vector<std::string> &fileNames)
  {
    using mitk::Utf8Util::Local8BitToUtf8;

    const auto series = ParseSeriesFileName(itksys::SystemTools::GetFilenameName(Local8BitToUtf8(fileNames.front())));

    if (!series.has_value())
      return;

    std::string errorMessage;
    const auto existingFiles = ListSeriesFiles(fileNames.front(), *series, errorMessage);

    // Writing the files tells whether the directory can be accessed.
    if (!existingFiles.has_value())
      return;

    std::string earlierFiles;

    for (const auto &existingFile : *existingFiles)
    {
      const auto existingName = itksys::SystemTools::GetFilenameName(Local8BitToUtf8(existingFile.Path));

      const auto isOverwritten = std::any_of(fileNames.begin(), fileNames.end(), [&existingName](const std::string &fileName) {
        return EqualsCaseInsensitive(itksys::SystemTools::GetFilenameName(Local8BitToUtf8(fileName)), existingName);
      });

      if (!isOverwritten)
        earlierFiles += (earlierFiles.empty() ? "" : ", ") + existingName;
    }

    if (!earlierFiles.empty())
    {
      mitkThrow() << "The directory holds files of an earlier time series named \"" << series->SeriesName
                  << "\", which would be read as part of this one: " << earlierFiles
                  << ". Remove them or choose another file name.";
    }
  }
}

mitk::SurfaceVtkIO::SurfaceVtkIO(const CustomMimeType &mimeType, const std::string &description)
  : AbstractFileIO(Surface::GetStaticNameOfClass(), mimeType, description)
{
}

mitk::IFileIO::ConfidenceLevel mitk::SurfaceVtkIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
    return Unsupported;

  return this->GetInput()->GetTimeGeometry()->IsValidAtEveryTimePoint()
    ? Supported
    : PartiallySupported;
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::SurfaceVtkIO::DoRead()
{
  m_ReadFiles.clear();
  m_CurrentFile = 0;
  m_NumberOfFiles = 1;

  const auto seriesFiles = this->GetInputStream() == nullptr
    ? FindSeriesFiles(this->GetInputLocation())
    : std::vector<SeriesFile>();

  std::vector<vtkSmartPointer<vtkPolyData>> polyDatas;
  std::vector<TimeBounds> timeBounds;

  if (seriesFiles.empty())
  {
    polyDatas.push_back(this->ReadPolyData(this->GetLocalFileName()));
  }
  else
  {
    m_NumberOfFiles = static_cast<unsigned int>(seriesFiles.size());

    for (const auto &seriesFile : seriesFiles)
    {
      polyDatas.push_back(this->ReadPolyData(seriesFile.Path));
      timeBounds.push_back(seriesFile.TimeBounds);
      m_ReadFiles.push_back(seriesFile.Path);
      ++m_CurrentFile;
    }

    if (!SurfaceIOHelper::AreValidTimeBounds(timeBounds))
    {
      MITK_WARN << "The time bounds in the file names of the time series \"" << this->GetInputLocation()
                << "\" are empty or overlap, for example because an older version of MITK wrote them. "
                << "The time steps are numbered instead.";

      timeBounds.clear();
    }
  }

  std::vector<itk::SmartPointer<BaseData>> result;
  result.push_back(SurfaceIOHelper::CreateSurface(polyDatas, timeBounds).GetPointer());
  return result;
}

void mitk::SurfaceVtkIO::SetDefaultDataNodeProperties(DataNode *node, const std::string &filePath)
{
  const auto *nameProperty = node->GetProperty("name", nullptr, false);

  if (!m_ReadFiles.empty() && (nameProperty == nullptr || nameProperty->GetValueAsString() == DataNode::NO_NAME_VALUE()))
  {
    const auto seriesFile = ParseSeriesFileName(itksys::SystemTools::GetFilenameName(Utf8Util::Local8BitToUtf8(filePath)));

    if (seriesFile.has_value())
      node->SetName(seriesFile->SeriesName);
  }

  AbstractFileReader::SetDefaultDataNodeProperties(node, filePath);
}

void mitk::SurfaceVtkIO::Write()
{
  this->ValidateOutputLocation();

  const auto *surface = dynamic_cast<const Surface *>(this->GetInput());
  const auto *timeGeometry = surface->GetTimeGeometry();
  const auto timeSteps = timeGeometry->CountTimeSteps();

  m_CurrentFile = 0;
  m_NumberOfFiles = 1;

  if (this->GetOutputStream() != nullptr || timeSteps == 1)
  {
    if (timeSteps > 1)
    {
      MITK_WARN << "Writing multiple time steps to output streams is not supported. "
                << "Only the first time step will be written.";
    }

    LocalFile localFile(this);
    this->WritePolyData(SurfaceIOHelper::GetWorldPolyData(surface, 0), localFile.GetFileName());
    return;
  }

  std::vector<std::string> fileNames;

  for (TimeStepType timeStep = 0; timeStep < timeSteps; ++timeStep)
    fileNames.push_back(GetTimeStepFileName(this->GetOutputLocation(), timeGeometry->GetTimeBounds(timeStep), timeStep));

  ThrowOnFilesOfEarlierSeries(fileNames);

  m_NumberOfFiles = static_cast<unsigned int>(timeSteps);

  for (TimeStepType timeStep = 0; timeStep < timeSteps; ++timeStep)
  {
    m_CurrentFile = static_cast<unsigned int>(timeStep);
    this->WritePolyData(SurfaceIOHelper::GetWorldPolyData(surface, timeStep), fileNames[timeStep]);
  }
}

void mitk::SurfaceVtkIO::ReportReadProgress(float progress)
{
  this->AbstractFileReader::ReportProgress((m_CurrentFile + progress) / m_NumberOfFiles);
}

void mitk::SurfaceVtkIO::ReportWriteProgress(float progress)
{
  this->AbstractFileWriter::ReportProgress((m_CurrentFile + progress) / m_NumberOfFiles);
}
