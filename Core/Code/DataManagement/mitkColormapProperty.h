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

#ifndef mitkColormapProperty_h_Included
#define mitkColormapProperty_h_Included

#include <MitkExports.h>
#include "mitkEnumerationProperty.h"
#include <itkObjectFactory.h>

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
  \brief Enumerates all known modalities

  \ingroup DataManagement
*/
class MITK_CORE_EXPORT ColormapProperty : public EnumerationProperty
{
  public:

    mitkClassMacro(ColormapProperty, EnumerationProperty);
    itkNewMacro(ColormapProperty);
    mitkNewMacro1Param(ColormapProperty, const IdType&);
    mitkNewMacro1Param(ColormapProperty, const std::string&);

    enum ColormapType
      {
        CM_BW,
        CM_BWINVERSE,
        CM_REDBLACKALPHA,
        CM_GREENBLACKALPHA,
        CM_BLUEBLACKALPHA,
        CM_VRMUSCLESBONES,
        CM_VRREDVESSELS,
        CM_STERN,
        CM_HOTGREEN,
        CM_VRBONES,
        CM_CARDIAC,
        CM_GRAYRAINBOW,
        CM_RAINBOW,
        CM_HOTMETAL,
        CM_HOTIRON,
        CM_SPECTRUM,
        CM_PETCOLOR,
        CM_FLOW,
        CM_LONI,
        CM_LONI2,
        CM_ASYMMETRY,
        CM_PVALUE,
        CM_MULTILABEL,
        CM_LEGACYBINARY
      //  Default = CM_BW;
      };

    /**
    * Returns the current rendering mode
    */
    virtual int GetColormap();

    using BaseProperty::operator=;

  protected:
    ColormapProperty();
    ColormapProperty( const IdType& value );
    ColormapProperty( const std::string& value );

    virtual ~ColormapProperty();
    virtual void AddEnumerationTypes();

  private:

    // purposely not implemented
    const ColormapProperty& operator=(const ColormapProperty&);

    virtual itk::LightObject::Pointer InternalClone() const;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace

#endif
