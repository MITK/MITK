#include <mitkMaskGenerator.h>

namespace mitk
{

MaskGenerator::MaskGenerator():
    m_TimeStep(0)
{
    m_inputImage = nullptr;
}

mitk::Image::Pointer MaskGenerator::GetMask()
{
  return mitk::Image::New();
}


//typename itk::Region<3>::Pointer MaskGenerator::GetImageRegionOfMask(Image::Pointer image)
//{
//    if (m_InternalMask.IsNull() || m_Modified)
//    {
//        MITK_ERROR << "Update MaskGenerator first!";
//    }

//    mitk::BaseGeometry::Pointer imageGeometry = image->GetGeometry();
//    mitk::BaseGeometry::Pointer maskGeometry = m_InternalMask->GetGeometry();


//}

void MaskGenerator::SetTimeStep(unsigned int timeStep)
{
    if (timeStep != m_TimeStep)
    {
        m_TimeStep = timeStep;
    }
}

void MaskGenerator::SetInputImage(mitk::Image::ConstPointer inputImg)
{
    if (inputImg != m_inputImage)
    {
        m_inputImage = inputImg;
        this->Modified();
    }
}

mitk::Image::ConstPointer MaskGenerator::GetReferenceImage()
{
    return m_inputImage;
}
}
