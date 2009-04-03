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


mitk::TransferFunction::TransferFunction()
{
  m_ScalarOpacityFunction = vtkPiecewiseFunction::New();
  m_ColorTransferFunction = vtkColorTransferFunction::New();
  m_GradientOpacityFunction = vtkPiecewiseFunction::New();
  
  this->m_ScalarOpacityFunction->Initialize();
  this->m_GradientOpacityFunction->Initialize();
  
}

mitk::TransferFunction::~TransferFunction() 
{
  m_ScalarOpacityFunction->Delete();
  m_ColorTransferFunction->Delete();
  m_GradientOpacityFunction->Delete();
}


void mitk::TransferFunction::SetPoints(int channel, mitk::TransferFunction::ControlPoints points)
{
  switch ( channel )
  {
    case 0: //scalar opacity
    { 
      m_ScalarOpacityFunction->RemoveAllPoints();
      m_ScalarOpacityPoints.clear();
      for(unsigned int i=0; i<=points.size()-1;i++)
      {
        this->AddPoint(channel, points[i].first, points[i].second);
      }
      break;
    }
    case 1: //gradient opacity
    { 
      m_GradientOpacityFunction->RemoveAllPoints();
      m_GradientOpacityPoints.clear();
      for(unsigned int i=0; i<=points.size()-1;i++)
      {
        this->AddPoint(channel, points[i].first, points[i].second);
      }
      break;
    }
    default:
    {
      std::cerr<<"cannot access channel "<<channel<<std::endl;
    }
  }
}

void mitk::TransferFunction::SetRGBPoints(mitk::TransferFunction::RGBControlPoints rgbpoints)
{
  m_ColorTransferFunction->RemoveAllPoints();
  m_RGBPoints.clear();
  for(unsigned int i=0; i<=rgbpoints.size()-1;i++)
  {
    this->AddRGBPoint(rgbpoints[i].first, rgbpoints[i].second[0], rgbpoints[i].second[1], rgbpoints[i].second[2]);
  }  
}

void mitk::TransferFunction::AddPoint(int channel, double x, double value)
{ 
  //std::cout<<"mitk::TransferFunction::AddPoint( "<<channel<<", "<<x<<", "<<value<<")"<<std::endl;
  switch ( channel )
  {
    case 0: //scalar opacity
    { 
      m_ScalarOpacityPoints.push_back(std::make_pair(x, value));
      m_ScalarOpacityFunction->AddPoint(x, value);
      break;
    }
    
    case 1: //gradient opacity
    { 
      m_GradientOpacityPoints.push_back(std::make_pair(x, value));
      m_GradientOpacityFunction->AddPoint(x, value);
      break;
    }
    
    default:
    {
      std::cerr<<"cannot access channel "<<channel<<std::endl;
    }
  }
}
  

void mitk::TransferFunction::AddRGBPoint(double x, double r, double g, double b)
{
  //std::cout<<"mitk::TransferFunction::AddRGBPoint( "<<x<<", "<<r<<", "<<g<<", "<<b<<")"<<std::endl;
  double rgb[] = {r,g,b};
  m_RGBPoints.push_back(std::make_pair(x, rgb));
  m_ColorTransferFunction->AddRGBPoint(x, r, g, b);
}

mitk::TransferFunction::ControlPoints mitk::TransferFunction::GetPoints(int channel)
{
  switch ( channel )
  {
    case 0: //scalar opacity
    {
      return m_ScalarOpacityPoints;
    }
    
    case 1: //gradient opacity
    { 
      return m_GradientOpacityPoints;
    }
    
    default:
    {
      std::cout<<"channel not found, returning scalar opacity control points!";
      return m_ScalarOpacityPoints;
    }
  }
}

mitk::TransferFunction::RGBControlPoints mitk::TransferFunction::GetRGBPoints()
{
  return m_RGBPoints;
}

void mitk::TransferFunction::RemovePoint(int channel, double x)
{
  switch ( channel )
  {
    case 0: //scalar opacity
    {
      for(unsigned int i = 0; i < m_ScalarOpacityPoints.size(); i++)
      {
        if(m_ScalarOpacityPoints[i].first == x)
        {
          m_ScalarOpacityPoints.erase(m_ScalarOpacityPoints.begin()+i);
          m_ScalarOpacityFunction->RemovePoint(x);
          break;
        }
      }
      break;
    }
    
    case 1: //gradient opacity
    { 
      for(unsigned int i = 0; i < m_GradientOpacityPoints.size(); i++)
      {
        if(m_GradientOpacityPoints[i].first == x)
        {
          m_GradientOpacityPoints.erase(m_GradientOpacityPoints.begin()+i);
          m_GradientOpacityFunction->RemovePoint(x);
          break;
        }
      }
      break;
    }
    
    default:
    {
      std::cerr<<"cannot access channel "<<channel<<std::endl;
    }
  }
}

void mitk::TransferFunction::RemoveRGBPoint(double x)
{
  for(unsigned int i = 0; i < m_RGBPoints.size(); i++)
  {
    if(m_RGBPoints[i].first == x)
    {
      m_RGBPoints.erase(m_RGBPoints.begin()+i);
      m_ColorTransferFunction->RemovePoint(x);
      break;
    }
  }
}

void mitk::TransferFunction::ClearPoints(int channel)
{
  switch ( channel )
  {
    case 0: //scalar opacity
    {
      m_ScalarOpacityPoints.clear();
      m_ScalarOpacityFunction->RemoveAllPoints();
      break;
    }
    
    case 1: //gradient opacity
    { 
      m_GradientOpacityPoints.clear();
      m_GradientOpacityFunction->RemoveAllPoints();
      break;
    }
    
    default:
    {
      std::cerr<<"cannot access channel "<<channel<<std::endl;
    }
  }
}
  
void mitk::TransferFunction::ClearRGBPoints()
{
  m_RGBPoints.clear();
  m_ColorTransferFunction->RemoveAllPoints();
}

void mitk::TransferFunction::InitializeByItkHistogram(const itk::Statistics::Histogram<double>* histogram) {
  m_Histogram = histogram;
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
  std::cout << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
}

void mitk::TransferFunction::InitializeByMitkImage( const mitk::Image * image )
{
  mitk::HistogramGenerator::Pointer histGen= mitk::HistogramGenerator::New();
  histGen->SetImage(image);
  histGen->SetSize(100);
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
  //std::cout << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
}


void mitk::TransferFunction::InitializeHistogram( const mitk::Image * image )
{
  mitk::HistogramGenerator::Pointer histGen= mitk::HistogramGenerator::New();
  histGen->SetImage(image);
  histGen->SetSize(100);
  histGen->ComputeHistogram();
  m_Histogram = histGen->GetHistogram();
  m_Min = (int)GetHistogram()->GetBinMin(0,0);
  m_Max = (int)GetHistogram()->GetBinMax(0, GetHistogram()->Size()-1);
}

void mitk::TransferFunction::SetTransferFunctionMode( int mode )
{
  //Define Transfer Function
  typedef enum TransferFunctionMode{TF_SKIN_50, TF_SKIN_75, TF_CT_AAA, TF_CT_Bone, TF_CT_CARDIAC, TF_CT_CA, TF_MR_Default, TF_MR_MIP, TF_MITK_DEFAULT};

  //remove all old points
  m_ScalarOpacityFunction->RemoveAllPoints();
  m_ColorTransferFunction->RemoveAllPoints();
  m_GradientOpacityFunction->RemoveAllPoints();

#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION >= 2)  )

  //VTK Version >= 5.2

  switch( mode )
  {
  case ( TF_SKIN_50 ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( 0.055176, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 27.879, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 87.593, 1.0);
    m_ScalarOpacityFunction->AddPoint( 551.76, 1.0 );

    //Set Color
    m_ColorTransferFunction->AddRGBPoint( 0.055176, 0.75, 0.60, 0.35, 0.25, 0 );
    m_ColorTransferFunction->AddRGBPoint( 551.76, 0.75, 0.60, 0.35 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);    

    break;

  case ( TF_SKIN_75 ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( 0.055176, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 87.593, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 198.84, 1.0);
    m_ScalarOpacityFunction->AddPoint( 551.76, 1.0 );

    //Set Color
    m_ColorTransferFunction->AddRGBPoint( 0.055176, 0.75, 0.60, 0.35, 0.25, 0 );
    m_ColorTransferFunction->AddRGBPoint( 551.76, 0.75, 0.60, 0.35 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_CT_AAA ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( -3024, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 143.556, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 166.222, 0.686275, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 214.389, 0.696078, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 419.736, 0.833333, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 3071, 0.803922, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 214.389, 1, 1, 1, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 419.736, 1, 0.937033, 0.954531, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 3071, 0.827451, 0.658824, 1, 0.5, 0 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_CT_Bone ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( -3024, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( -16.4458, 0, 0.492958, 0.61 );
    m_ScalarOpacityFunction->AddPoint( 641.385, 0.715686, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 3071, 0.705882, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( -16.4458, 0.729412, 0.254902, 0.301961, 0.492958, 0.61 );
    m_ColorTransferFunction->AddRGBPoint( 641.385, 0.905882, 0.815686, 0.552941, 0.5, 0);
    m_ColorTransferFunction->AddRGBPoint( 3071, 1, 1, 1, 0.5, 0 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_CT_CARDIAC ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( -3024, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( -77.6875, 0, 0.846154, 0.92 );
    m_ScalarOpacityFunction->AddPoint( 94.9518, 0.285714, 0.33, 0.45 );
    m_ScalarOpacityFunction->AddPoint( 179.052, 0.553571, 0.5, 0.39 );
    m_ScalarOpacityFunction->AddPoint( 260.439, 0.848214, 0.5, 0.0 );
    m_ScalarOpacityFunction->AddPoint( 3071, 0.875, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( -77.6875, 0.54902, 0.25098, 0.14902, 0.846154, 0.92 );
    m_ColorTransferFunction->AddRGBPoint( 94.9518, 0.882353, 0.603922, 0.290196, 0.33, 0.45 );
    m_ColorTransferFunction->AddRGBPoint( 179.052, 1, 0.937033, 0.954531, 0.5, 0.39 );
    m_ColorTransferFunction->AddRGBPoint( 260.439, 0.615686, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 3071, 0.827451, 0.658824, 1, 0.5, 0 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_CT_CA ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( -2048, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 142.677, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 145.016, 0.116071, 0.5, 0.26 );
    m_ScalarOpacityFunction->AddPoint( 192.174, 0.5625, 0.469638, 0.39 );
    m_ScalarOpacityFunction->AddPoint( 217.24, 0.776786, 0.666667, 0.41 );
    m_ScalarOpacityFunction->AddPoint( 384.347, 0.830357, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 3661, 0.830357, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint( -2048, 0, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 142.677, 0, 0, 0, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 145.016, 0.615686, 0, 0.0156863, 0.5, 0.26 );
    m_ColorTransferFunction->AddRGBPoint( 192.174, 0.909804, 0.454902, 0, 0.469638, 0.39 );
    m_ColorTransferFunction->AddRGBPoint( 217.24, 0.972549, 0.807843, 0.611765, 0.666667, 0.41 );
    m_ColorTransferFunction->AddRGBPoint( 384.347, 0.909804, 0.909804, 1, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 3661, 1, 1, 1, 0.5, 0 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_MR_Default ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( 0, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 20, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 40, 0.15, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 120, 0.3, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 220, 0.375, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 1024, 0.5, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint( 0, 0, 0, 0, 0.5, 0  );
    m_ColorTransferFunction->AddRGBPoint( 20, 0.168627, 0, 0, 0.5, 0   );
    m_ColorTransferFunction->AddRGBPoint( 40, 0.403922, 0.145098, 0.0784314, 0.5, 0   );
    m_ColorTransferFunction->AddRGBPoint( 120, 0.780392, 0.607843, 0.380392, 0.5, 0   );
    m_ColorTransferFunction->AddRGBPoint( 220, 0.847059, 0.835294, 0.788235, 0.5, 0   );
    m_ColorTransferFunction->AddRGBPoint( 1024, 1, 1, 1, 0.5, 0   );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_MR_MIP ):

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint( 0, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 98.3725, 0, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 416.637, 1, 0.5, 0 );
    m_ScalarOpacityFunction->AddPoint( 2800, 0, 0.5, 0 );

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint( 0, 1, 1, 1, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 98.3725, 1, 1, 1, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 416.637, 1, 1, 1, 0.5, 0 );
    m_ColorTransferFunction->AddRGBPoint( 2800, 1, 1, 1, 0.5, 0 );

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint( 0, 1, 0.5, 0 );
    m_GradientOpacityFunction->AddPoint( 255, 1, 0.5, 0);

    break;

  case ( TF_MITK_DEFAULT ): //Old MITK Transfer Function Style

    //Set Opacity
    m_ScalarOpacityFunction->Initialize();
    m_ScalarOpacityFunction->AddPoint(m_Min,0.0);
    m_ScalarOpacityFunction->AddPoint(0.0,0.0);
    m_ScalarOpacityFunction->AddPoint(m_Max,1.0);

    //Set Color
    m_ColorTransferFunction->RemoveAllPoints();
    m_ColorTransferFunction->AddRGBPoint(m_Min,1,0,0);
    m_ColorTransferFunction->AddRGBPoint(m_Max,1,1,0);  
    m_ColorTransferFunction->SetColorSpaceToHSV();

    //Set Gradient
    m_GradientOpacityFunction->Initialize();
    m_GradientOpacityFunction->AddPoint(m_Min,0.0);
    m_GradientOpacityFunction->AddPoint(0.0,1.0);
    m_GradientOpacityFunction->AddPoint((m_Max*0.125),1.0);
    m_GradientOpacityFunction->AddPoint((m_Max*0.2),1.0);
    m_GradientOpacityFunction->AddPoint((m_Max*0.25),1.0);
    m_GradientOpacityFunction->AddPoint(m_Max,1.0);

    break;

  default:

    break;
  }

#else

//VTK Version < 5.2

switch( mode )
{
case ( TF_SKIN_50 ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( 0.055176, 0.0 );
  m_ScalarOpacityFunction->AddPoint( 27.879, 0.0 );
  m_ScalarOpacityFunction->AddPoint( 87.593, 1.0);
  m_ScalarOpacityFunction->AddPoint( 551.76, 1.0 );

  //Set Color
  m_ColorTransferFunction->AddRGBPoint( 0.055176, 0.75, 0.60, 0.35 );
  m_ColorTransferFunction->AddRGBPoint( 551.76, 0.75, 0.60, 0.35 );


  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );  

  break;

case ( TF_SKIN_75 ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( 0.055176, 0.0 );
  m_ScalarOpacityFunction->AddPoint( 87.593, 0.0 );
  m_ScalarOpacityFunction->AddPoint( 198.84, 1.0);
  m_ScalarOpacityFunction->AddPoint( 551.76, 1.0 );

  //Set Color
  m_ColorTransferFunction->AddRGBPoint( 0.055176, 0.75, 0.60, 0.35 );
  m_ColorTransferFunction->AddRGBPoint( 551.76, 0.75, 0.60, 0.35 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_CT_AAA ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( -3024, 0 );
  m_ScalarOpacityFunction->AddPoint( 143.556, 0 );
  m_ScalarOpacityFunction->AddPoint( 166.222, 0.686275 );
  m_ScalarOpacityFunction->AddPoint( 214.389, 0.696078 );
  m_ScalarOpacityFunction->AddPoint( 419.736, 0.833333 );
  m_ScalarOpacityFunction->AddPoint( 3071, 0.803922 );

  //Set Color
  m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( 143.556, 0.615686, 0.356863, 0.184314 );
  m_ColorTransferFunction->AddRGBPoint( 166.222, 0.882353, 0.603922, 0.290196 );
  m_ColorTransferFunction->AddRGBPoint( 214.389, 1, 1, 1 );
  m_ColorTransferFunction->AddRGBPoint( 419.736, 1, 0.937033, 0.954531 );
  m_ColorTransferFunction->AddRGBPoint( 3071, 0.827451, 0.658824, 1 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_CT_Bone ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( -3024, 0 );
  m_ScalarOpacityFunction->AddPoint( -16.4458, 0 );
  m_ScalarOpacityFunction->AddPoint( 641.385, 0.715686 );
  m_ScalarOpacityFunction->AddPoint( 3071, 0.705882 );

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( -16.4458, 0.729412, 0.254902, 0.301961 );
  m_ColorTransferFunction->AddRGBPoint( 641.385, 0.905882, 0.815686, 0.552941 );
  m_ColorTransferFunction->AddRGBPoint( 3071, 1, 1, 1 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );
  break;

case ( TF_CT_CARDIAC ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( -3024, 0);
  m_ScalarOpacityFunction->AddPoint( -77.6875, 0 );
  m_ScalarOpacityFunction->AddPoint( 94.9518, 0.285714 );
  m_ScalarOpacityFunction->AddPoint( 179.052, 0.553571 );
  m_ScalarOpacityFunction->AddPoint( 260.439, 0.848214 );
  m_ScalarOpacityFunction->AddPoint( 3071, 0.875 );

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint( -3024, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( -77.6875, 0.54902, 0.25098, 0.14902 );
  m_ColorTransferFunction->AddRGBPoint( 94.9518, 0.882353, 0.603922, 0.290196 );
  m_ColorTransferFunction->AddRGBPoint( 179.052, 1, 0.937033, 0.954531 );
  m_ColorTransferFunction->AddRGBPoint( 260.439, 0.615686, 0, 0);
  m_ColorTransferFunction->AddRGBPoint( 3071, 0.827451, 0.658824, 1 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_CT_CA ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( -2048, 0);
  m_ScalarOpacityFunction->AddPoint( 142.677, 0 );
  m_ScalarOpacityFunction->AddPoint( 145.016, 0.116071 );
  m_ScalarOpacityFunction->AddPoint( 192.174, 0.5625 );
  m_ScalarOpacityFunction->AddPoint( 217.24, 0.776786 );
  m_ScalarOpacityFunction->AddPoint( 384.347, 0.830357 );
  m_ScalarOpacityFunction->AddPoint( 3661, 0.830357 );

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint( -2048, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( 142.677, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( 145.016, 0.615686, 0, 0.0156863 );
  m_ColorTransferFunction->AddRGBPoint( 192.174, 0.909804, 0.454902, 0 );
  m_ColorTransferFunction->AddRGBPoint( 217.24, 0.972549, 0.807843, 0.611765 );
  m_ColorTransferFunction->AddRGBPoint( 384.347, 0.909804, 0.909804, 1 );
  m_ColorTransferFunction->AddRGBPoint( 3661, 1, 1, 1 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_MR_Default ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( 0, 0 );
  m_ScalarOpacityFunction->AddPoint( 20, 0 );
  m_ScalarOpacityFunction->AddPoint( 40, 0.15 );
  m_ScalarOpacityFunction->AddPoint( 120, 0.3 );
  m_ScalarOpacityFunction->AddPoint( 220, 0.375 );
  m_ScalarOpacityFunction->AddPoint( 1024, 0.5 );

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint( 0, 0, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( 20, 0.168627, 0, 0 );
  m_ColorTransferFunction->AddRGBPoint( 40, 0.403922, 0.145098, 0.0784314 );
  m_ColorTransferFunction->AddRGBPoint( 120, 0.780392, 0.607843, 0.380392 );
  m_ColorTransferFunction->AddRGBPoint( 220, 0.847059, 0.835294, 0.788235 );
  m_ColorTransferFunction->AddRGBPoint( 1024, 1, 1, 1  );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_MR_MIP ):

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint( 0, 0 );
  m_ScalarOpacityFunction->AddPoint( 98.3725, 0 );
  m_ScalarOpacityFunction->AddPoint( 416.637, 1 );
  m_ScalarOpacityFunction->AddPoint( 2800, 0 );

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint( 0, 1, 1, 1 );
  m_ColorTransferFunction->AddRGBPoint( 98.3725, 1, 1, 1 );
  m_ColorTransferFunction->AddRGBPoint( 416.637, 1, 1, 1 );
  m_ColorTransferFunction->AddRGBPoint( 2800, 1, 1, 1 );

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint( 0, 1 );
  m_GradientOpacityFunction->AddPoint( 255, 1 );

  break;

case ( TF_MITK_DEFAULT ): //Old MITK Transfer Function Style

  //Set Opacity
  m_ScalarOpacityFunction->Initialize();
  m_ScalarOpacityFunction->AddPoint(m_Min,0.0);
  m_ScalarOpacityFunction->AddPoint(0.0,0.0);
  m_ScalarOpacityFunction->AddPoint(m_Max,1.0);

  //Set Color
  m_ColorTransferFunction->RemoveAllPoints();
  m_ColorTransferFunction->AddRGBPoint(m_Min,1,0,0);
  m_ColorTransferFunction->AddRGBPoint(m_Max,1,1,0);  
  m_ColorTransferFunction->SetColorSpaceToHSV();

  //Set Gradient
  m_GradientOpacityFunction->Initialize();
  m_GradientOpacityFunction->AddPoint(m_Min,0.0);
  m_GradientOpacityFunction->AddPoint(0.0,1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.125),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.2),1.0);
  m_GradientOpacityFunction->AddPoint((m_Max*0.25),1.0);
  m_GradientOpacityFunction->AddPoint(m_Max,1.0);

  break;

default:

  break;
}


#endif

}