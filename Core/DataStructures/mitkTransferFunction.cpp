#include "mitkTransferFunction.h"

#include <mitkImageToItk.h>

#include <itkScalarImageToHistogramGenerator.h>

#include <mitkHistogramGenerator.h>


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
  std::cout << "min/max in tf-c'tor:" << m_Min << "/" << m_Max << std::endl;
}

mitk::TransferFunction::Pointer mitk::TransferFunction::CreateForMitkImage(const mitk::Image* image) {
  mitk::TransferFunction::Pointer newTF = mitk::TransferFunction::New();
  newTF->InitializeByMitkImage(image);
  return newTF;
}
