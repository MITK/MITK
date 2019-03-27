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

#ifndef _MITK_RT_CONSTANTS_H_
#define _MITK_RT_CONSTANTS_H_

#include <string>

#include "MitkDicomRTExports.h"

namespace mitk
{

struct MITKDICOMRT_EXPORT RTConstants

{
  /**
      * Name of the property that indicates if a data/node is a dose.
      */
  static const std::string DOSE_PROPERTY_NAME;

  /**
      * Name of the property that encodes the prescribed dose associated with the data node
      * If a RTPLAN file exists the value can be extracted from the tag (300A,0026) - Target Prescription Dose in the plan file.
      */
  static const std::string PRESCRIBED_DOSE_PROPERTY_NAME;

  /**
      * Name of the property that encodes the reference dose that should be used for relative dose vizualization/evaluation purpose.
      * It is often the prescribed dose but may differ e.g. when to dose distributions sould be compared using the same reference.
      */
  static const std::string REFERENCE_DOSE_PROPERTY_NAME;

  /**
  * Name of the property that encodes the reference structure set.
  */
  static const std::string REFERENCE_STRUCTURE_SET_PROPERTY_NAME;

  /**
      * Name of the property that encodes the optional string property holding the information from the tag (3004,0004) - Dose Type.
      * This contains useful information for medical doctors
      */
  static const std::string DOSE_TYPE_PROPERTY_NAME;

  /**
  * Name of the property that encodes the optional string property holding the description information from the tag (300A,0016) - Dose Reference Description.
  */
  static const std::string REFERENCE_DESCRIPTION_DOSE_PROPERTY_NAME;

  /**
      * Name of the property that encodes the optional string property holding the information from the tag (3004,000A) - Dose Summation Type.
      * This contains useful information for medical doctors
      */
  static const std::string DOSE_SUMMATION_TYPE_PROPERTY_NAME;

  /**
      * Name of the property that encodes the number of fractions.
      * It is for example in DICOM stored in tag (300A,0078) - Number of Fractions Prescribed (from the RTPLAN file if this file exists).
      * This value could be used to further scale the dose according to dose summation type.
      * For example a given plan consists of 8 fractions. Scaling the fraction dose by 8 gives the complete planned dose.
      */
  static const std::string DOSE_FRACTION_COUNT_PROPERTY_NAME;

  /**
  * Name of the property that encodes the number of beams.
  * It is for example in DICOM stored in tag (300A,0080) - Number of Beams (from the RTPLAN file if this file exists).
  */
  static const std::string DOSE_FRACTION_NUMBER_OF_BEAMS_PROPERTY_NAME;
  /**
  * Name of the property that encodes the radiation type of beams.
  * It is for example in DICOM stored in tag (300A,00C6) - Radiation Type (from the RTPLAN file if this file exists).
  */
  static const std::string DOSE_RADIATION_TYPE_PROPERTY_NAME;

  /**
      * Name of the property that encodes if the iso line rendering should be activated for the node.
      */
  static const std::string DOSE_SHOW_ISOLINES_PROPERTY_NAME;

  /**
      * Name of the property that encodes if the color wash rendering should be activated for the node.
      */
  static const std::string DOSE_SHOW_COLORWASH_PROPERTY_NAME;

  /**
      * Name of the property that encodes if the set of iso levels should be used to visualize the dose distribution.
      */
  static const std::string DOSE_ISO_LEVELS_PROPERTY_NAME;

  /**
      * Name of the property that encodes user defined iso values that mark special dose values in the distribution.
      */
  static const std::string DOSE_FREE_ISO_VALUES_PROPERTY_NAME;

};
}

#endif
