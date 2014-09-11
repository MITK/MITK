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


#include "mitkLookupTable.h"
#include <itkProcessObject.h>

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

#include <mitkLookupTableProperty.h>
#include <Colortables/Jet.h>
#include <Colortables/HotIron.h>
#include <Colortables/PETColor.h>
#include <Colortables/PET20.h>

const char* const mitk::LookupTable::typenameList[] =
{
  "Grayscale",
  "Inverse Grayscale",
  "Hot Iron",
  "Jet",
  "Legacy Binary",
  "Legacy Rainbow Color",
  "Multilabel",
  "PET Color",
  "PET 20",
  "END_OF_ARRAY" // Do not add typenames after this entry (see QmitkDataManagerView::ColormapMenuAboutToShow())
};

mitk::LookupTable::LookupTable():
  m_Window(0.0),
  m_Level(0.0),
  m_Opacity(1.0),
  m_type(mitk::LookupTable::GRAYSCALE)
{
  m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();
  this->BuildGrayScaleLookupTable();
}

mitk::LookupTable::LookupTable(const LookupTable& other)
  : itk::DataObject()
  , m_LookupTable(vtkSmartPointer<vtkLookupTable>::New())
{
  m_LookupTable->DeepCopy(other.m_LookupTable);
}

mitk::LookupTable::~LookupTable()
{
}

void mitk::LookupTable::SetVtkLookupTable( vtkSmartPointer<vtkLookupTable> lut )
{
  if ((!lut) || (m_LookupTable == lut))
  {
    return;
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::SetType(const mitk::LookupTable::LookupTableType type)
{
  if (m_type == type)
    return;

  switch(type)
  {
  case (mitk::LookupTable::GRAYSCALE):
    this->BuildGrayScaleLookupTable();
    break;
  case (mitk::LookupTable::INVERSE_GRAYSCALE):
    this->BuildInverseGrayScaleLookupTable();
    break;
  case (mitk::LookupTable::HOT_IRON):
    this->BuildHotIronLookupTable();
    break;
  case (mitk::LookupTable::JET):
    this->BuildJetLookupTable();
    break;
  case (mitk::LookupTable::LEGACY_BINARY):
    this->BuildLegacyBinaryLookupTable();
    break;
  case (mitk::LookupTable::MULTILABEL):
    this->BuildMultiLabelLookupTable();
    break;
  case (mitk::LookupTable::PET_COLOR):
    this->BuildPETColorLookupTable();
    break;
  case (mitk::LookupTable::PET_20):
    this->BuildPET20LookupTable();
    break;
  case (mitk::LookupTable::LEGACY_RAINBOW_COLOR):
    this->BuildLegacyRainbowColorLookupTable();
    break;
  default:
    MITK_ERROR << "non-existing colormap";
    return;
  }

  m_type = type;
}

void mitk::LookupTable::SetType(const std::string& typeName)
{
  int i = 0;
  std::string lutType = this->typenameList[i];

  while (lutType != "END_OF_ARRAY")
  {
    if (lutType == typeName)
    {
      this->SetType(static_cast<mitk::LookupTable::LookupTableType>(i));
    }

    lutType = this->typenameList[++i];
  }
}

const std::string mitk::LookupTable::GetActiveTypeAsString()
{
  return std::string(typenameList[(int)m_type]);
}

void mitk::LookupTable::ChangeOpacityForAll( float opacity )
{

  int noValues = m_LookupTable->GetNumberOfTableValues ();

  double rgba[ 4 ];

  for ( int i = 0;i < noValues;i++ )
  {
    m_LookupTable->GetTableValue ( i, rgba );
    rgba[ 3 ] = opacity;
    m_LookupTable->SetTableValue ( i, rgba );
  }
  this->Modified();  // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is executed
}

void mitk::LookupTable::ChangeOpacity(int index, float opacity )
{

  int noValues = m_LookupTable->GetNumberOfTableValues ();
  if (index>noValues)
  {
    MITK_INFO << "could not change opacity. index exceed size of lut ... " << std::endl;
    return;
  }

  double rgba[ 4 ];

  m_LookupTable->GetTableValue ( index, rgba );
  rgba[ 3 ] = opacity;
  m_LookupTable->SetTableValue ( index, rgba );

  this->Modified();  // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is executed
}

void mitk::LookupTable::GetColor(int x, double rgb[3])
{
  this->GetVtkLookupTable()->GetColor(x,rgb);
}

void mitk::LookupTable::GetTableValue(int x, double rgba[4])
{
  this->GetVtkLookupTable()->GetTableValue(x,rgba);
}

void mitk::LookupTable::SetTableValue(int x, double rgba[4])
{
  this->GetVtkLookupTable()->SetTableValue(x,rgba);
}

vtkSmartPointer<vtkLookupTable> mitk::LookupTable::GetVtkLookupTable() const
{
  return m_LookupTable;
}

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{
  return m_LookupTable->GetPointer( 0 );
}

/*!
* \brief equality operator inplementation
*/
bool mitk::LookupTable::operator==( const mitk::LookupTable& other ) const
{
  if ( m_LookupTable == other.GetVtkLookupTable())
    return true;
  vtkLookupTable* olut = other.GetVtkLookupTable();
  if (olut == NULL)
    return false;

  bool equal = (m_LookupTable->GetNumberOfColors() == olut->GetNumberOfColors())
      && (m_LookupTable->GetTableRange()[0] == olut->GetTableRange()[0])
      && (m_LookupTable->GetTableRange()[1] == olut->GetTableRange()[1])
      && (m_LookupTable->GetHueRange()[0] == olut->GetHueRange()[0])
      && (m_LookupTable->GetHueRange()[1] == olut->GetHueRange()[1])
      && (m_LookupTable->GetSaturationRange()[0] == olut->GetSaturationRange()[0])
      && (m_LookupTable->GetSaturationRange()[1] == olut->GetSaturationRange()[1])
      && (m_LookupTable->GetValueRange()[0] == olut->GetValueRange()[0])
      && (m_LookupTable->GetValueRange()[1] == olut->GetValueRange()[1])
      && (m_LookupTable->GetAlphaRange()[0] == olut->GetAlphaRange()[0])
      && (m_LookupTable->GetAlphaRange()[1] == olut->GetAlphaRange()[1])
      && (m_LookupTable->GetRamp() == olut->GetRamp())
      && (m_LookupTable->GetScale() == olut->GetScale())
      && (m_LookupTable->GetAlpha() == olut->GetAlpha())
      && (m_LookupTable->GetTable()->GetNumberOfTuples() == olut->GetTable()->GetNumberOfTuples());
  if (equal == false)
    return false;
  for (vtkIdType i=0; i < m_LookupTable->GetNumberOfTableValues(); i++)
  {
    bool tvequal = (m_LookupTable->GetTableValue(i)[0] == olut->GetTableValue(i)[0])
        && (m_LookupTable->GetTableValue(i)[1] == olut->GetTableValue(i)[1])
        && (m_LookupTable->GetTableValue(i)[2] == olut->GetTableValue(i)[2])
        && (m_LookupTable->GetTableValue(i)[3] == olut->GetTableValue(i)[3]);
    if (tvequal == false)
      return false;
  }
  return true;
}

/*!
* \brief un-equality operator implementation
*/
bool mitk::LookupTable::operator!=( const mitk::LookupTable& other ) const
{
  return !(*this == other);
}

/*!
* \brief assignment operator implementation
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


void mitk::LookupTable::SetRequestedRegion(const itk::DataObject *)
{
  //not implemented, since we always want to have the RequestedRegion
  //to be set to LargestPossibleRegion
}

vtkSmartPointer<vtkColorTransferFunction> mitk::LookupTable::CreateColorTransferFunction()
{
  vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();

  mitk::LookupTable::RawLookupTableType *rawLookupTable = this->GetRawLookupTable();
  int num_of_values = m_LookupTable->GetNumberOfTableValues();

  double* cols = new double[3*num_of_values];
  double* colsHead = cols;

  for (int i = 0; i<num_of_values; ++i)
  {
    *cols=static_cast<double>(*rawLookupTable)/255.0; ++cols; ++rawLookupTable;
    *cols=static_cast<double>(*rawLookupTable)/255.0; ++cols; ++rawLookupTable;
    *cols=static_cast<double>(*rawLookupTable)/255.0; ++cols; ++rawLookupTable;
    ++rawLookupTable;
  }
  colorFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values, colsHead);
  return colorFunction;
}

void mitk::LookupTable::CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction)
{
  colorFunction = this->CreateColorTransferFunction();
}

vtkSmartPointer<vtkPiecewiseFunction> mitk::LookupTable::CreateOpacityTransferFunction()
{
  vtkSmartPointer<vtkPiecewiseFunction> opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

  mitk::LookupTable::RawLookupTableType *rgba = this->GetRawLookupTable();
  int num_of_values=m_LookupTable->GetNumberOfTableValues();

  double *alphas = new double [num_of_values];
  double *alphasHead = alphas;

  rgba+=3;
  for(int i=0;i<num_of_values;++i)
  {
    *alphas=static_cast<double>(*rgba)/255.0; ++alphas; rgba+=4;
  }

  opacityFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values, alphasHead);
  return opacityFunction;
}

void mitk::LookupTable::CreateOpacityTransferFunction(vtkPiecewiseFunction*& opacityFunction)
{
  opacityFunction = this->CreateOpacityTransferFunction();
}

vtkSmartPointer<vtkPiecewiseFunction> mitk::LookupTable::CreateGradientTransferFunction()
{
  vtkSmartPointer<vtkPiecewiseFunction> gradientFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

  mitk::LookupTable::RawLookupTableType *rgba = this->GetRawLookupTable();
  int num_of_values=m_LookupTable->GetNumberOfTableValues();

  double *alphas = new double [num_of_values];
  double *alphasHead = alphas;

  rgba+=3;
  for(int i=0;i<num_of_values;++i)
  {
    *alphas=static_cast<double>(*rgba)/255.0; ++alphas; rgba+=4;
  }

  gradientFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values, alphasHead);
  return gradientFunction;
}

void mitk::LookupTable::CreateGradientTransferFunction(vtkPiecewiseFunction*& gradientFunction)
{
  gradientFunction = this->CreateGradientTransferFunction();
}

void mitk::LookupTable::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  os << indent;
  m_LookupTable->PrintHeader(os, vtkIndent());
}

itk::LightObject::Pointer mitk::LookupTable::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

void mitk::LookupTable::BuildGrayScaleLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetRampToLinear();
  lut->SetSaturationRange( 0.0, 0.0 );
  lut->SetHueRange( 0.0, 0.0 );
  lut->SetValueRange( 0.0, 1.0 );
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildLegacyBinaryLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetRampToLinear();
  lut->SetSaturationRange( 0.0, 0.0 );
  lut->SetHueRange( 0.0, 0.0 );
  lut->SetValueRange( 0.0, 1.0 );
  lut->Build();
  lut->SetTableValue(0,0.0,0.0,0.0,0.0);

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildInverseGrayScaleLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetTableRange (0, 1);
  lut->SetSaturationRange (0, 0);
  lut->SetHueRange (0, 0);
  lut->SetValueRange (1, 0);
  lut->SetAlphaRange (1, 0);
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildHotIronLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {

    lut->SetTableValue(i, (double)HotIron[i][0]/255.0, (double)HotIron[i][1]/255.0, (double)HotIron[i][2]/255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildJetLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for (int i = 0; i < 256; i++)
  {
    lut->SetTableValue(i, (double)Jet[i][0] / 255.0, (double)Jet[i][1] / 255.0, (double)Jet[i][2] / 255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildPETColorLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(256);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)PETColor[i][0]/255.0, (double)PETColor[i][1]/255.0, (double)PETColor[i][2]/255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildPET20LookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues(256);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)PET20[i][0]/255.0, (double)PET20[i][1]/255.0, (double)PET20[i][2]/255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildMultiLabelLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetNumberOfTableValues (256);
  lut->SetTableRange ( 0, 255 );
  lut->SetTableValue (0, 0.0, 0.0, 0.0, 0.0); // background

  lut->SetTableValue (1, 1.0, 1.0, 0.0, 0.4);
  lut->SetTableValue (2, 0.0, 1.0, 0.0, 0.4);
  lut->SetTableValue (3, 0.0, 0.0, 1.0, 0.4);
  lut->SetTableValue (4, 1.0, 1.0, 0.4, 0.4);
  lut->SetTableValue (5, 0.0, 0.4, 0.7, 0.4);
  lut->SetTableValue (6, 1.0, 0.0, 1.0, 0.4);

  lut->SetTableValue (7, 1.0, 0.5, 0.0, 0.4);
  lut->SetTableValue (8, 0.0, 1.0, 0.5, 0.4);
  lut->SetTableValue (9, 0.5, 0.0, 1.0, 0.4);
  lut->SetTableValue (10, 1.0, 1.0, 0.5, 0.4);
  lut->SetTableValue (11, 0.5, 1.0, 1.0, 0.4);
  lut->SetTableValue (12, 1.0, 0.5, 0.6, 0.4);
  lut->SetTableValue (13, 1.0, 0.3, 0.3, 0.4);
  lut->SetTableValue (14, 0.4, 0.7, 1.0, 0.4);
  lut->SetTableValue (15, 0.4, 0.5, 1.0, 0.4);
  lut->SetTableValue (16, 0.8, 0.5, 1.0, 0.4);
  lut->SetTableValue (17, 1.0, 0.3, 1.0, 0.4);
  lut->SetTableValue (18, 1.0, 0.5, 0.6, 0.4);
  lut->SetTableValue (19, 1.0, 0.5, 0.4, 0.4);
  lut->SetTableValue (20, 0.4, 0.5, 0.4, 0.4);
  lut->SetTableValue (21, 1.0, 0.5, 0.76, 0.4);
  lut->SetTableValue (22, 0.76, 0.4, 0.4, 0.4);
  lut->SetTableValue (23, 1.0, 0.5, 0.4, 0.4);
  lut->SetTableValue (24, 0.76, 0.3, 0.4, 0.4);
  lut->SetTableValue (25, 1.0, 0.3, 0.4, 0.4);

  for (int i = 26; i < 256; i++)
  {
    if (i%12 == 0)
      lut->SetTableValue (i, 1.0, 0.0, 0.0, 0.4);
    else if (i%12 == 1)
      lut->SetTableValue (i, 0.0, 1.0, 0.0, 0.4);
    else if (i%12 == 2)
      lut->SetTableValue (i, 0.0, 0.0, 1.0, 0.4);
    else if (i%12 == 3)
      lut->SetTableValue (i, 1.0, 1.0, 0.0, 0.4);
    else if (i%12 == 4)
      lut->SetTableValue (i, 0.0, 1.0, 1.0, 0.4);
    else if (i%12 == 5)
      lut->SetTableValue (i, 1.0, 0.0, 1.0, 0.4);
    else if (i%12 == 6)
      lut->SetTableValue (i, 1.0, 0.5, 0.0, 0.4);
    else if (i%12 == 7)
      lut->SetTableValue (i, 0.0, 1.0, 0.5, 0.4);
    else if (i%12 == 8)
      lut->SetTableValue (i, 0.5, 0.0, 1.0, 0.4);
    else if (i%12 == 9)
      lut->SetTableValue (i, 1.0, 1.0, 0.5, 0.4);
    else if (i%12 == 10)
      lut->SetTableValue (i, 0.5, 1.0, 1.0, 0.4);
    else if (i%12 == 11)
      lut->SetTableValue (i, 1.0, 0.5, 1.0, 0.4);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildLegacyRainbowColorLookupTable()
{
  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetRampToLinear();
  lut->SetHueRange(0.6667, 0.0);
  lut->SetTableRange(0.0, 20.0);
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}
