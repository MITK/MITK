#include "mitkTransferFunction.h"

void mitk::TransferFunction::FillValues(std::vector<mitk::TransferFunction::RGBO> &values, const mitk::TransferFunction::ElementSetType &elements) {
  for (unsigned int i=0;i<values.size();i++) {
    float ix = (float)i / values.size();
    values[i].m_Opacity =0.0f ;
    for (mitk::TransferFunction::ElementSetType::iterator elemIt = elements.begin(); elemIt != elements.end(); elemIt++ ) {
      //values[i].m_Opacity += (*elemIt)->GetValueAt(ix); 
      values[i] = values[i] + (*elemIt)->GetRGBOAt(ix); 
    } 
  }

  return;
  float maxValue = 0.0f;
  for (unsigned int i=0;i<values.size();i++) {
    maxValue = std::max(maxValue,values[i].m_Opacity);
  }
  // do not scale if not neccessary
  // maxValue  = std::min(1.0f,maxValue);

  for (unsigned int i=0;i<values.size();i++) {
    float red = 0, green = 0, blue = 0;
    for (mitk::TransferFunction::ElementSetType::iterator elemIt = elements.begin(); elemIt != elements.end(); elemIt++ ) {
      float ix = (float)i / values.size();
      if (values[i].m_Opacity>0) { 
        red += ((*elemIt) -> m_Red) * (*elemIt)->GetValueAt(ix) / values[i].m_Opacity ;   
        green += ((*elemIt) -> m_Green) * (*elemIt)->GetValueAt(ix) / values[i].m_Opacity ;   
        blue += ((*elemIt) -> m_Blue) * (*elemIt)->GetValueAt(ix) / values[i].m_Opacity ;   
      }
    }

  }
}


