/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:31:16 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkOdfNormalizationMethodProperty.h"

// ODFN_MINMAX, ODFN_MAX, ODFN_NONE ODFN_GLOBAL_MAX

mitk::OdfNormalizationMethodProperty::OdfNormalizationMethodProperty( )
{
  this->AddInterpolationTypes();
  this->SetValue( static_cast<IdType>( ODFN_MINMAX ) );
}


mitk::OdfNormalizationMethodProperty::OdfNormalizationMethodProperty( const IdType& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value ) ;
  }
  else
  {
    this->SetValue( static_cast<IdType>( ODFN_MINMAX ) );
  }
}

mitk::OdfNormalizationMethodProperty::OdfNormalizationMethodProperty( const std::string& value )
{
  this->AddInterpolationTypes();
  if ( IsValidEnumerationValue( value ) )
  {
    this->SetValue( value );
  }
  else
  {
    this->SetValue( static_cast<IdType>( ODFN_MINMAX ) );
  }
}


int mitk::OdfNormalizationMethodProperty::GetNormalization()
{
  return static_cast<int>( this->GetValueAsId() );
}


void mitk::OdfNormalizationMethodProperty::SetNormalizationToMinMax()
{
  this->SetValue( static_cast<IdType>( ODFN_MINMAX ) );
}


void mitk::OdfNormalizationMethodProperty::SetNormalizationToMax()
{
  this->SetValue( static_cast<IdType>( ODFN_MAX ) );
}


void mitk::OdfNormalizationMethodProperty::SetNormalizationToNone()
{
  this->SetValue( static_cast<IdType>( ODFN_NONE ) );
}


void mitk::OdfNormalizationMethodProperty::SetNormalizationToGlobalMax()
{
  this->SetValue( static_cast<IdType>( ODFN_GLOBAL_MAX ) );
}


void mitk::OdfNormalizationMethodProperty::AddInterpolationTypes()
{
  AddEnum( "Min-Max", static_cast<IdType>( ODFN_MINMAX ) );
  AddEnum( "Maximum", static_cast<IdType>( ODFN_MAX ) );
  AddEnum( "None", static_cast<IdType>( ODFN_NONE ) );
  AddEnum( "Global Maximum", static_cast<IdType>( ODFN_GLOBAL_MAX ) );
}


bool mitk::OdfNormalizationMethodProperty::AddEnum( const std::string& name, const IdType& id )
{
  return Superclass::AddEnum( name, id );  
}
