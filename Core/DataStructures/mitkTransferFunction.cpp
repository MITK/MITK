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
}
void mitk::TransferFunction::UpdateVtkFunctions() {
             
};

