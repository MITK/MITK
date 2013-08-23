#ifndef MITKBATCHEDREGISTRATION_H
#define MITKBATCHEDREGISTRATION_H

// ITK
#include <itkLightObject.h>
// MITK
#include <DiffusionCoreExports.h>
#include "mitkCommon.h"
#include "mitkImage.h"


namespace mitk
{
/**
 * @brief The BatchedRegistration class Wrapper to calculate and apply a reference transformation to several images.
 *
 *  Use if several pictures with the same world geometry are to be registered
 *  to one reference image, the registration is only computed once (for the moving image) and the geometry transformed for the complete
 *  image batch accordingly. Can handle image types that are usually not supported by registrations filters, e.g. fiber bundles and segmentations:
 *  these can be registered if a "registerable" image such as B0/T2 from which they are derived is supplied, since the transformation can be calculated
 *  on those and applied to the derived objects.
 */
class DiffusionCore_EXPORT BatchedRegistration : public itk::LightObject
{
public:

  typedef vnl_matrix<double> TransformType;

  BatchedRegistration();

  void SetFixedImage(Image::Pointer &fixedImage);
  void SetMovingReferenceImage(mitk::Image::Pointer& movingImage);

  void SetBatch(std::vector<mitk::Image::Pointer> imageBatch);

  /**
   * @brief GetRegisteredImages returns registered images ,
   *
   * at position 0 the registered moving reference image is supplied followed all registered images from the batch.
   */
  std::vector<mitk::Image::Pointer> GetRegisteredImages();

  mitk::Image::Pointer ApplyTransformationToImage(mitk::Image::Pointer& img, const TransformType& transformation) const;

  TransformType GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage);

protected:

private:
  BatchedRegistration(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented

  bool m_RegisteredImagesValid;


  mitk::Image::Pointer m_FixedImage;
  mitk::Image::Pointer m_MovingReference;

  /**
   * @brief m_ImageBatch List of images on which that the reference transformation is applied
   *
   */
  std::vector<mitk::Image::Pointer> m_ImageBatch;

  /**
   * @brief m_RegisteredImages List of references to registered images.
   *
   */
  std::vector<mitk::Image::Pointer> m_RegisteredImages;

};

}
#endif // MITKBATCHEDREGISTRATION_H
