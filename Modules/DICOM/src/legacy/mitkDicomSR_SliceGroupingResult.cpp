/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <legacy/mitkDicomSeriesReader.h>

namespace mitk
{
  DicomSeriesReader::SliceGroupingAnalysisResult::SliceGroupingAnalysisResult() : m_GantryTilt(false) {}
  DicomSeriesReader::StringContainer DicomSeriesReader::SliceGroupingAnalysisResult::GetBlockFilenames()
  {
    return m_GroupedFiles;
  }

  DicomSeriesReader::StringContainer DicomSeriesReader::SliceGroupingAnalysisResult::GetUnsortedFilenames()
  {
    return m_UnsortedFiles;
  }

  bool DicomSeriesReader::SliceGroupingAnalysisResult::ContainsGantryTilt() { return m_GantryTilt; }
  void DicomSeriesReader::SliceGroupingAnalysisResult::AddFileToSortedBlock(const std::string &filename)
  {
    m_GroupedFiles.push_back(filename);
  }

  void DicomSeriesReader::SliceGroupingAnalysisResult::AddFileToUnsortedBlock(const std::string &filename)
  {
    m_UnsortedFiles.push_back(filename);
  }

  void DicomSeriesReader::SliceGroupingAnalysisResult::AddFilesToUnsortedBlock(const StringContainer &filenames)
  {
    m_UnsortedFiles.insert(m_UnsortedFiles.end(), filenames.begin(), filenames.end());
  }

  void DicomSeriesReader::SliceGroupingAnalysisResult::FlagGantryTilt() { m_GantryTilt = true; }
  void DicomSeriesReader::SliceGroupingAnalysisResult::UndoPrematureGrouping()
  {
    assert(!m_GroupedFiles.empty());
    m_UnsortedFiles.insert(m_UnsortedFiles.begin(), m_GroupedFiles.back());
    m_GroupedFiles.pop_back();
  }

} // end namespace mitk
