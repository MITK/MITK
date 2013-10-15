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
 */
class DiffusionCore_EXPORT RegistrationWrapper : public itk::LightObject
{
public:

  typedef double* RidgidTransformType;

  mitkClassMacro(BatchedRegistration, itk::LightObject)
  itkNewMacro(Self)

  void ApplyTransformationToImage(mitk::Image::Pointer& img, const RidgidTransformType& transformation, double *offset, mitk::Image::Pointer resampleReference = NULL , bool binary = false) const;

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
