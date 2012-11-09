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

#ifndef _MITK_OdfNormalizationMethodProperty__H_
#define _MITK_OdfNormalizationMethodProperty__H_

#include "mitkEnumerationProperty.h"
#include "mitkEnumerationPropertySerializer.h"
#include "MitkDiffusionImagingExports.h"

#include "mitkSerializerMacros.h"

namespace mitk
{

  enum OdfNormalizationMethod
  {
    ODFN_MINMAX,
    ODFN_MAX,
    ODFN_NONE,
    ODFN_GLOBAL_MAX
  };

/**
 * Encapsulates the enumeration for ODF normalization. Valid values are
 * ODFN_MINMAX, ODFN_MAX, ODFN_NONE ODFN_GLOBAL_MAX
 * Default is ODFN_MINMAX
 */
class MitkDiffusionImaging_EXPORT OdfNormalizationMethodProperty : public EnumerationProperty
{
public:

  mitkClassMacro( OdfNormalizationMethodProperty, EnumerationProperty );
  itkNewMacro(OdfNormalizationMethodProperty);

  mitkNewMacro1Param(OdfNormalizationMethodProperty, const IdType&);

  mitkNewMacro1Param(OdfNormalizationMethodProperty, const std::string&);

  /**
   * Returns the current interpolation value as defined by VTK constants.
   */
  virtual int GetNormalization();

  /**
   * Sets the interpolation type to ODFN_MINMAX.
   */
  virtual void SetNormalizationToMinMax();

  /**
   * Sets the interpolation type to ODFN_MAX.
   */
  virtual void SetNormalizationToMax();

  /**
   * Sets the interpolation type to ODFN_NONE.
   */
  virtual void SetNormalizationToNone();

  /**
   * Sets the interpolation type to ODFN_GLOBAL_MAX.
   */
  virtual void SetNormalizationToGlobalMax();

protected:

  /** Sets reslice interpolation mode to default (VTK_RESLICE_NEAREST).
   */
  OdfNormalizationMethodProperty( );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfNormalizationMethodProperty( const IdType& value );

  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfNormalizationMethodProperty( const std::string& value );

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

class MitkDiffusionImaging_EXPORT OdfNormalizationMethodPropertySerializer : public EnumerationPropertySerializer
{
  public:

    mitkClassMacro( OdfNormalizationMethodPropertySerializer, EnumerationPropertySerializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      const char* sa( element->Attribute("value") );
      std::string s(sa?sa:"");
      OdfNormalizationMethodProperty::Pointer property = OdfNormalizationMethodProperty::New();
      property->SetValue( s );
      return property.GetPointer();
    }

  protected:

    OdfNormalizationMethodPropertySerializer () {}
    virtual ~OdfNormalizationMethodPropertySerializer () {}
};


} // end of namespace mitk

#endif


