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

#include "mitkClassicDICOMSeriesReader.h"

#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"
#include "mitkSortByImagePositionPatient.h"


mitk::ClassicDICOMSeriesReader
::ClassicDICOMSeriesReader()
:DICOMITKSeriesGDCMReader()
{
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // all the things that split by tag in mitk::DicomSeriesReader
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0010) ); // Number of Rows
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0011) ); // Number of Columns
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0037), new mitk::DICOMTagBasedSorter::CutDecimalPlaces(5) ); // Image Orientation (Patient)
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x000e) ); // Series Instance UID
  tagSorter->AddDistinguishingTag( DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  tagSorter->AddDistinguishingTag( DICOMTag(0x0028, 0x0008) ); // Number of Frames
  //tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

  // a sorter...
  // TODO ugly syntax, improve..
  mitk::DICOMSortCriterion::ConstPointer sorting =
    mitk::SortByImagePositionPatient::New( // image position patient and image orientation
      mitk::DICOMSortByTag::New( DICOMTag(0x0020, 0x0012), // aqcuisition number
        mitk::DICOMSortByTag::New( DICOMTag(0x0008, 0x0032), // aqcuisition time
          mitk::DICOMSortByTag::New( DICOMTag(0x0018, 0x1060), // trigger time
            mitk::DICOMSortByTag::New( DICOMTag(0x0008, 0x0018) // SOP instance UID (last resort, not really meaningful but decides clearly)
            ).GetPointer()
          ).GetPointer()
        ).GetPointer()
      ).GetPointer()
    ).GetPointer();
  tagSorter->SetSortCriterion( sorting );

  // define above sorting for this class
  this->AddSortingElement( tagSorter );

  this->SetFixTiltByShearing(true);
}

mitk::ClassicDICOMSeriesReader
::ClassicDICOMSeriesReader(const ClassicDICOMSeriesReader& other )
:DICOMITKSeriesGDCMReader(other)
{
}

mitk::ClassicDICOMSeriesReader
::~ClassicDICOMSeriesReader()
{
}

mitk::ClassicDICOMSeriesReader&
mitk::ClassicDICOMSeriesReader
::operator=(const ClassicDICOMSeriesReader& other)
{
  if (this != &other)
  {
    DICOMITKSeriesGDCMReader::operator=(other);
  }
  return *this;
}
