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
 * @brief The RegistrationWrapper class wraps the pyramid registration to calculate and apply a reference transformation to several images.
 *
 *  Use if several pictures with the same world geometry are to be registered
 *  to one reference image, the registration is only computed once (for the moving image) and the geometry can be transformed for the complete
 *  image batch accordingly. Can handle image types that are usually not supported by registrations filters, e.g. fiber bundles and segmentations:
 *  these can be registered if a "registerable" image such as B0/T2 from which they are derived is supplied, since the transformation can be calculated
 *  on those and applied to the derived objects.
 *
 *  For DWI images a registerable B0 Image will automatically be extracted.
 *
 */
class DiffusionCore_EXPORT RegistrationWrapper : public itk::LightObject
{
public:

  typedef double* RidgidTransformType;

  mitkClassMacro(RegistrationWrapper, itk::LightObject)
  itkNewMacro(Self)

  /**
   * @brief ApplyTransformationToImage Applies transformation from GetTransformation to provided image.
   *
   *  Transforms image according to previously calculated transformation. Can be applied to derived resources also (e.g. binary images).
   *  Handles DWI Data and rotates the gradients according to the transformation.
   *  Images are resampled to provided reference, if no reference it supplied a copy of the input image is used as reference.
   *
   * @param img - image on which the transformation is to be applied
   * @param transformation - transformation returned from  GetTransformation
   * @param offset - offset  transformation returned from  GetTransformation
   * @param resampleReference - image to which is to be resampled
   * @param binary
   */
  void ApplyTransformationToImage(mitk::Image::Pointer& img, const RidgidTransformType& transformation, double *offset, mitk::Image::Pointer resampleReference = NULL , bool binary = false) const;

  /**
   * @brief GetTransformation Registeres the moving to the fixed image and returns the according transformation
   *
   * \note Does not return a registered image \see  ApplyTransformationToImage for this.
   *
   * Both images are set to the same origin (the one of the fixed image), this is supposed to ensure overlapping,
   * the correction of the moving image is returned in the offset.
   *
   * It is possible mask a certain area and thereby excluding it from the registration metric (e.g. a tumor that is operated on),
   * this can be set as mitk::Image where all non-zero voxels are excluded.
   *
   * @param fixedImage
   * @param movingImage
   * @param transformation
   * @param offset - stores offset that has been applied to match origin of both images
   * @param mask - optional, provide a mask that is excluded from registration metric
   */
  void GetTransformation(mitk::Image::Pointer fixedImage , mitk::Image::Pointer movingImage, RidgidTransformType transformation, double* offset, mitk::Image::Pointer mask = NULL);


protected:
  RegistrationWrapper();
  ~RegistrationWrapper(){};

private:
  RegistrationWrapper(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented
};

}
#endif // MITKBATCHEDREGISTRATION_H
