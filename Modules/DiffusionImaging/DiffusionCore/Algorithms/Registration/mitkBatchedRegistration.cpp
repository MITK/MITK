#include "mitkBatchedRegistration.h"
#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkDiffusionImage.h"

mitk::BatchedRegistration::BatchedRegistration() :
  m_RegisteredImagesValid(false)
{
}

void mitk::BatchedRegistration::SetFixedImage(mitk::Image::Pointer& fixedImage)
{
  m_FixedImage = fixedImage;
}

void mitk::BatchedRegistration::SetMovingReferenceImage(Image::Pointer &movingImage)
{
  m_MovingReference = movingImage;
  m_RegisteredImagesValid = false;
}

void mitk::BatchedRegistration::SetBatch(std::vector<mitk::Image::Pointer> imageBatch)
{
  m_ImageBatch.clear();
  m_ImageBatch = imageBatch;
}

std::vector<mitk::Image::Pointer> mitk::BatchedRegistration::GetRegisteredImages()
{
  if (!m_RegisteredImagesValid)
  {
    m_RegisteredImages.clear();
    // First transform moving reference image
    TransformType transf = GetTransformation(m_FixedImage, m_MovingReference);
    // store it as first element in vector
    m_RegisteredImages.push_back(ApplyTransformationToImage(m_MovingReference,transf));
    // apply transformation to whole batch
    std::vector<mitk::Image::Pointer>::const_iterator itEnd = m_ImageBatch.end();
    for (std::vector<mitk::Image::Pointer>::iterator it = m_ImageBatch.begin(); it != itEnd; ++it)
    {
      m_RegisteredImages.push_back(ApplyTransformationToImage(*it,transf));
    }
  }
  return m_RegisteredImages;
}

mitk::Image::Pointer mitk::BatchedRegistration::ApplyTransformationToImage(mitk::Image::Pointer &img, const mitk::BatchedRegistration::TransformType &transformation) const
{
 // TODO: perform some magic!

  if (dynamic_cast<mitk::DiffusionImage<short>*> (img.GetPointer()) != NULL) // gaaaaahh template mist!
  {
   // apply transformation to image geometry as well as to all gradients !?
  }
  else
  {
    // do regular stuff
  }

  return NULL;
}

mitk::BatchedRegistration::TransformType mitk::BatchedRegistration::GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage)
{
  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );
  registrationMethod->SetTransformToRigid();
  registrationMethod->SetCrossModalityOn();
  registrationMethod->SetMovingImage(movingImage);
  registrationMethod->Update();
  // TODO fancy shit, where you query and create a transformation type object thingy
  TransformType transformation;

  return transformation;
}
