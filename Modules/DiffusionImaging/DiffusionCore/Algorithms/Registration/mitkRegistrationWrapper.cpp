#include "mitkRegistrationWrapper.h"

#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkDiffusionImage.h"
#include <mitkDiffusionImageCorrectionFilter.h>
#include "itkB0ImageExtractionImageFilter.h"
#include <itkResampleImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkNearestNeighborExtrapolateImageFunction.h>

#include <vnl/vnl_inverse.h>


mitk::RegistrationWrapper::RegistrationWrapper()
{
}

void mitk::RegistrationWrapper::ApplyTransformationToImage(mitk::Image::Pointer &img, const mitk::RegistrationWrapper::RidgidTransformType &transformation,double* offset, mitk::Image::Pointer resampleReference,  bool binary) const
{
  typedef mitk::DiffusionImage<short> DiffusionImageType;

  if (dynamic_cast<DiffusionImageType*> (img.GetPointer()) == NULL)
  {
    ItkImageType::Pointer itkImage = ItkImageType::New();
    CastToItkImage(img, itkImage);

    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();
    RigidTransformType::ParametersType parameters(RigidTransformType::ParametersDimension);

    for (int i = 0; i<6;++i)
      parameters[i] = transformation[i];

    rtransform->SetParameters( parameters );

    mitk::Point3D origin = itkImage->GetOrigin();
    origin[0]-=offset[0];
    origin[1]-=offset[1];
    origin[2]-=offset[2];

    mitk::Point3D newOrigin = rtransform->GetInverseTransform()->TransformPoint(origin);

    itk::Matrix<double,3,3> dir = itkImage->GetDirection();
    itk::Matrix<double,3,3> transM  ( vnl_inverse(rtransform->GetMatrix().GetVnlMatrix()));
    itk::Matrix<double,3,3> newDirection = transM * dir;

    itkImage->SetOrigin(newOrigin);
    itkImage->SetDirection(newDirection);

    // Perform Resampling if reference image is provided
    if (resampleReference.IsNotNull())
    {
      typedef itk::ResampleImageFilter<ItkImageType, ItkImageType>  ResampleFilterType;

      ItkImageType::Pointer itkReference = ItkImageType::New();
      CastToItkImage(resampleReference,itkReference);

      typedef itk::WindowedSincInterpolateImageFunction< ItkImageType, 3> WindowedSincInterpolatorType;
      typename WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

      typedef itk::NearestNeighborInterpolateImageFunction< ItkImageType, double > NearestNeighborInterpolatorType;
      typename NearestNeighborInterpolatorType::Pointer nn_interpolator = NearestNeighborInterpolatorType::New();


      ResampleFilterType::Pointer resampler = ResampleFilterType::New();
      resampler->SetInput(itkImage);
      resampler->SetReferenceImage( itkReference );
      resampler->UseReferenceImageOn();
      if (binary)
        resampler->SetInterpolator(nn_interpolator);
      else
        resampler->SetInterpolator(sinc_interpolator);

      resampler->Update();

      GrabItkImageMemory(resampler->GetOutput(), img);
    }
    else
     GrabItkImageMemory(itkImage, img);

  }
  else
  {
    DiffusionImageType::Pointer diffImages = dynamic_cast<DiffusionImageType*>(img.GetPointer());

    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();
    RigidTransformType::ParametersType parameters(RigidTransformType::ParametersDimension);

    for (int i = 0; i<6;++i)
    {
      parameters[i] = transformation[i];
    }

    rtransform->SetParameters( parameters );

    mitk::Point3D b0origin = diffImages->GetVectorImage()->GetOrigin();
    b0origin[0]-=offset[0];
    b0origin[1]-=offset[1];
    b0origin[2]-=offset[2];

    mitk::Point3D newOrigin = rtransform->GetInverseTransform()->TransformPoint(b0origin);

    itk::Matrix<double,3,3> dir = diffImages->GetVectorImage()->GetDirection();
    itk::Matrix<double,3,3> transM  ( vnl_inverse(rtransform->GetMatrix().GetVnlMatrix()));
    itk::Matrix<double,3,3> newDirection = transM * dir;

    diffImages->GetVectorImage()->SetOrigin(newOrigin);
    diffImages->GetVectorImage()->SetDirection(newDirection);
    diffImages->Modified();

    mitk::DiffusionImageCorrectionFilter<short>::Pointer correctionFilter = mitk::DiffusionImageCorrectionFilter<short>::New();

    // For Diff. Images: Need to rotate the gradients (works in-place)
    correctionFilter->SetImage(diffImages);
    correctionFilter->CorrectDirections(transM.GetVnlMatrix());
    img = diffImages;
  }
}

void mitk::RegistrationWrapper::GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage, RidgidTransformType transformation,double* offset, mitk::Image::Pointer mask)
{
  // Handle the case that fixed/moving image is a DWI image
  mitk::DiffusionImage<short>* fixedDwi = dynamic_cast<mitk::DiffusionImage<short>*> (fixedImage.GetPointer());
  mitk::DiffusionImage<short>* movingDwi = dynamic_cast<mitk::DiffusionImage<short>*> (movingImage.GetPointer());
  itk::B0ImageExtractionImageFilter<short,short >::Pointer b0Extraction = itk::B0ImageExtractionImageFilter<short,short>::New();
  offset[0]=offset[1]=offset[2]=0;
  if (fixedDwi != NULL)
  { // Set b0 extraction as fixed image
    b0Extraction->SetInput(fixedDwi->GetVectorImage());
    b0Extraction->SetDirections(fixedDwi->GetDirections());
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    fixedImage = tmp;
  }
  if (movingDwi != NULL)
  { // Set b0 extraction as moving image
    b0Extraction->SetInput(movingDwi->GetVectorImage());
    b0Extraction->SetDirections(movingDwi->GetDirections());
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    movingImage = tmp;
  }

  // align the offsets of the two images. this is done to avoid non-overlapping initialization
  Point3D origin = fixedImage->GetGeometry()->GetOrigin();
  Point3D originMoving = movingImage->GetGeometry()->GetOrigin();

  offset[0] = originMoving[0]-origin[0];
  offset[1] = originMoving[1]-origin[1];
  offset[2] = originMoving[2]-origin[2];

  mitk::Image::Pointer tmpImage = movingImage->Clone();
  tmpImage->GetGeometry()->SetOrigin(origin);
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

  registrationMethod->SetMovingImage(tmpImage);
  registrationMethod->Update();
  registrationMethod->GetParameters(transformation); // first three: euler angles, last three translation
}
