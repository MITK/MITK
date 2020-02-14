/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef _MITK_REG_VIS_DIRECTION_PROPERTY__H_
#define _MITK_REG_VIS_DIRECTION_PROPERTY__H_

// MITK
#include <mitkEnumerationProperty.h>

// MITK
#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the enumeration for direction for the registration visualization. Valid values are:
 * 0/Direct, 1/Inverse
 * Default is "Inverse"
 */
class MITKMATCHPOINTREGISTRATION_EXPORT RegVisDirectionProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RegVisDirectionProperty, EnumerationProperty );

  itkNewMacro(RegVisDirectionProperty);

  mitkNewMacro1Param(RegVisDirectionProperty, const IdType&);

  mitkNewMacro1Param(RegVisDirectionProperty, const std::string&);


  using BaseProperty::operator=;

protected:

  /**
   * Constructor. Sets the representation to a default value of 1
   */
  RegVisDirectionProperty( );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 1
   */
  RegVisDirectionProperty( const IdType& value );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 1
   */
  RegVisDirectionProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid interpolation types.
   */
  bool AddEnum( const std::string& name, const IdType& id ) override;

  /**
   * Adds the enumeration types
   */
  void AddTypes();

private:

  // purposely not implemented
  RegVisDirectionProperty(const RegVisDirectionProperty&);
  RegVisDirectionProperty& operator=(const RegVisDirectionProperty&);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif
