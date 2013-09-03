#include "mitkBatchedRegistration.h"
#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkDiffusionImage.h"

#include <mitkRotationOperation.h>

#include <itkScalableAffineTransform.h>
#include <vnl/vnl_inverse.h>
// VTK
#include <vtkTransform.h>

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
  itk::ScalableAffineTransform<mitk::ScalarType,3>::Pointer rotationTransform;
  vtkMatrix4x4* transformationMatrix = vtkMatrix4x4::New();
  for (int i = 0; i < 4;++i)
    for (int j = 0; j < 4;++j)
      transformationMatrix->Element[i][j] = transformation.get(i,j);

  rotationTransform  = itk::ScalableAffineTransform<mitk::ScalarType,3>::New();

  itk::ScalableAffineTransform<mitk::ScalarType,3>::Pointer geometryTransform = img->GetGeometry()->GetIndexToWorldTransform();
  img->GetGeometry()->Compose( transformationMatrix); //SetIndexToWorldTransform(geometryTransform);

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
  else
  {
    registrationMethod->SetUseFixedImageMask(false);
  }

  registrationMethod->SetTransformToRigid();
  registrationMethod->SetCrossModalityOn();
  registrationMethod->SetMovingImage(movingImage);
  registrationMethod->Update();

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
  //transformation =vnl_inverse<double>(transformation);
  return transformation;
}
