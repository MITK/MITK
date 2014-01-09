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
#include <mitkColormapProperty.h>

#include "Spectrum.h"
#include "VRMusclesBones.h"
#include "VRRedVessels.h"
#include "VRBones.h"
#include "Stern.h"
#include "BlackBody.h"
#include "HotGreen.h"
#include "HotMetal.h"
#include "HotIron.h"
#include "GrayRainbow.h"
#include "Cardiac.h"
#include "PETColor.h"
#include "Flow.h"
#include "LONI.h"
#include "LONI2.h"
#include "Asymmetry.h"

mitk::LookupTable::LookupTable():
m_Window(0.0),
m_Level(0.0),
m_Opacity(1.0)
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

void mitk::LookupTable::SetActiveColormap(int index)
{
    switch(index)
    {
    case (mitk::ColormapProperty::CM_BW):
        this->BuildGrayScaleLookupTable();
        break;
    case (mitk::ColormapProperty::CM_BWINVERSE):
        this->BuildInverseGrayScaleLookupTable();
        break;
    case (mitk::ColormapProperty::CM_REDBLACKALPHA):
        this->BuildRedBlackAlphaLookupTable();
        break;
    case (mitk::ColormapProperty::CM_GREENBLACKALPHA):
        this->BuildGreenBlackAlphaLookupTable();
        break;
    case (mitk::ColormapProperty::CM_BLUEBLACKALPHA):
        this->BuildBlueBlackAlphaLookupTable();
        break;
    case (mitk::ColormapProperty::CM_VRMUSCLESBONES):
        this->BuildVRMusclesBonesLookupTable();
        break;
    case (mitk::ColormapProperty::CM_VRREDVESSELS):
        this->BuildVRRedVesselsLookupTable();
        break;
    case (mitk::ColormapProperty::CM_STERN):
        this->BuildSternLookupTable();
        break;
    case (mitk::ColormapProperty::CM_HOTGREEN):
        this->BuildHotGreenLookupTable();
        break;
    case (mitk::ColormapProperty::CM_VRBONES):
        this->BuildVRBonesLookupTable();
        break;
    case (mitk::ColormapProperty::CM_CARDIAC):
        this->BuildCardiacLookupTable();
        break;
    case (mitk::ColormapProperty::CM_GRAYRAINBOW):
        this->BuildGrayRainbowLookupTable();
        break;
    case (mitk::ColormapProperty::CM_RAINBOW):
        this->BuildRainbowLookupTable();
        break;
    case (mitk::ColormapProperty::CM_HOTMETAL):
        this->BuildHotMetalLookupTable();
        break;
    case (mitk::ColormapProperty::CM_HOTIRON):
        this->BuildHotIronLookupTable();
        break;
    case (mitk::ColormapProperty::CM_SPECTRUM):
        this->BuildSpectrumLookupTable();
        break;
    case (mitk::ColormapProperty::CM_PETCOLOR):
        this->BuildPETColorLookupTable();
        break;
    case (mitk::ColormapProperty::CM_FLOW):
        this->BuildFlowLookupTable();
        break;
    case (mitk::ColormapProperty::CM_LONI):
        this->BuildLONILookupTable();
        break;
    case (mitk::ColormapProperty::CM_LONI2):
        this->BuildLONI2LookupTable();
        break;
    case (mitk::ColormapProperty::CM_ASYMMETRY):
        this->BuildAsymmetryLookupTable();
        break;
    case (mitk::ColormapProperty::CM_PVALUE):
        this->BuildPValueLookupTable();
        break;
    case (mitk::ColormapProperty::CM_MULTILABEL):
        this->BuildMultiLabelLookupTable();
        break;
    case (mitk::ColormapProperty::CM_LEGACYBINARY):
        this->BuildLegacyBinaryLookupTable();
        break;
    default:
        MITK_ERROR << "Unexisting colormap";
    }
}

void mitk::LookupTable::SetVtkLookupTable( vtkLookupTable* lut )
{
  if ((!lut) || (m_LookupTable == lut))
  {
    return;
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
  this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-update is executed
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

void mitk::LookupTable::ChangeOpacity(int index, float opacity )
{

  int noValues = m_LookupTable->GetNumberOfTableValues ();
  if (index>noValues)
  {
    MITK_INFO << "could not change opacity. index exceed size of lut ... ";
    return;
  }

  vtkFloatingPointType rgba[ 4 ];

  m_LookupTable->GetTableValue ( index, rgba );
  rgba[ 3 ] = opacity;
  m_LookupTable->SetTableValue ( index, rgba );

  this->Modified();  // need to call modified, since LookupTableProperty seems to be unchanged so no widget-update is executed
}


vtkLookupTable* mitk::LookupTable::GetVtkLookupTable() const
{
  return m_LookupTable.GetPointer();
};

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{
  return m_LookupTable->GetPointer( 0 );
};

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
  //for (vtkIdType i=0; i < m_LookupTable->GetTable()->GetNumberOfTuples(); i++)
  //{
  //  if (m_LookupTable->GetTable()->GetTuple(i) != olut->GetTable()->GetTuple(i))
  //    return false;
  //}
  for (vtkIdType i=0; i < m_LookupTable->GetNumberOfTableValues(); i++)
  {
    //double v0_1 = m_LookupTable->GetTableValue(i)[0]; double v0_2 = olut->GetTableValue(i)[0];
    //double v1_1 = m_LookupTable->GetTableValue(i)[1]; double v1_2 = olut->GetTableValue(i)[1];
    //double v2_1 = m_LookupTable->GetTableValue(i)[2]; double v2_2 = olut->GetTableValue(i)[2];
    //double v3_1 = m_LookupTable->GetTableValue(i)[3]; double v3_2 = olut->GetTableValue(i)[3];
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

void mitk::LookupTable::CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction)
{
  if(colorFunction==NULL)
    colorFunction = vtkColorTransferFunction::New();

  mitk::LookupTable::RawLookupTableType *rgba = this->GetRawLookupTable();
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

  mitk::LookupTable::RawLookupTableType *rgba = this->GetRawLookupTable();
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

  mitk::LookupTable::RawLookupTableType *rgba = this->GetRawLookupTable();
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

void mitk::LookupTable::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  os << indent;
  m_LookupTable->PrintHeader(os, vtkIndent());
}

itk::LightObject::Pointer mitk::LookupTable::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}

void mitk::LookupTable::BuildGrayScaleLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
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
  vtkLookupTable* lut = vtkLookupTable::New();
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
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetTableRange (0, 1);
  lut->SetSaturationRange (0, 0);
  lut->SetHueRange (0, 0);
  lut->SetValueRange (1, 0);
  lut->SetAlphaRange (1, 0);
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildCardiacLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)Cardiac[i]/255.0, (double)Cardiac[256+i]/255.0, (double)Cardiac[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildHotGreenLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)HotGreen[i]/255.0, (double)HotGreen[256+i]/255.0, (double)HotGreen[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildRedBlackAlphaLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  lut->SetTableValue(0,0,0,0,0);

  for( int i=1; i<256; i++)
  {
    lut->SetTableValue(i, i/255.0, 0.0, 0.0, (i/255.0*68.0 + 187)/255.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildGreenBlackAlphaLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  lut->SetTableValue(0,0,0,0,0);

  for( int i=1; i<256; i++)
  {
    lut->SetTableValue(i, 0.0, i/255.0, 0.0, (i/255.0*68.0 + 187)/255.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildBlueBlackAlphaLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  lut->SetTableValue(0,0,0,0,0);

  for( int i=1; i<256; i++)
  {
    lut->SetTableValue(i, 0.0, 0.0, i/255.0, (i/255.0*68.0 + 187)/255.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildVRMusclesBonesLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)VRMusclesBones[i]/255.0, (double)VRMusclesBones[256+i]/255.0, (double)VRMusclesBones[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildSternLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)Stern[i]/255.0, (double)Stern[256+i]/255.0, (double)Stern[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildVRRedVesselsLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)VRRedVessels[i]/255.0, (double)VRRedVessels[256+i]/255.0, (double)VRRedVessels[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildGrayRainbowLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)GrayRainbow[i]/255.0, (double)GrayRainbow[256+i]/255.0, (double)GrayRainbow[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}


void mitk::LookupTable::BuildRainbowLookupTable()
{
  // add a default rainbow lookup table for color mapping
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->SetHueRange(0.6667, 0.0);
  lut->SetTableRange(0.0, 20.0);
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildSpectrumLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
      lut->SetTableValue(i, (double)Spectrum[i]/255.0, (double)Spectrum[256+i]/255.0, (double)Spectrum[256*2+i]/255.0, (double)(i)/255.0 );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildHotIronLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)HotIron[i][0]/255.0, (double)HotIron[i][1]/255.0, (double)HotIron[i][2]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildVRBonesLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)VRBones[i]/255.0, (double)VRBones[256+i]/255.0, (double)VRBones[256*2+i]/255.0, /*(double)(i)/255.0*/ log (1.0+(double)(i)/255.0*9.0)/log (10.0) );
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildHotMetalLookupTable()
{
//convert RGB to HSV
  double saturation = 0.0;

  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->SetSaturationRange (saturation, saturation);
  lut->SetAlphaRange (m_Opacity, m_Opacity);
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)HotMetal[i]/255.0, (double)HotMetal[256+i]/255.0, (double)HotMetal[256*2+i]/255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildPETColorLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
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

void mitk::LookupTable::BuildFlowLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<256; i++)
  {
      lut->SetTableValue(i, (double)Flow[i]/255.0, (double)Flow[256+i]/255.0, (double)Flow[256*2+i]/255.0, 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildLONILookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(203);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<203; i++)
  {
    lut->SetTableValue(i, (double)LONI[i][0], (double)LONI[i][1], (double)LONI[i][2], 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildLONI2LookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(120);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<120; i++)
  {
    lut->SetTableValue(i, (double)LONI2[i][0], (double)LONI2[i][1], (double)LONI2[i][2], 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildAsymmetryLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetNumberOfTableValues(256);
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
  lut->Build();

  for( int i=0; i<256; i++)
  {
    lut->SetTableValue(i, (double)AsymmetryLUT[i][0], (double)AsymmetryLUT[i][1], (double)AsymmetryLUT[i][2], 1.0);
  }

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildPValueLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
  lut->SetTableRange ( (m_Level - m_Window/2.0), (m_Level + m_Window/2.0) );
//  lut->SetTableRange (0, 1);
  lut->SetSaturationRange (1.0, 1.5);
  lut->SetHueRange (0.666, 0.8333);
  //lut->SetHueRange (0.666, 0.0);
  lut->SetValueRange (0.5, 1.0);
  lut->Build();

  m_LookupTable = lut;
  this->Modified();
}

void mitk::LookupTable::BuildMultiLabelLookupTable()
{
  vtkLookupTable* lut = vtkLookupTable::New();
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
