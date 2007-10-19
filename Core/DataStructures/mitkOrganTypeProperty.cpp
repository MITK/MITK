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
  IdType newId = EnumerationProperty::Size();

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
  AddEnum( "Hand", newId++ );
  AddEnum( "Heart", newId++ );
  AddEnum( "Hip", newId++ );
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

