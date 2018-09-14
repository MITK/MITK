/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKBASEDICOMREADERSERVICE_H
#define MITKBASEDICOMREADERSERVICE_H

#include <mitkAbstractFileReader.h>
#include <mitkDICOMFileReader.h>

#include "MitkDICOMReaderExports.h"

namespace mitk {

  /**
  Base class for service wrappers that make DICOMFileReader from
  the DICOMReader module usable.
  */
class MITKDICOMREADER_EXPORT BaseDICOMReaderService : public AbstractFileReader
{
public:
  BaseDICOMReaderService(const std::string& description);
  BaseDICOMReaderService(const mitk::CustomMimeType& customType, const std::string& description);

  using AbstractFileReader::Read;

  /** Uses this->GetRelevantFile() and this->GetReader to load the image.
   * data and puts it into base data instances-*/
  std::vector<itk::SmartPointer<BaseData> > Read() override;

  IFileReader::ConfidenceLevel GetConfidenceLevel() const override;

protected:
  /** Returns the list of all DCM files that are in the same directory
   * like this->GetLocalFileName().*/
  mitk::StringList GetRelevantFiles() const;

  /** Returns the reader instance that should be used. The descission may be based
   * one the passed relevant file list.*/
  virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const = 0;
};


class IPropertyProvider;

/** Helper function that generates a name string (e.g. for DataNode names) from the DICOM properties of the passed
  provider instance. If the instance is nullptr, or has no dicom properties DataNode::NO_NAME_VALUE() will be returned.*/
std::string MITKDICOMREADER_EXPORT GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider);

}

#endif // MITKBASEDICOMREADERSERVICE_H
