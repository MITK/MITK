#include <mitkMaskGenerator.h>

namespace mitk
{
void MaskGenerator::SetTimeStep(unsigned int timeStep)
{
    if (m_TimeStep != timeStep)
    {
        m_TimeStep = timeStep;
        this->Modified();
    }
}

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

void MaskGenerator::SetInputImage(mitk::Image::Pointer inputImg)
{
    if (inputImg != m_inputImage)
    {
        m_inputImage = inputImg;
        this->Modified();
    }
}

mitk::Image::Pointer MaskGenerator::GetReferenceImage()
{
    return m_inputImage;
}
}
