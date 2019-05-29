#ifndef MITKMASKGENERATOR
#define MITKMASKGENERATOR

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <itkRegion.h>
#include <itkObject.h>
#include <itkSmartPointer.h>

namespace mitk
{
/**
* \class MaskGenerator
* \brief Base Class for all Mask Generators. Mask generators are classes that provide functionality for the
* creation of binary (or unsigned short) masks that can be applied to an image. See dervied classes for more
* information.
*/
class MITKIMAGESTATISTICS_EXPORT MaskGenerator: public itk::Object
{
public:
    /** Standard Self typedef */
    typedef MaskGenerator                       Self;
    typedef itk::Object                         Superclass;
    typedef itk::SmartPointer< Self >           Pointer;
    typedef itk::SmartPointer< const Self >     ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(MaskGenerator, itk::Object)

    //~MaskGenerator();

    /**
     * @brief GetMask must be overridden by derived classes.
     * @return mitk::Image::Pointer of generated mask
     */
    virtual mitk::Image::Pointer GetMask();

    /**
     * @brief GetReferenceImage per default returns the inputImage (as set by SetInputImage). If no input image is set it will return a nullptr.
     */
    virtual mitk::Image::ConstPointer GetReferenceImage();

    /**
     * @brief SetInputImage is used to set the input image to the mask generator. Some subclasses require an input image, others don't. See the documentation of the specific Mask Generator for more information.
     */
    void SetInputImage(mitk::Image::ConstPointer inputImg);

    virtual void SetTimeStep(unsigned int timeStep);

protected:
    MaskGenerator();

    unsigned int m_TimeStep;
    mitk::Image::Pointer m_InternalMask;
    mitk::Image::ConstPointer m_inputImage;

private:

};
}

#endif // MITKMASKGENERATOR

