/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRegEvalStyleProperty_h
#define mitkRegEvalStyleProperty_h

// MITK
#include <mitkEnumerationProperty.h>

// MITK
#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{

/**
 * \brief Enumeration property defining the visualization style for registration evaluation.
 *
 * Valid values are:
 * - 0: Blend
 * - 1: Color Blend
 * - 2: Checkerboard
 * - 3: Wipe
 * - 4: Difference
 * - 5: Contour
 *
 * Default is "Blend".
 *
 * \sa mitk::RegEvaluationMapper2D, mitk::RegEvalWipeStyleProperty
 */
class MITKMATCHPOINTREGISTRATION_EXPORT RegEvalStyleProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RegEvalStyleProperty, EnumerationProperty );

  itkNewMacro(RegEvalStyleProperty);

  /** \brief Constructs a RegEvalStyleProperty from a numeric ID. */
  mitkNewMacro1Param(RegEvalStyleProperty, const IdType&);

  /** \brief Constructs a RegEvalStyleProperty from a string name. */
  mitkNewMacro1Param(RegEvalStyleProperty, const std::string&);


  using BaseProperty::operator=;

protected:

  /**
   * Constructor. Sets the representation to a default value of 0
   */
  RegEvalStyleProperty( );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegEvalStyleProperty( const IdType& value );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegEvalStyleProperty( const std::string& value );

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
  RegEvalStyleProperty(const RegEvalStyleProperty&);
  RegEvalStyleProperty& operator=(const RegEvalStyleProperty&);
};


} // end of namespace mitk

#endif
