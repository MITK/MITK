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

#ifndef mitkDICOMITKSeriesGDCMReaderConfigurator_h
#define mitkDICOMITKSeriesGDCMReaderConfigurator_h

#include "mitkDICOMITKSeriesGDCMReader.h"
#include "mitkDICOMTagBasedSorter.h"

class DicomFileReader;
class TiXmlElement;

namespace mitk {
class DICOMReader_EXPORT DICOMITKSeriesGDCMReaderConfigurator
{
public:

  DICOMITKSeriesGDCMReaderConfigurator();

  mitk::DICOMTagBasedSorter::Pointer LoadDicomReaderConfigFile(const std::string& filename);

  mitk::DICOMITKSeriesGDCMReader::Pointer CreateDicomReaderInstance();
  mitk::DICOMITKSeriesGDCMReader::Pointer CreateDicomReaderInstance(const std::string& filename);


private:

  DICOMTag getTagFromXMLElement(TiXmlElement*);
};
}

#endif // mitkDICOMITKSeriesGDCMReaderConfigurator_h
