/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-17 13:14:37 +0100 (Di, 17 Feb 2009) $
Version:   $Revision: 16318 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkModalityProperty.h"

mitk::ModalityProperty::ModalityProperty()
{
  AddEnumerationTypes();
}

mitk::ModalityProperty::ModalityProperty( const IdType& value )
{
  AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value ) ;
  }
  else
  {
    SetValue( 0 );
  }
}

mitk::ModalityProperty::ModalityProperty( const std::string& value )
{
  AddEnumerationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    SetValue( value );
  }
  else
  {
    SetValue( "undefined" );
  }
}


mitk::ModalityProperty::~ModalityProperty()
{
}

    
void mitk::ModalityProperty::AddEnumerationTypes()
{
  IdType newId = EnumerationProperty::Size();

  AddEnum( "undefined", newId++ );
  AddEnum( "CR", newId++ ); // computer radiography
  AddEnum( "CT", newId++ ); // computed tomography
  AddEnum( "MR", newId++ ); // magnetic resonance
  AddEnum( "NM", newId++ ); // nuclear medicine
  AddEnum( "US", newId++ ); // ultrasound
  AddEnum( "Color Doppler", newId++ ); // ultrasound
  AddEnum( "Power Doppler", newId++ ); // ultrasound
}


