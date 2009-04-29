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


#include "mitkLookupTable.h"
#include <itkProcessObject.h>
#include <itkSmartPointerForwardReference.txx>
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

const std::string mitk::LookupTable::XML_NODE_NAME = "lookupTable";
const std::string mitk::LookupTable::TABLE_RANGE = "tableRange";
const std::string mitk::LookupTable::TABLE_VALUE = "tableValue";
const std::string mitk::LookupTable::NUMBER_OF_COLORS = "NUMBER_OF_COLORS";
const std::string mitk::LookupTable::TABLE_LOWER_RANGE = "TABLE_LOWER_RANGE";
const std::string mitk::LookupTable::TABLE_UPPER_RANGE = "TABLE_UPPER_RANGE";
const std::string mitk::LookupTable::VALUE_RANGE = "valueRange";
const std::string mitk::LookupTable::VALUE_LOWER_RANGE = "VALUE_LOWER_RANGE";
const std::string mitk::LookupTable::VALUE_UPPER_RANGE = "VALUE_UPPER_RANGE";
const std::string mitk::LookupTable::HUE_RANGE = "hueRange";
const std::string mitk::LookupTable::HUE_LOWER_RANGE = "HUE_LOWER_RANGE";
const std::string mitk::LookupTable::HUE_UPPER_RANGE = "HUE_UPPER_RANGE";
const std::string mitk::LookupTable::SATURATION_RANGE = "saturationRange";
const std::string mitk::LookupTable::SATURATION_LOWER_RANGE = "SATURATION_LOWER_RANGE";
const std::string mitk::LookupTable::SATURATION_UPPER_RANGE = "SATURATION_UPPER_RANGE";
const std::string mitk::LookupTable::ALPHA_RANGE = "alphaRange";
const std::string mitk::LookupTable::ALPHA_LOWER_RANGE = "ALPHA_LOWER_RANGE";
const std::string mitk::LookupTable::ALPHA_UPPER_RANGE = "ALPHA_UPPER_RANGE";
const std::string mitk::LookupTable::SCALE = "SCALE";
const std::string mitk::LookupTable::RAMP = "RAMP";

mitk::LookupTable::LookupTable()
{
  m_LookupTable = vtkLookupTable::New();
  this->SetRequestedRegionToLargestPossibleRegion();
}


mitk::LookupTable::~LookupTable()
{
  if ( m_LookupTable )
  {
    m_LookupTable->Delete();   
    m_LookupTable = NULL;
  }
}

void mitk::LookupTable::SetVtkLookupTable( vtkLookupTable* lut )
{
  
  if(m_LookupTable == lut)
  {
    return;
  }

  if(m_LookupTable)
  {
    m_LookupTable->UnRegister(NULL);
    m_LookupTable = NULL;
  }

  if(lut)
  {
    lut->Register(NULL);    
  }

  m_LookupTable = lut;
  this->Modified();  

}



void mitk::LookupTable::ChangeOpacityForAll( float opacity )
{

  int noValues = m_LookupTable->GetNumberOfTableValues ();

  vtkFloatingPointType rgba[ 4 ];

  for ( int i = 0;i < noValues;i++ )
  {
    m_LookupTable->GetTableValue ( i, rgba );
    rgba[ 3 ] = opacity;
    m_LookupTable->SetTableValue ( i, rgba );
  }
  this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-updat is executed
}

void mitk::LookupTable::ChangeOpacity(int index, float opacity )
{

  int noValues = m_LookupTable->GetNumberOfTableValues ();
  if (index>noValues)
  {
    std::cout << "could not change opacity. index exceed size of lut ... " << std::endl;
    return;
  }

  vtkFloatingPointType rgba[ 4 ];

  m_LookupTable->GetTableValue ( index, rgba );
  rgba[ 3 ] = opacity;
  m_LookupTable->SetTableValue ( index, rgba );

  this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-updat is executed
}


vtkLookupTable* mitk::LookupTable::GetVtkLookupTable() const
{
  return m_LookupTable;
};

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{

  if (m_LookupTable==NULL) std::cout << "uuups..." << std::endl;
  return m_LookupTable->GetPointer( 0 );
};

/*!
* \brief equality operator inplementation
*/
bool mitk::LookupTable::operator==( const mitk::LookupTable& LookupTable ) const
{
  if ( m_LookupTable == LookupTable.GetVtkLookupTable() )
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*!
* \brief equality operator inplementation
*/
bool mitk::LookupTable::operator!=( const mitk::LookupTable& LookupTable ) const
{

  if ( m_LookupTable == ( LookupTable.GetVtkLookupTable() ) )
  {
    return false;
  }
  else
  {
    return true;
  }

}

/*!
* \brief non equality operator inplementation
*/
mitk::LookupTable& mitk::LookupTable::operator=( const mitk::LookupTable& LookupTable )
{
  if ( this == &LookupTable )
  {
    return * this;
  }
  else
  {
    m_LookupTable = LookupTable.GetVtkLookupTable();
    return *this;
  }
}

void mitk::LookupTable::UpdateOutputInformation( )
{
  if ( this->GetSource( ) )
  {
    this->GetSource( ) ->UpdateOutputInformation( );
  }
}


void mitk::LookupTable::SetRequestedRegionToLargestPossibleRegion( )
{}


bool mitk::LookupTable::RequestedRegionIsOutsideOfTheBufferedRegion( )
{
  return false;
}


bool mitk::LookupTable::VerifyRequestedRegion( )
{
  //normally we should check if the requested region lies within the
  //largest possible region. Since for lookup-tables we assume, that the
  //requested region is always the largest possible region, we can always
  //return true!
  return true;
}


void mitk::LookupTable::SetRequestedRegion( itk::DataObject *)
{
  //not implemented, since we always want to have the RequestedRegion
  //to be set to LargestPossibleRegion
}

void mitk::LookupTable::CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction)
{
  if(colorFunction==NULL)
    colorFunction = vtkColorTransferFunction::New();

  mitk::LookupTable::RawLookupTableType *rgba = GetRawLookupTable();
  int i, num_of_values=m_LookupTable->GetNumberOfTableValues();


  vtkFloatingPointType *cols;
  vtkFloatingPointType *colsHead;
  colsHead=cols=(vtkFloatingPointType *)malloc(sizeof(vtkFloatingPointType)*num_of_values*3);

  for(i=0;i<num_of_values;++i)
  {
    *cols=*rgba/255.0; ++cols; ++rgba;
    *cols=*rgba/255.0; ++cols; ++rgba;
    *cols=*rgba/255.0; ++cols; ++rgba;
    ++rgba;
  }
  colorFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values-1, colsHead);

  free(colsHead);
}

void mitk::LookupTable::CreateOpacityTransferFunction(vtkPiecewiseFunction*& opacityFunction)
{
  if(opacityFunction==NULL)
    opacityFunction = vtkPiecewiseFunction::New();

  mitk::LookupTable::RawLookupTableType *rgba = GetRawLookupTable();
  int i, num_of_values=m_LookupTable->GetNumberOfTableValues();

  vtkFloatingPointType *alphas;
  vtkFloatingPointType *alphasHead;
  alphasHead=alphas=(vtkFloatingPointType*)malloc(sizeof(vtkFloatingPointType)*num_of_values);

  rgba+=3;
  for(i=0;i<num_of_values;++i)
  {
    *alphas=*rgba * 1024.0; ++alphas; rgba+=4;
  }

  opacityFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values-1, alphasHead);

  free(alphasHead);
}

void mitk::LookupTable::CreateGradientTransferFunction(vtkPiecewiseFunction*& gradientFunction)
{
  if(gradientFunction==NULL)
    gradientFunction = vtkPiecewiseFunction::New();

  mitk::LookupTable::RawLookupTableType *rgba = GetRawLookupTable();
  int i, num_of_values=m_LookupTable->GetNumberOfTableValues();

  vtkFloatingPointType *alphas;
  vtkFloatingPointType *alphasHead;
  alphasHead=alphas=(vtkFloatingPointType*)malloc(sizeof(vtkFloatingPointType)*num_of_values);

  rgba+=3;
  for(i=0;i<num_of_values;++i)
  {
    *alphas=*rgba * 1024.0; ++alphas; rgba+=4;
  }

  gradientFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values-1, alphasHead);

  free(alphasHead);
}

bool mitk::LookupTable::WriteXMLData( XMLWriter& xmlWriter )
{
  vtkFloatingPointType color[ 4 ], lowerRange, upperRange;

  xmlWriter.WriteProperty( NUMBER_OF_COLORS, m_LookupTable->GetNumberOfColors() );
  xmlWriter.WriteProperty( SCALE, m_LookupTable->GetScale() );
  xmlWriter.WriteProperty( RAMP, m_LookupTable->GetRamp() );
  xmlWriter.BeginNode( VALUE_RANGE );
  xmlWriter.WriteProperty( VALUE_LOWER_RANGE, m_LookupTable->GetValueRange()[0] );
  xmlWriter.WriteProperty( VALUE_UPPER_RANGE, m_LookupTable->GetValueRange()[1] );
  xmlWriter.EndNode();
  xmlWriter.BeginNode( HUE_RANGE );
  xmlWriter.WriteProperty( HUE_LOWER_RANGE, m_LookupTable->GetHueRange()[0] );
  xmlWriter.WriteProperty( HUE_UPPER_RANGE, m_LookupTable->GetHueRange()[1] );
  xmlWriter.EndNode();
  xmlWriter.BeginNode( SATURATION_RANGE );
  xmlWriter.WriteProperty( SATURATION_LOWER_RANGE, m_LookupTable->GetSaturationRange()[0] );
  xmlWriter.WriteProperty( SATURATION_UPPER_RANGE, m_LookupTable->GetSaturationRange()[1] );
  xmlWriter.EndNode();
  xmlWriter.BeginNode( ALPHA_RANGE );
  xmlWriter.WriteProperty( ALPHA_LOWER_RANGE, m_LookupTable->GetAlphaRange()[0] );
  xmlWriter.WriteProperty( ALPHA_UPPER_RANGE, m_LookupTable->GetAlphaRange()[1] );
  xmlWriter.EndNode();
  xmlWriter.BeginNode(TABLE_RANGE);
  lowerRange = m_LookupTable->GetTableRange()[0];
  upperRange = m_LookupTable->GetTableRange()[1];
  xmlWriter.WriteProperty( TABLE_LOWER_RANGE, lowerRange );
  xmlWriter.WriteProperty( TABLE_UPPER_RANGE, upperRange );
  for(int i=(int)lowerRange; i<=(int)upperRange; ++i){
    xmlWriter.BeginNode(TABLE_VALUE);
    xmlWriter.WriteProperty( "INDEX", i );
    m_LookupTable->GetTableValue(i, color);
    xmlWriter.WriteProperty( "COLOR", color );
    xmlWriter.EndNode();
  }
  xmlWriter.EndNode();
  return true;
}

bool mitk::LookupTable::ReadXMLData( XMLReader& xmlReader )
{

  vtkFloatingPointType color[ 4 ], lowerRange, upperRange;

  int index;
  XMLReader::RGBAType rgba;

  if(xmlReader.Goto(VALUE_RANGE)){
    xmlReader.GetAttribute(VALUE_LOWER_RANGE, lowerRange);
    xmlReader.GetAttribute(VALUE_UPPER_RANGE, upperRange);
    m_LookupTable->SetValueRange(lowerRange, upperRange);
    xmlReader.GotoParent();
  }

  if(xmlReader.Goto(HUE_RANGE)){
    xmlReader.GetAttribute(HUE_LOWER_RANGE, lowerRange);
    xmlReader.GetAttribute(HUE_UPPER_RANGE, upperRange);
    m_LookupTable->SetHueRange(lowerRange, upperRange);
    xmlReader.GotoParent();
  }

  if(xmlReader.Goto(SATURATION_RANGE)){
    xmlReader.GetAttribute(SATURATION_LOWER_RANGE, lowerRange);
    xmlReader.GetAttribute(SATURATION_UPPER_RANGE, upperRange);
    m_LookupTable->SetSaturationRange(lowerRange, upperRange);
    xmlReader.GotoParent();
  }

  if(xmlReader.Goto(ALPHA_RANGE)){
    xmlReader.GetAttribute(ALPHA_LOWER_RANGE, lowerRange);
    xmlReader.GetAttribute(ALPHA_UPPER_RANGE, upperRange);
    m_LookupTable->SetAlphaRange(lowerRange, upperRange);
    xmlReader.GotoParent();
  }

  if(xmlReader.Goto(TABLE_RANGE)){
    xmlReader.GetAttribute(TABLE_LOWER_RANGE, lowerRange);
    xmlReader.GetAttribute(TABLE_UPPER_RANGE, upperRange);
    m_LookupTable->SetTableRange(lowerRange, upperRange);
    if(xmlReader.Goto(TABLE_VALUE)){
      for(int i = (int)lowerRange; i<=(int)upperRange; ++i){
        if (xmlReader.GetAttribute("INDEX", index) && xmlReader.GetAttribute("COLOR", rgba)){
          for(int j=0; j<4; ++j)
            color[j]=rgba[j];
          m_LookupTable->SetTableValue(index, color);
          xmlReader.GotoNext();
        }
      }
    }
    this->UpdateOutputInformation();
    xmlReader.GotoParent();
  }
  std::cout << "read mitk::LookupTable: " << " " << std::endl;

  return true;
}

const std::string& mitk::LookupTable::GetXMLNodeName() const
{
  return XML_NODE_NAME;
}
