#include "mitkBatchedRegistration.h"
#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkDiffusionImage.h"

#include <mitkRotationOperation.h>
#include <mitkDiffusionImageCorrectionFilter.h>
#include "itkB0ImageExtractionImageFilter.h"

#include <itkScalableAffineTransform.h>
#include <vnl/vnl_inverse.h>
// VTK
#include <vtkTransform.h>

// DEBUG
#include <mitkIOUtil.h>


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
    RidgidTransformType transf = new double(6);
    double offset[3];
    //GetTransformation(m_FixedImage, m_MovingReference,transf,NULL,offset);
    // store it as first element in vector
  //  ApplyTransformationToImage(m_MovingReference,transf,NULL,false,offset);
    m_RegisteredImages.push_back(m_MovingReference);
    // apply transformation to whole batch
    std::vector<mitk::Image::Pointer>::const_iterator itEnd = m_ImageBatch.end();
    for (std::vector<mitk::Image::Pointer>::iterator it = m_ImageBatch.begin(); it != itEnd; ++it)
    {
      //TODO fixme
     // ApplyTransformationToImage(*it,transf);
      m_RegisteredImages.push_back(*it);
    }
  }
  return m_RegisteredImages;
}

void mitk::BatchedRegistration::ApplyTransformationToImage(mitk::Image::Pointer &img, const mitk::BatchedRegistration::RidgidTransformType &transformation,double* offset, mitk::Image::Pointer resampleReference,  bool binary) const
{
  typedef mitk::DiffusionImage<short> DiffusionImageType;

  mitk::Image::Pointer ref;
  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetTransformToRigid();
  if (binary)
    registrationMethod->SetUseNearestNeighborInterpolation(true);

  if (resampleReference.IsNotNull())
  {
    registrationMethod->SetFixedImage( resampleReference );
  }
  else
  {
    // clone image, to prevent recursive access on resampling ..
    ref = img->Clone();
    registrationMethod->SetFixedImage( ref );
  }


  if (dynamic_cast<DiffusionImageType*> (img.GetPointer()) == NULL)
  {
    img = registrationMethod->GetResampledMovingImage(img, transformation);
  }
  else
  {

    MITK_INFO << " Transformation : " << transformation[3] << ", "<< transformation[4] << ", "<< transformation[5];
    MITK_INFO << " Offset : " << offset[0] << ", "<< offset[1] << ", "<< offset[2];
    MITK_INFO << " Original Origin Moving Image : " << img->GetGeometry()->GetOrigin();
    DiffusionImageType::Pointer diffImages =
        dynamic_cast<DiffusionImageType*>(img.GetPointer());

    // Workaround for Diffusion Images:
    // Extracting a B0-image
    // Apply the transformation to this image
    // Copy Transformation informations from B0-ITK-Image to
    //    diffusion vector image.
    itk::B0ImageExtractionImageFilter<short,short >::Pointer b0Extraction =
        itk::B0ImageExtractionImageFilter<short,short>::New();
    b0Extraction->SetInput(diffImages->GetVectorImage());
    b0Extraction->SetDirections(diffImages->GetDirections());
    b0Extraction->Update();

    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    tmp->Modified();

    MITK_INFO << " Original B0 Moving Image : " << b0Extraction->GetOutput()->GetOrigin();
    MITK_INFO << " Original MITK TMP Image : " << tmp->GetGeometry()->GetOrigin();

 //   itk::Euler3DTransform< double > bullTrans;
 //   bullTrans.GetInverseTransform()->GetIn

    // Fancy shit
    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();
    RigidTransformType::ParametersType parameters(RigidTransformType::ParametersDimension);
    //itk::Euler3DTransform< double >* rtransform;

    for (int i = 0; i<6;++i)
      parameters[i] = transformation[i];

    rtransform->SetParameters( parameters );
    mitk::Point3D b0origin = tmp->GetGeometry()->GetOrigin();
    b0origin[0]-=offset[0];
    b0origin[1]-=offset[1];
    b0origin[2]-=offset[2];
    tmp->GetGeometry()->SetOrigin(b0origin);

    // apply transformation
    ref = tmp->Clone();
    registrationMethod->SetFixedImage( ref );

    mitk::Image::Pointer resampled = registrationMethod->GetResampledMovingImage(tmp, transformation);
    resampled->Modified();

    mitk::Point3D    newOrigin = rtransform->TransformPoint(b0origin);
        tmp->GetGeometry()->SetOrigin(newOrigin);
        IOUtil::SaveImage(tmp, "/home/cweber/B0_ManualChangedOriginNormal.nrrd");

 newOrigin = rtransform->GetInverseTransform()->TransformPoint(b0origin);
    tmp->GetGeometry()->SetOrigin(newOrigin);
    IOUtil::SaveImage(tmp, "/home/cweber/B0_ManualChangedOriginInverse.nrrd");

    {
      itk::Euler3DTransform< double >::Pointer blub = itk::Euler3DTransform< double >::New();
      itk::Image<short, 3>::Pointer itkTmp;
      mitk::CastToItkImage<itk::Image<short, 3> >(tmp, itkTmp);
      itk::Image<short, 3>::DirectionType dir = itkTmp->GetDirection();
     // rtransform->GetInverse(blub.GetPointer());
      itk::Matrix<double,3,3>  transM  ( vnl_inverse(rtransform->GetMatrix().GetVnlMatrix()));
      itk::Matrix<double,3,3> erg = transM * dir;
      itkTmp->SetDirection(erg);
      mitk::CastToMitkImage(itkTmp, tmp);

      IOUtil::SaveImage(tmp, "/home/cweber/B0_ManualChangedDirectionInverse.nrrd");
    }


    // End of fancy shit


    MITK_INFO << " Resampled Origin : " << resampled->GetGeometry()->GetOrigin();
    MITK_INFO << " Resampled tmp ? Origin : " << tmp->GetGeometry()->GetOrigin();

    IOUtil::SaveImage(resampled, "/home/cweber/resampledB0heuteimg.nrrd");

    itk::Image<short, 3>::Pointer itkTmp;
    mitk::CastToItkImage<itk::Image<short, 3> >(resampled, itkTmp);

    MITK_INFO << "ITK ORIGIN " << itkTmp->GetOrigin();
    diffImages->SetGeometry(resampled->GetGeometry());
    diffImages->GetVectorImage()->SetOrigin(itkTmp->GetOrigin());
    diffImages->GetVectorImage()->SetDirection(itkTmp->GetDirection());
    diffImages->GetVectorImage()->SetSpacing(itkTmp->GetSpacing());
    diffImages->Modified();
    MITK_INFO << "DIFF IM ORIGIN " << diffImages->GetGeometry()->GetOrigin();


    mitk::DiffusionImageCorrectionFilter<short>::Pointer correctionFilter =
        mitk::DiffusionImageCorrectionFilter<short>::New();

    // For Diff. Images: Need to rotate the gradients
    correctionFilter->SetImage(diffImages);
    //correctionFilter->CorrectDirections(vnlRotationMatrix);
    //correctionFilter->Update();

    img = diffImages;
  }

}

void mitk::BatchedRegistration::GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage, RidgidTransformType transformation,double* offset, mitk::Image::Pointer mask)
{
  // Handle case that fixed or moving image is a DWI image
  mitk::DiffusionImage<short>* fixedDwi = dynamic_cast<mitk::DiffusionImage<short>*> (fixedImage.GetPointer());
  mitk::DiffusionImage<short>* movingDwi = dynamic_cast<mitk::DiffusionImage<short>*> (movingImage.GetPointer());
  itk::B0ImageExtractionImageFilter<short,short >::Pointer b0Extraction = itk::B0ImageExtractionImageFilter<short,short>::New();
  offset[0]=offset[1]=offset[2]=0;
  if (fixedDwi != NULL)
  {
    b0Extraction->SetInput(fixedDwi->GetVectorImage());
    b0Extraction->SetDirections(fixedDwi->GetDirections());
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    fixedImage = tmp;
  }
  if (movingDwi != NULL)
  {
    b0Extraction->SetInput(movingDwi->GetVectorImage());
    b0Extraction->SetDirections(movingDwi->GetDirections());
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    movingImage = tmp;

    Point3D origin = fixedImage->GetGeometry()->GetOrigin();
    Point3D originMoving = movingImage->GetGeometry()->GetOrigin();
    offset[0] = originMoving[0]-origin[0];
    offset[1] = originMoving[1]-origin[1];
    offset[2] = originMoving[2]-origin[2];
    movingImage->GetGeometry()->SetOrigin(origin);
  }
  // Start registration
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

  IOUtil::SaveImage(registrationMethod->GetResampledMovingImage(),"/home/cweber/b0_filterRegisteredImage");

  registrationMethod->GetParameters(transformation); // first three: euler angles, last three translation
}
