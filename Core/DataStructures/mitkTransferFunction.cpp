#include <mitkImageToItk.h>

#include "mitkTransferFunction.h"
#include <itkScalarImageToHistogramGenerator.h>

#include <mitkHistogramGenerator.h>

void mitk::TransferFunction::FillValues(std::vector<mitk::TransferFunction::RGBO> &values, const mitk::TransferFunction::ElementSetType &elements) {
  for (unsigned int i=0;i<values.size();i++) {
    float ix = (float)i / values.size();
    values[i].m_Opacity =0.0f ;
    for (mitk::TransferFunction::ElementSetType::const_iterator elemIt = elements.begin(); elemIt != elements.end(); elemIt++ ) {
      //values[i].m_Opacity += (*elemIt)->GetValueAt(ix); 
      values[i] = values[i] + (*elemIt)->GetRGBOAt(ix); 
    } 
  }
  return;
}
void mitk::TransferFunction::UpdateVtkFunctions() {
    std::vector<mitk::TransferFunction::RGBO> values(100);
    FillValues(values,m_Elements);
    if (m_ScalarOpacityFunction->GetSize() > 0) {
      m_ScalarOpacityFunction->RemoveAllPoints();
      m_ColorTransferFunction->RemoveAllPoints();
    }  
 //   std::cout << "Fill TF: ";
    for (unsigned int x = 0; x < values.size() ; x++) {
      float fx = m_Min + x * (float)(m_Max - m_Min)/values.size(); 
  //    std::cout << fx << "/" << values[x].m_Opacity << " ";
      m_ScalarOpacityFunction->AddPoint(fx, values[x].m_Opacity);
      m_ColorTransferFunction->AddRGBPoint(fx, values[x].m_Red / 255.0, values[x].m_Green / 255.0, values[x].m_Blue / 255.0); 
    }
 //   std::cout << std::endl;
   if (m_Elements.size() > 1) {
     m_Valid = true;
     std::cout << "TF is now valid" << std::endl;
   } 
};

void mitk::TransferFunction::InitializeByMitkImage( const mitk::Image * image )
{
  mitk::HistogramGenerator::Pointer histGen= mitk::HistogramGenerator::New();
  histGen->SetImage(image);
  histGen->SetSize(100);
  histGen->ComputeHistogram();
  m_Histogram = histGen->GetHistogram();
  m_Min = GetHistogram()->GetBinMin(0,0);
  m_Max = GetHistogram()->GetBinMin(0, GetHistogram()->Size()-1);
}
