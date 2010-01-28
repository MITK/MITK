/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkOrganTypeProperty_h_Included
#define mitkOrganTypeProperty_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkEnumerationProperty.h"
#include <itkObjectFactory.h>

namespace mitk
{

/**
  \brief Enumerates all known organs :-)

  \sa QmitkInteractiveSegmentation
  \ingroup Reliver
  \ingroup DataManagement

  Last contributor $Author$
*/
class MitkExt_EXPORT OrganTypeProperty : public EnumerationProperty
{
  public:
    
    mitkClassMacro(OrganTypeProperty, EnumerationProperty);
    itkNewMacro(OrganTypeProperty);
    mitkNewMacro1Param(OrganTypeProperty, const IdType&);
    mitkNewMacro1Param(OrganTypeProperty, const std::string&);

  protected:
    OrganTypeProperty();
    OrganTypeProperty( const IdType& value );
    OrganTypeProperty( const std::string& value );
    
    virtual ~OrganTypeProperty();
    virtual void AddEnumerationTypes();
};

} // namespace

#endif


