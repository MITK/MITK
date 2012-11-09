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

#ifndef _MITK_OdfScaleByProperty__H_
#define _MITK_OdfScaleByProperty__H_

#include "MitkDiffusionImagingExports.h"
#include "mitkEnumerationProperty.h"
#include "mitkEnumerationPropertySerializer.h"

namespace mitk
{

  enum OdfScaleBy
  {
    ODFSB_NONE,
    ODFSB_GFA,
    ODFSB_PC
  };

/**
 * Encapsulates the enumeration for ODF normalization. Valid values are
 *     ODFSB_NONE, ODFSB_GFA, ODFSB_PC
 * Default is ODFSB_NONE
 */
class MitkDiffusionImaging_EXPORT OdfScaleByProperty : public EnumerationProperty
{
public:

  mitkClassMacro( OdfScaleByProperty, EnumerationProperty );
  itkNewMacro(OdfScaleByProperty);

  mitkNewMacro1Param(OdfScaleByProperty, const IdType&);

  mitkNewMacro1Param(OdfScaleByProperty, const std::string&);

  /**
   * Returns the current interpolation value as defined by VTK constants.
   */
  virtual int GetScaleBy();

  virtual void SetScaleByNothing();
  virtual void SetScaleByGFA();
  virtual void SetScaleByPrincipalCurvature();

protected:

  /** Sets reslice interpolation mode to default (ODFSB_NONE).
   */
  OdfScaleByProperty( );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfScaleByProperty( const IdType& value );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfScaleByProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid interpolation types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known
   * enumeration values.
   */
  virtual void AddInterpolationTypes();
};

class MitkDiffusionImaging_EXPORT OdfScaleByPropertySerializer : public EnumerationPropertySerializer
{
  public:

    mitkClassMacro( OdfScaleByPropertySerializer, EnumerationPropertySerializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      const char* sa( element->Attribute("value") );
      std::string s(sa?sa:"");
      OdfScaleByProperty::Pointer property = OdfScaleByProperty::New();
      property->SetValue( s );
      return property.GetPointer();
    }

  protected:

    OdfScaleByPropertySerializer () {}
    virtual ~OdfScaleByPropertySerializer () {}
};

} // end of namespace mitk

#endif


