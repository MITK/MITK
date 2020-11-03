/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKBASEDICOMREADERSERVICE_H
#define MITKBASEDICOMREADERSERVICE_H

#include <mitkAbstractFileReader.h>
#include <mitkDICOMFileReader.h>

#include "MitkDICOMExports.h"

namespace mitk {

  /**
  Base class for service wrappers that make DICOMFileReader from
  the DICOM module usable.
  */
class MITKDICOM_EXPORT BaseDICOMReaderService : public AbstractFileReader
{
public:
  using AbstractFileReader::Read;

  IFileReader::ConfidenceLevel GetConfidenceLevel() const override;

protected:
  BaseDICOMReaderService(const std::string& description);
  BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description);

  /** Uses this->GetRelevantFile() and this->GetReader to load the image.
   * data and puts it into base data instances-*/
  std::vector<itk::SmartPointer<BaseData>> DoRead() override;

 /** Returns the list of all DCM files that are in the same directory
   * like this->GetLocalFileName().*/
  mitk::StringList GetDICOMFilesInSameDirectory() const;

  /** Returns the reader instance that should be used. The decision may be based
   * one the passed list of relevant files.*/
  virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const = 0;

  void SetOnlyRegardOwnSeries(bool);
  bool GetOnlyRegardOwnSeries() const;

private:
  /** Flags that constrols if the read() operation should only regard DICOM files of the same series
  if the specified GetLocalFileName() is a file. If it is a director, this flag has no impact (it is
  assumed false then).
  */
  bool m_OnlyRegardOwnSeries = true;
};


class IPropertyProvider;

/** Helper function that generates a name string (e.g. for DataNode names) from the DICOM properties of the passed
  provider instance. If the instance is nullptr, or has no dicom properties DataNode::NO_NAME_VALUE() will be returned.*/
std::string MITKDICOM_EXPORT GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider);

}

#endif // MITKBASEDICOMREADERSERVICE_H
