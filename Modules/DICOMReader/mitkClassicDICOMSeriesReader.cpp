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
:ThreeDnTDICOMSeriesReader()
{
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // all the things that split by tag in mitk::DicomSeriesReader
  tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x000e) ); // Series Instance UID
  //tagSorter->AddDistinguishingTag( DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

  // a sorter...
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
  this->SetToleratedOriginOffset(0.005);
  this->SetGroup3DandT(true);
}

mitk::ClassicDICOMSeriesReader
::ClassicDICOMSeriesReader(const ClassicDICOMSeriesReader& other )
:itk::Object()
,ThreeDnTDICOMSeriesReader(other)
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
    ThreeDnTDICOMSeriesReader::operator=(other);
  }
  return *this;
}
