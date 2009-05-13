/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14605 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKDelegateProperty_H_HEADER_INCLUDED_C1C02491
#define MITKDelegateProperty_H_HEADER_INCLUDED_C1C02491

#include <itkConfigure.h>

#include "mitkCommon.h"
#include "mitkStringProperty.h"

namespace mitk {

/**
 * @brief Property for commands.
 * @ingroup DataTree
 *
 * A command property saves the name of 
 */
  class MITKEXT_CORE_EXPORT DelegateProperty : public StringProperty
  {
    protected:
      DelegateProperty( const char* string = 0 );
      DelegateProperty( const std::string&  s );

    public:
      mitkClassMacro(DelegateProperty, StringProperty);
      itkNewMacro(DelegateProperty);
      mitkNewMacro1Param(DelegateProperty, const char*);
      mitkNewMacro1Param(DelegateProperty, const std::string&);

      virtual bool operator==(const DelegateProperty& property ) const;
      virtual DelegateProperty& operator=(const DelegateProperty& other);

      virtual bool WriteXMLData( XMLWriter& xmlWriter );
      virtual bool ReadXMLData( XMLReader& xmlReader );

  };

} // namespace mitk

#endif 

