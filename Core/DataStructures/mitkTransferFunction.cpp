/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <itkScalarImageToHistogramGenerator.h>

#include <vector>


mitk::TransferFunction::TransferFunction()
{
  m_ScalarOpacityFunction = vtkPiecewiseFunction::New();
  m_ColorTransferFunction = vtkColorTransferFunction::New();
  m_GradientOpacityFunction = vtkPiecewiseFunction::New();
  
  this->m_ScalarOpacityFunction->Initialize();
  this->m_GradientOpacityFunction->Initialize();
  
}

mitk::TransferFunction::~TransferFunction() {
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
      std::cerr<<"cannot access channel "<<channel<<std::endl;
    }
  }
}

mitk::TransferFunction::RGBControlPoints mitk::TransferFunction::GetRGBPoints()
{
  return m_RGBPoints;
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

void mitk::TransferFunction::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing
}

bool mitk::TransferFunction::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;    
}

bool mitk::TransferFunction::VerifyRequestedRegion()
{
  return true;
}

void mitk::TransferFunction::SetRequestedRegion(itk::DataObject*)
{
    //nothing
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
