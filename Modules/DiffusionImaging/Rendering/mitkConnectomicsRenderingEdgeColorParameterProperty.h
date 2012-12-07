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

#ifndef _MITK_CONNECTOMICS_RENDERING_EDGE_COLOR_PARAMETER_PROPERTY__H_
#define _MITK_CONNECTOMICS_RENDERING_EDGE_COLOR_PARAMETER_PROPERTY__H_

#include "mitkEnumerationProperty.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the enumeration of different rendering schemes for connectomics networks
 */
class MitkDiffusionImaging_EXPORT ConnectomicsRenderingEdgeColorParameterProperty : public EnumerationProperty
{
public:

  mitkClassMacro( ConnectomicsRenderingEdgeColorParameterProperty, EnumerationProperty );

  itkNewMacro(ConnectomicsRenderingEdgeColorParameterProperty);

  mitkNewMacro1Param(ConnectomicsRenderingEdgeColorParameterProperty, const IdType&);

  mitkNewMacro1Param(ConnectomicsRenderingEdgeColorParameterProperty, const std::string&);

  using BaseProperty::operator=;

protected:

  /**
   * Constructor. Sets the representation to a default value of 0
   */
  ConnectomicsRenderingEdgeColorParameterProperty( );

  /**
   * Constructor. Sets the parameter to the given value. If it is not
   * valid, the value is set to 0
   * @param value the integer representation of the parameter
   */
  ConnectomicsRenderingEdgeColorParameterProperty( const IdType& value );

  /**
   * Constructor. Sets the parameter to the given value. If it is not
   * valid, the value is set to 0
   * @param value the string representation of the parameter
   */
  ConnectomicsRenderingEdgeColorParameterProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional enumerations.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  virtual void AddRenderingEdgeColorParameters();

private:

  // purposely not implemented
  ConnectomicsRenderingEdgeColorParameterProperty(const ConnectomicsRenderingEdgeColorParameterProperty&);
  ConnectomicsRenderingEdgeColorParameterProperty& operator=(const ConnectomicsRenderingEdgeColorParameterProperty&);
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif
