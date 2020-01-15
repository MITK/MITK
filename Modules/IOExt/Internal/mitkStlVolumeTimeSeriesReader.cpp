/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStlVolumeTimeSeriesReader.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include <mitkIOUtil.h>

void mitk::StlVolumeTimeSeriesReader::GenerateData()
{
  if (!this->GenerateFileList())
  {
    itkWarningMacro(<< "Sorry, file list could not be determined...");
    return;
  }

  mitk::Surface::Pointer result = this->GetOutput();
  MITK_INFO << "prefix: " << m_FilePrefix << ", pattern: " << m_FilePattern << std::endl;
  result->Expand(m_MatchedFileNames.size());
  for (unsigned int i = 0; i < m_MatchedFileNames.size(); ++i)
  {
    std::string fileName = m_MatchedFileNames[i];
    MITK_INFO << "Loading " << fileName << " as stl..." << std::endl;

    auto timestepSurface = IOUtil::Load<Surface>(fileName.c_str());

    if (timestepSurface.IsNull())
    {
      itkWarningMacro(<< "stlReader returned nullptr while reading " << fileName
                      << ". Trying to continue with empty vtkPolyData...");
      result->SetVtkPolyData(vtkPolyData::New(), i);
      return;
    }

    result->SetVtkPolyData(timestepSurface->GetVtkPolyData(), i);
  }
}

bool mitk::StlVolumeTimeSeriesReader::CanReadFile(const std::string /*filename*/,
                                                  const std::string filePrefix,
                                                  const std::string filePattern)
{
  if (filePattern != "" && filePrefix != "")
    return false;

  bool extensionFound = false;
  std::string::size_type STLPos = filePattern.rfind(".stl");
  if ((STLPos != std::string::npos) && (STLPos == filePattern.length() - 4))
    extensionFound = true;

  STLPos = filePattern.rfind(".STL");
  if ((STLPos != std::string::npos) && (STLPos == filePattern.length() - 4))
    extensionFound = true;

  if (!extensionFound)
    return false;

  return true;
}

mitk::StlVolumeTimeSeriesReader::StlVolumeTimeSeriesReader()
{
}

mitk::StlVolumeTimeSeriesReader::~StlVolumeTimeSeriesReader()
{
}
