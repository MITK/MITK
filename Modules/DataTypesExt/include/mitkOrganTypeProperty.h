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

#ifndef mitkOrganTypeProperty_h_Included
#define mitkOrganTypeProperty_h_Included

#include "mitkCommon.h"
#include "MitkDataTypesExtExports.h"
#include "mitkEnumerationProperty.h"
#include <itkObjectFactory.h>

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
  \brief Enumerates all known organs :-)

  \sa QmitkInteractiveSegmentation
  \ingroup ToolManagerEtAl
  \ingroup DataManagement

  Last contributor $Author$
*/
class MITKDATATYPESEXT_EXPORT OrganTypeProperty : public EnumerationProperty
{
  public:

    mitkClassMacro(OrganTypeProperty, EnumerationProperty);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro1Param(OrganTypeProperty, const IdType&);
    mitkNewMacro1Param(OrganTypeProperty, const std::string&);

    using BaseProperty::operator=;

  protected:
    OrganTypeProperty();
    OrganTypeProperty( const IdType& value );
    OrganTypeProperty( const std::string& value );

    virtual ~OrganTypeProperty();
    virtual void AddEnumerationTypes();

  private:

    // purposely not implemented
    OrganTypeProperty& operator=(const OrganTypeProperty&);

    virtual itk::LightObject::Pointer InternalClone() const override;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // namespace

#endif
