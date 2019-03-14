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

#include "mitkDICOMPMConstants.h"

namespace mitk
{
  DICOMTagPath DICOMPMConstants::RWVM_SEQUENCE_PATH()
  {
    return DICOMTagPath().AddElement(0x0040, 0x9096);
   }
  DICOMTagPath DICOMPMConstants::UNITS_SEQUENCE_PATH()
  {
    return DICOMPMConstants::RWVM_SEQUENCE_PATH().AddElement(0x0040, 0x08EA);
  }
  DICOMTagPath DICOMPMConstants::UNITS_CODE_VALUE_PATH()
  {
    return DICOMPMConstants::UNITS_SEQUENCE_PATH().AddElement(0x0008, 0x0100);
  }
  DICOMTagPath DICOMPMConstants::UNITS_CODE_SCHEME_PATH()
  {
    return DICOMPMConstants::UNITS_SEQUENCE_PATH().AddElement(0x0008, 0x0102);
  }
  DICOMTagPath DICOMPMConstants::UNITS_CODE_MEANING_PATH()
  {
    return DICOMPMConstants::UNITS_SEQUENCE_PATH().AddElement(0x0008, 0x0104);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_SEQUENCE_PATH()
  {
    return DICOMPMConstants::RWVM_SEQUENCE_PATH().AddElement(0x0040, 0x9220);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_VALUE_TYPE_PATH()
  {
		return DICOMPMConstants::QUANTITY_DEFINITION_SEQUENCE_PATH().AddElement(0x0040, 0xA040);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH()
  {
    return DICOMPMConstants::QUANTITY_DEFINITION_SEQUENCE_PATH().AddElement(0x0040, 0xA168);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_VALUE_PATH()
  {
    return DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH().AddElement(0x0008, 0x0100);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SCHEME_PATH()
  {
    return DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH().AddElement(0x0008, 0x0102);
  }
  DICOMTagPath DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_MEANING_PATH()
  {
    return DICOMPMConstants::QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH().AddElement(0x0008, 0x0104);
  }
}
