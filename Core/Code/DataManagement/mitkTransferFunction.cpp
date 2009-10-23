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

#include "mitkTransferFunction.h"
#include "mitkImageToItk.h"
#include "mitkHistogramGenerator.h"

#include <itkRGBPixel.h>

#include <vector>

namespace mitk
{


TransferFunction::TransferFunction()
{
  m_ScalarOpacityFunction = vtkPiecewiseFunction::New();
  m_ColorTransferFunction = vtkColorTransferFunction::New();
  m_GradientOpacityFunction = vtkPiecewiseFunction::New();
  
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint(0,1);

  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint(0,1);

  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->SetColorSpaceToHSV();
  m_ColorTransferFunction->AddRGBPoint(0,1,1,1);
}

TransferFunction::~TransferFunction() 
{
  m_ScalarOpacityFunction->Delete();
  m_ColorTransferFunction->Delete();
  m_GradientOpacityFunction->Delete();
}

bool TransferFunction::operator==(Self& other)
{
  if ((m_Min != other.m_Min) || (m_Max != other.m_Max))
    return false;
  
  bool sizes = (m_ScalarOpacityFunction->GetSize() == other.m_ScalarOpacityFunction->GetSize())
            && (m_GradientOpacityFunction->GetSize() == other.m_GradientOpacityFunction->GetSize())
            && (m_ColorTransferFunction->GetSize() == other.m_ColorTransferFunction->GetSize());
  if (sizes == false)
    return false;

  for (int i = 0; i < m_ScalarOpacityFunction->GetSize(); i++ )
  {
    double myVal[4];
    double otherVal[4];
    m_ScalarOpacityFunction->GetNodeValue(i, myVal);
    other.m_ScalarOpacityFunction->GetNodeValue(i, otherVal);
    bool equal = (myVal[0] == otherVal[0])
              && (myVal[1] == otherVal[1])
              && (myVal[2] == otherVal[2])
              && (myVal[3] == otherVal[3]);
    if (equal == false)
      return false;
  }
  for (int i = 0; i < m_GradientOpacityFunction->GetSize(); i++ )
  {
    double myVal[4];
    double otherVal[4];
    m_GradientOpacityFunction->GetNodeValue(i, myVal);
    other.m_GradientOpacityFunction->GetNodeValue(i, otherVal);
    bool equal = (myVal[0] == otherVal[0])
      && (myVal[1] == otherVal[1])
      && (myVal[2] == otherVal[2])
      && (myVal[3] == otherVal[3]);
    if (equal == false)
      return false;
  }
  for (int i = 0; i < m_ColorTransferFunction->GetSize(); i++ )
  {
    double myVal[6];
    double otherVal[6];
    m_ColorTransferFunction->GetNodeValue(i, myVal);
    other.m_ColorTransferFunction->GetNodeValue(i, otherVal);
    bool equal = (myVal[0] == otherVal[0]) // X
      && (myVal[1] == otherVal[1])  // R
      && (myVal[2] == otherVal[2])  // G
      && (myVal[3] == otherVal[3])  // B
      && (myVal[4] == otherVal[4])  // midpoint
      && (myVal[5] == otherVal[5]); // sharpness
    if (equal == false)
      return false;
  }
  return true;
}


void TransferFunction::SetScalarOpacityPoints(TransferFunction::ControlPoints points)
{
  m_ScalarOpacityFunction->RemoveAllPoints();
  for(unsigned int i=0; i<=points.size()-1;i++)
  {
    this->AddScalarOpacityPoint(points[i].first, points[i].second);
  }
}


void TransferFunction::SetGradientOpacityPoints(TransferFunction::ControlPoints points)
{
  m_GradientOpacityFunction->RemoveAllPoints();
  for(unsigned int i=0; i<=points.size()-1;i++)
  {
    this->AddGradientOpacityPoint(points[i].first, points[i].second);
  }
}


void TransferFunction::SetRGBPoints(TransferFunction::RGBControlPoints rgbpoints)
{
  m_ColorTransferFunction->RemoveAllPoints();
  for(unsigned int i=0; i<=rgbpoints.size()-1;i++)
  {
    this->AddRGBPoint(rgbpoints[i].first, rgbpoints[i].second[0], 
      rgbpoints[i].second[1], rgbpoints[i].second[2]);
  }  
}

void TransferFunction::AddScalarOpacityPoint(double x, double value)
{ 
  m_ScalarOpacityFunction->AddPoint(x, value);
}
  

void TransferFunction::AddGradientOpacityPoint(double x, double value)
{ 
  m_GradientOpacityFunction->AddPoint(x, value);
}


void TransferFunction::AddRGBPoint(double x, double r, double g, double b)
{
  m_ColorTransferFunction->AddRGBPoint(x, r, g, b);
}


TransferFunction::ControlPoints &TransferFunction::GetScalarOpacityPoints()
{
  // Retrieve data points from VTK transfer function and store them in a vector
  m_ScalarOpacityPoints.clear();
  vtkFloatingPointType *data = m_ScalarOpacityFunction->GetDataPointer();
  for ( int i = 0; i < m_ScalarOpacityFunction->GetSize(); ++i )
  {
    m_ScalarOpacityPoints.push_back( std::make_pair( data[i*2], data[i*2+1] ));
  }

  return m_ScalarOpacityPoints;
}


TransferFunction::ControlPoints &TransferFunction::GetGradientOpacityPoints()
{
  // Retrieve data points from VTK transfer function and store them in a vector
  m_GradientOpacityPoints.clear();
  vtkFloatingPointType *data = m_GradientOpacityFunction->GetDataPointer();
  for ( int i = 0; i < m_GradientOpacityFunction->GetSize(); ++i )
  {
    m_GradientOpacityPoints.push_back( std::make_pair( data[i*2], data[i*2+1] ));
  }

  return m_GradientOpacityPoints;
}


TransferFunction::RGBControlPoints &TransferFunction::GetRGBPoints()
{
  // Retrieve data points from VTK transfer function and store them in a vector
  m_RGBPoints.clear();
  vtkFloatingPointType *data = m_ColorTransferFunction->GetDataPointer();
  for ( int i = 0; i < m_ColorTransferFunction->GetSize(); ++i )
  {
    double rgb[] = { data[i*4+1], data[i*4+2], data[i*4+3] };
    m_RGBPoints.push_back( std::make_pair( data[i*4], rgb ));
  }

  return m_RGBPoints;
}


int TransferFunction::RemoveScalarOpacityPoint(double x)
{
  return m_ScalarOpacityFunction->RemovePoint(x);
}


int TransferFunction::RemoveGradientOpacityPoint(double x)
{
  return m_GradientOpacityFunction->RemovePoint(x);
}


int TransferFunction::RemoveRGBPoint(double x)
{
  return m_ColorTransferFunction->RemovePoint(x);
}


void TransferFunction::ClearScalarOpacityPoints()
{
  m_ScalarOpacityFunction->RemoveAllPoints();
}
  

void TransferFunction::ClearGradientOpacityPoints()
{
  m_GradientOpacityFunction->RemoveAllPoints();
}


void TransferFunction::ClearRGBPoints()
{
  m_ColorTransferFunction->RemoveAllPoints();
}


void TransferFunction::InitializeByItkHistogram(
  const itk::Statistics::Histogram<double>* histogram)
{
  m_Histogram = histogram;
  m_Min = (int)GetHistogram()->GetBinMin(0,0);
  m_Max = (int)GetHistogram()->GetBinMax(0, GetHistogram()->Size()-1);
  
  /*
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint(m_Min,0.0);
  m_ScalarOpacityFunction->AddPoint(0.0,0.0);
  m_ScalarOpacityFunction->AddPoint(m_Max,1.0);
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint(m_Min,0.0);
  m_GradientOpacityFunction->AddPoint(0.0,1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.125),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.2),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.25),1.0);
  m_GradientOpacityFunction->AddPoint(m_Max,1.0);
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint(m_Min,1,0,0);
  m_ColorTransferFunction->AddRGBPoint(m_Max,1,1,0);  
  m_ColorTransferFunction->SetColorSpaceToHSV();
  LOG_INFO << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
  */
}

void TransferFunction::InitializeByMitkImage( const Image * image )
{
  HistogramGenerator::Pointer histGen= HistogramGenerator::New();
  histGen->SetImage(image);
  histGen->SetSize(256);
  histGen->ComputeHistogram();
  m_Histogram = histGen->GetHistogram();
  m_Min = (int)GetHistogram()->GetBinMin(0,0);
  m_Max = (int)GetHistogram()->GetBinMax(0, GetHistogram()->Size()-1);
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint(m_Min,0.0);
  m_ScalarOpacityFunction->AddPoint(0.0,0.0);
  m_ScalarOpacityFunction->AddPoint(m_Max,1.0);
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint(m_Min,0.0);
  m_GradientOpacityFunction->AddPoint(0.0,1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.125),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.2),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.25),1.0);
  m_GradientOpacityFunction->AddPoint(m_Max,1.0);
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint(m_Min,1,0,0);
  m_ColorTransferFunction->AddRGBPoint(m_Max,1,1,0);  
  m_ColorTransferFunction->SetColorSpaceToHSV();
  //LOG_INFO << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
}


void TransferFunction::InitializeHistogram( const Image * image )
{
  HistogramGenerator::Pointer histGen= HistogramGenerator::New();
  histGen->SetImage(image);
  histGen->SetSize(256);
  histGen->ComputeHistogram();
  m_Histogram = histGen->GetHistogram();
  m_Min = (int)GetHistogram()->GetBinMin(0,0);
  m_Max = (int)GetHistogram()->GetBinMax(0, GetHistogram()->Size()-1);
}

void TransferFunction::SetTransferFunctionMode( int mode )
{
  //Define Transfer Function
  enum TransferFunctionMode{
    TF_CT_DEFAULT, 
    TF_CT_BLACK_WHITE, 
    TF_CT_CARDIAC, 
    TF_CT_BONE, 
    TF_CT_BONE_GRADIENT, 
  };

  //remove all old points
  m_ScalarOpacityFunction->RemoveAllPoints();
  m_ColorTransferFunction->RemoveAllPoints();
  m_GradientOpacityFunction->RemoveAllPoints();

  switch( mode )
  {
  case ( TF_CT_DEFAULT ):
   
    // grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_ScalarOpacityFunction;
      f->AddPoint(135.063521,0.000000);
      f->AddPoint(180.498182,0.027847);
      f->AddPoint(948.137931,1.000000);
    }
    
    // gradient at grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_GradientOpacityFunction;
      f->AddPoint(560.695000,1.000000);
    }
    
    // grayvalue->color 
    {
      vtkColorTransferFunction *f=m_ColorTransferFunction;
      f->AddRGBPoint( -24.956443,0.650980,0.000000,0.000000);
      f->AddRGBPoint( 208.586207,0.972549,0.227451,0.160784);
      f->AddRGBPoint( 238.860254,0.980392,0.352941,0.145098);
      f->AddRGBPoint( 279.903212,1.000000,0.858824,0.149020);
      f->AddRGBPoint( 355.631579,1.000000,0.647059,0.152941);
      f->AddRGBPoint( 455.103448,1.000000,1.000000,1.000000);
      f->AddRGBPoint( 623.773140,1.000000,0.800000,0.396078);
      f->AddRGBPoint( 796.767695,1.000000,0.901961,0.815686);
      f->AddRGBPoint( 930.838475,1.000000,1.000000,1.000000);
      f->AddRGBPoint(1073.558984,1.000000,0.839216,0.423529);
      f->AddRGBPoint(1220.604356,1.000000,0.772549,0.490196);
    }
    
    break;

  case ( TF_CT_BLACK_WHITE ):
  
    //Set Opacity
    m_ScalarOpacityFunction->AddPoint( 135.063521, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 948.137931, 1.0 );
    

    //Set Color
    m_ColorTransferFunction->AddRGBPoint( 122.088929, 0.352941, 0.352941, 0.352941);
    m_ColorTransferFunction->AddRGBPoint( 372.931034, 1.000000, 1.000000, 1.000000 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 560.695000, 1);
   
    break;


  case ( TF_CT_BONE ):

    // grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_ScalarOpacityFunction;
      f->AddPoint(126.413793,0.000000);
      f->AddPoint(178.312160,0.014663);
      f->AddPoint(247.509982,0.000000);
      f->AddPoint(1013.010889,1.000000);
    }
    // gradient at grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_GradientOpacityFunction;
      f->AddPoint(485.377495,1.000000);
    }
    // grayvalue->color 
    {
      vtkColorTransferFunction *f=m_ColorTransferFunction;
      f->AddRGBPoint( 312.382940,1.000000,0.564706,0.274510);
      f->AddRGBPoint( 455.103448,1.000000,0.945098,0.768627);
      f->AddRGBPoint( 623.773140,1.000000,0.800000,0.333333);
      f->AddRGBPoint( 796.767695,1.000000,0.901961,0.815686);
      f->AddRGBPoint( 930.838475,1.000000,1.000000,1.000000);
      f->AddRGBPoint(1073.558984,1.000000,0.839216,0.423529);
      f->AddRGBPoint(1220.604356,1.000000,0.772549,0.490196);
    }
    break;

  case ( TF_CT_BONE_GRADIENT ):

    // grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_ScalarOpacityFunction;
      f->AddPoint(126.413793,0.000000);
      f->AddPoint(186.961887,0.146628);
      f->AddPoint(247.509982,0.000000);
      f->AddPoint(1013.010889,1.000000);
    }
    // gradient at grayvalue->opacity 
    {
      vtkPiecewiseFunction *f=m_GradientOpacityFunction;
      f->AddPoint(22.617060,0.000000);
      f->AddPoint(65.865699,1.000000);
    }
    // grayvalue->color 
    {
      vtkColorTransferFunction *f=m_ColorTransferFunction;
      f->AddRGBPoint( 312.382940,1.000000,0.564706,0.274510);
      f->AddRGBPoint( 455.103448,1.000000,0.945098,0.768627);
      f->AddRGBPoint( 623.773140,1.000000,0.800000,0.333333);
      f->AddRGBPoint( 796.767695,1.000000,0.901961,0.815686);
      f->AddRGBPoint( 930.838475,1.000000,1.000000,1.000000);
      f->AddRGBPoint(1073.558984,1.000000,0.839216,0.423529);
      f->AddRGBPoint(1220.604356,1.000000,0.772549,0.490196);
    }
    break;

  case ( TF_CT_CARDIAC ):

    //Set Opacity
    m_ScalarOpacityFunction->AddPoint( 150.246824, 0.000000 );
    m_ScalarOpacityFunction->AddPoint( 179.974592, 0.202346);
    m_ScalarOpacityFunction->AddPoint( 276.589837, 0.000000);
    m_ScalarOpacityFunction->AddPoint( 781.961887, 1.000000);

    //Set Color
    m_ColorTransferFunction->AddRGBPoint( 395.500907,1.000000,0.000000,0.000000);
    m_ColorTransferFunction->AddRGBPoint( 410.364791,1.000000,0.749020,0.000000);
    m_ColorTransferFunction->AddRGBPoint( 484.684211,1.000000,0.878431,0.662745);
    m_ColorTransferFunction->AddRGBPoint( 588.731397,1.000000,0.784314,0.482353);
     
    //Set Gradient
    m_GradientOpacityFunction->AddPoint( 246.862069, 0.215827 );

    break;

  default:

    break;
  }
  
  m_ScalarOpacityFunction->Modified();
  m_ColorTransferFunction->Modified();
  m_GradientOpacityFunction->Modified();
}
} // namespace
