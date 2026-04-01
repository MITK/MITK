/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRegVisStyleProperty_h
#define mitkRegVisStyleProperty_h

// MITK
#include <mitkEnumerationProperty.h>

// MITK
#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * \brief Enumeration property defining the visual representation style for registrations.
 *
 * Valid values are:
 * - 0: Grid (deformation grid)
 * - 1: Glyph (arrow glyphs showing deformation vectors)
 * - 2: Points (point cloud)
 *
 * Default is "Grid".
 *
 * \sa mitk::MITKRegistrationWrapperMapperBase, mitk::RegVisColorStyleProperty, mitk::RegVisDirectionProperty
 */
class MITKMATCHPOINTREGISTRATION_EXPORT RegVisStyleProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RegVisStyleProperty, EnumerationProperty );

  itkNewMacro(RegVisStyleProperty);

  /** \brief Constructs a RegVisStyleProperty from a numeric ID. */
  mitkNewMacro1Param(RegVisStyleProperty, const IdType&);

  /** \brief Constructs a RegVisStyleProperty from a string name. */
  mitkNewMacro1Param(RegVisStyleProperty, const std::string&);


  using BaseProperty::operator=;

protected:

  /**
   * Constructor. Sets the representation to a default value of 0
   */
  RegVisStyleProperty( );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegVisStyleProperty( const IdType& value );

  /**
   * Constructor. Sets the enumeration to the given value. If it is not
   * valid, the enumeration is set to 0
   */
  RegVisStyleProperty( const std::string& value );

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
  RegVisStyleProperty(const RegVisStyleProperty&);
  RegVisStyleProperty& operator=(const RegVisStyleProperty&);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif
