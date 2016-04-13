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

namespace mitk {

  /**
  Base class for service wrappers that make DICOMFileReader from
  the DICOMReader module usable.
  */
class BaseDICOMReaderService : public AbstractFileReader
{
public:
  BaseDICOMReaderService(const std::string& description);

  using AbstractFileReader::Read;

  /** Uses this->GetRelevantFile() and this->GetReader to load the image.
   * data and puts it into base data instances-*/
  virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

protected:
  /** Returns the list of all DCM files that are in the same directory
   * like this->GetLocalFileName().*/
  mitk::StringList GetRelevantFiles() const;

  /** Returns the reader instance that should be used. The descission may be based
   * one the passed relevant file list.*/
  virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const = 0;
};

}

#endif // MITKBASEDICOMREADERSERVICE_H
