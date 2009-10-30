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

#include "mitkOrganTypeProperty.h"

mitk::OrganTypeProperty::OrganTypeProperty()
{
  AddEnumerationTypes();
}

mitk::OrganTypeProperty::OrganTypeProperty( const IdType& value )
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

mitk::OrganTypeProperty::OrganTypeProperty( const std::string& value )
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


mitk::OrganTypeProperty::~OrganTypeProperty()
{
}

    
void mitk::OrganTypeProperty::AddEnumerationTypes()
{
  IdType newId = static_cast<IdType>(EnumerationProperty::Size());

  // On changes, please also change mitk::DataTreeNodeFactory::DefaultColorForOrgan()

  AddEnum( "undefined", newId++ );
  AddEnum( "Ankle", newId++ );
  AddEnum( "Appendix", newId++ );
  AddEnum( "Blood vessels", newId++ );
  AddEnum( "Bone", newId++ );
  AddEnum( "Brain", newId++ );
  AddEnum( "Bronchial tree", newId++ );
  AddEnum( "Coccyx", newId++ );
  AddEnum( "Colon", newId++ );
  AddEnum( "Cyst", newId++ );
  AddEnum( "Elbow", newId++ );
  AddEnum( "Eye", newId++ );
  AddEnum( "Fallopian tube", newId++ );
  AddEnum( "Fat", newId++ );
  AddEnum( "Gall bladder", newId++ );
  AddEnum( "Hand", newId++ );
  AddEnum( "Heart", newId++ );
  AddEnum( "Hip", newId++ );
  AddEnum( "Hippocampus", newId++ );
  AddEnum( "Kidney", newId++ );
  AddEnum( "Knee", newId++ );
  AddEnum( "Larynx", newId++ );
  AddEnum( "Liver", newId++ );
  AddEnum( "Lung", newId++ );
  AddEnum( "Lymph node", newId++ );
  AddEnum( "Muscle", newId++ );
  AddEnum( "Nerve", newId++ );
  AddEnum( "Nose", newId++ );
  AddEnum( "Oesophagus", newId++ );
  AddEnum( "Ovaries", newId++ );
  AddEnum( "Pancreas", newId++ );
  AddEnum( "Pelvis", newId++ );
  AddEnum( "Penis", newId++ );
  AddEnum( "Pharynx", newId++ );
  AddEnum( "Prostate", newId++ );
  AddEnum( "Rectum", newId++ );
  AddEnum( "Sacrum", newId++ );
  AddEnum( "Seminal vesicle", newId++ );
  AddEnum( "Shoulder", newId++ );
  AddEnum( "Spinal cord", newId++ );
  AddEnum( "Spleen", newId++ );
  AddEnum( "Stomach", newId++ );
  AddEnum( "Teeth", newId++ );
  AddEnum( "Testicles", newId++ );
  AddEnum( "Thyroid", newId++ );
  AddEnum( "Tongue", newId++ );
  AddEnum( "Tumor", newId++ );
  AddEnum( "Urethra", newId++ );
  AddEnum( "Urinary bladder", newId++ );
  AddEnum( "Uterus", newId++ );
  AddEnum( "Vagina", newId++ );
  AddEnum( "Vertebra", newId++ );
  AddEnum( "Wrist", newId++ );
}
