#include "mitkBatchedRegistration.h"
#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkDiffusionImage.h"

#include <mitkRotationOperation.h>
#include <itkScalableAffineTransform.h>

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
    ApplyTransformationToImage(m_MovingReference,transf);
    m_RegisteredImages.push_back(m_MovingReference);
    // apply transformation to whole batch
    std::vector<mitk::Image::Pointer>::const_iterator itEnd = m_ImageBatch.end();
    for (std::vector<mitk::Image::Pointer>::iterator it = m_ImageBatch.begin(); it != itEnd; ++it)
    {
      ApplyTransformationToImage(*it,transf);
      m_RegisteredImages.push_back(*it);
    }
  }
  return m_RegisteredImages;
}

void mitk::BatchedRegistration::ApplyTransformationToImage(mitk::Image::Pointer &img, const mitk::BatchedRegistration::TransformType &transformation) const
{
 // TODO: perform some magic!
  mitk::Vector3D translateVector;
  translateVector[0] = transformation.get(0,3);
  translateVector[1] = transformation.get(1,3);
  translateVector[2] = transformation.get(2,3);
  img->GetGeometry()->Translate(translateVector);

  itk::ScalableAffineTransform<mitk::ScalarType,3>::Pointer rotationTransform;
  itk::Matrix<mitk::ScalarType, 3,3> rotationMatrix;
  for (int i = 0; i < 3;++i)
    for (int j = 0; j < 3;++j)
      rotationMatrix[i][j] = transformation.get(i,j);
  rotationTransform  = itk::ScalableAffineTransform<mitk::ScalarType,3>::New();
  rotationTransform->SetMatrix(rotationMatrix);
  itk::ScalableAffineTransform<mitk::ScalarType,3>::Pointer geometryTransform = img->GetGeometry()->GetIndexToWorldTransform();
  geometryTransform->Compose(rotationTransform);
  img->GetGeometry()->SetIndexToWorldTransform(geometryTransform);

  if (dynamic_cast<mitk::DiffusionImage<short>*> (img.GetPointer()) != NULL)
  {
   // apply transformation to image geometry as well as to all gradients !?
  }
  else
  {
    // do regular stuff
  }
}

mitk::BatchedRegistration::TransformType mitk::BatchedRegistration::GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage, mitk::Image::Pointer mask)
{

  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );

  if (mask.IsNotNull())
  {
    registrationMethod->SetFixedImageMask(mask);
    registrationMethod->SetUseFixedImageMask(true);
  }
  registrationMethod->SetTransformToRigid();
  registrationMethod->SetCrossModalityOn();
  registrationMethod->SetMovingImage(movingImage);
  registrationMethod->Update();

  // TODO fancy shit, where you query and create a transformation type object thingy

  TransformType transformation;
  mitk::PyramidImageRegistrationMethod::TransformMatrixType rotationMatrix;
  rotationMatrix = registrationMethod->GetLastRotationMatrix().transpose();
  double param[6];
  registrationMethod->GetParameters(param); // first three: euler angles, last three translation
  for (unsigned int i = 0; i < 3; i++)
  {
    for (unsigned int j = 0; j < 3; ++j)
    {
      double value = rotationMatrix.get(i,j);
      transformation.set(i,j, value);
    }
  }
  transformation.set(0,3,-param[3]);
  transformation.set(1,3,-param[4]);
  transformation.set(2,3,-param[5]);
  transformation.set(3,3,1);

  return transformation;
}
