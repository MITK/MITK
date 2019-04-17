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


#ifndef _MITK_REG_EVAL_WIPE_STYLE_PROPERTY__H_
#define _MITK_REG_EVAL_WIPE_STYLE_PROPERTY__H_

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
 * Encapsulates the enumeration for visualization styles. Valid values are:
 * 0/Cross 1/horizontal wipe 2/vertical wipe
 * Default is "Cross"
 */
class MITKMATCHPOINTREGISTRATION_EXPORT RegEvalWipeStyleProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RegEvalWipeStyleProperty, EnumerationProperty );

  itkNewMacro(RegEvalWipeStyleProperty);

  mitkNewMacro1Param(RegEvalWipeStyleProperty, const IdType&);

  mitkNewMacro1Param(RegEvalWipeStyleProperty, const std::string&);


  using BaseProperty::operator=;

protected:

  /**
   * Constructor. Sets the representation to a default value of 0
   */
  RegEvalWipeStyleProperty( );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegEvalWipeStyleProperty( const IdType& value );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegEvalWipeStyleProperty( const std::string& value );

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
  RegEvalWipeStyleProperty(const RegEvalWipeStyleProperty&);
  RegEvalWipeStyleProperty& operator=(const RegEvalWipeStyleProperty&);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif


