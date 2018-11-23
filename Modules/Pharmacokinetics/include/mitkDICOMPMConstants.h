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

#ifndef MITKDICOMPMCONSTANTS_H_
#define MITKDICOMPMCONSTANTS_H_

#include <mitkDICOMTagPath.h>

#include <MitkPharmacokineticsExports.h>


namespace mitk
{
  struct MITKPHARMACOKINETICS_EXPORT DICOMPMConstants
  {
    // IKO
	static DICOMTagPath RWVM_SEQUENCE_PATH();
	
	static DICOMTagPath UNITS_SEQUENCE_PATH();
	static DICOMTagPath UNITS_CODE_VALUE_PATH();
	static DICOMTagPath UNITS_CODE_SCHEME_PATH();
	static DICOMTagPath UNITS_CODE_MEANING_PATH();

	static DICOMTagPath QUANTITY_DEFINITION_SEQUENCE_PATH();
	static DICOMTagPath QUANTITY_DEFINITION_VALUE_TYPE_PATH();
	
	static DICOMTagPath QUANTITY_DEFINITION_CONCEPT_CODE_SEQUENCE_PATH();
	static DICOMTagPath QUANTITY_DEFINITION_CONCEPT_CODE_VALUE_PATH();
	static DICOMTagPath QUANTITY_DEFINITION_CONCEPT_CODE_SCHEME_PATH();
	static DICOMTagPath QUANTITY_DEFINITION_CONCEPT_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENT_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_NUMBER_PATH();
    static DICOMTagPath SEGMENT_LABEL_PATH();
    static DICOMTagPath SEGMENT_ALGORITHM_TYPE_PATH();

    static DICOMTagPath ANATOMIC_REGION_SEQUENCE_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_VALUE_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_SCHEME_PATH();
    static DICOMTagPath ANATOMIC_REGION_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_CATEGORY_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_CATEGORY_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_TYPE_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_TYPE_CODE_MEANING_PATH();

    static DICOMTagPath SEGMENTED_PROPERTY_MODIFIER_SEQUENCE_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_VALUE_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_SCHEME_PATH();
    static DICOMTagPath SEGMENT_MODIFIER_CODE_MEANING_PATH();


  };
}

#endif // MITKDICOMSEGMENTATIONCONSTANTS_H_
