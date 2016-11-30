#ifndef MITKMASKUTIL
#define MITKMASKUTIL

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <itkImage.h>

namespace mitk
{
/**
 * @brief Utility class for mask operations. It checks whether an image and a mask are compatible (spacing, orientation, etc...)
 * and it can also crop an image to the LargestPossibleRegion of the Mask
 */
template <class TPixel, unsigned int VImageDimension>
class MITKIMAGESTATISTICS_EXPORT MaskUtilities: public itk::Object
    {
    public:
        /** Standard Self typedef */
        typedef MaskUtilities                       Self;
        typedef itk::Object                         Superclass;
        typedef itk::SmartPointer< Self >           Pointer;
        typedef itk::SmartPointer< const Self >     ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self)

        /** Runtime information support. */
        itkTypeMacro(MaskUtilities, itk::Object)

        typedef itk::Image<TPixel, VImageDimension> ImageType;
        typedef itk::Image<unsigned short, VImageDimension> MaskType;

        /**
         * @brief Set image
         */
        void SetImage(ImageType* image);

        /**
         * @brief Set mask
         */
        void SetMask(MaskType* mask);

        /**
         * @brief Checks whether mask and image are compatible for joint access (as via iterators).
         * Spacing and direction must be the same between the two and they must be aligned. Also, the mask must be completely inside the image
         */
        bool CheckMaskSanity();

        /**
         * @brief Crops the image to the LargestPossibleRegion of the mask
         */
        typename itk::Image<TPixel, VImageDimension>::Pointer ExtractMaskImageRegion();

    protected:
        MaskUtilities(){}

        ~MaskUtilities(){}

    private:
        itk::Image<TPixel, VImageDimension>* m_Image;
        itk::Image<unsigned short, VImageDimension>* m_Mask;
    };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkMaskUtilities.cpp>
#endif

#endif
