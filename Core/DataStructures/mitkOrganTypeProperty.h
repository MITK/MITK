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
#include "mitkEnumerationProperty.h"
#include <itkObjectFactory.h>

namespace mitk
{

/**
  \brief Enumerates all known organs :-)

  \sa QmitkSliceBasedSegmentation
  \ingroup Reliver
  \ingroup DataTree

  Last contributor $Author$
*/
class OrganTypeProperty : public EnumerationProperty
{
  public:
    
    mitkClassMacro(OrganTypeProperty, EnumerationProperty);
    itkNewMacro(OrganTypeProperty);

    OrganTypeProperty();
    OrganTypeProperty( const IdType& value );
    OrganTypeProperty( const std::string& value );

  protected:

    virtual ~OrganTypeProperty();
    virtual void AddEnumerationTypes();
};

} // namespace

#endif

