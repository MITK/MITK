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
  m_ScalarOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  m_ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  m_GradientOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  
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
  MITK_INFO << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
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
  //MITK_INFO << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
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

void TransferFunction::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  os << indent << "ScalarOpacity: ";
  m_ScalarOpacityFunction->PrintHeader(os, vtkIndent());
  os << indent << "GradientOpacity: ";
  m_GradientOpacityFunction->PrintHeader(os, vtkIndent());
  os << indent << "ColorTransfer: ";
  m_ColorTransferFunction->PrintHeader(os, vtkIndent());
  os << indent << "Min: " << m_Min << ", Max: " << m_Max << std::endl;
}

}// namespace
