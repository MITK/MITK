#include "mitkRegistrationWrapper.h"

#include "mitkPyramidImageRegistrationMethod.h"
#include "mitkImage.h"
#include <mitkDiffusionImageCorrectionFilter.h>
#include "itkB0ImageExtractionImageFilter.h"
#include <itkResampleImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborExtrapolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <mitkDiffusionPropertyHelper.h>

#include <vnl/vnl_inverse.h>

void mitk::RegistrationWrapper::ApplyTransformationToImage(mitk::Image::Pointer img, const mitk::RegistrationWrapper::RidgidTransformType &transformation,double* offset, mitk::Image* resampleReference,  bool binary)
{
  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientDirections =
  static_cast<mitk::GradientDirectionsProperty*>( img->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

  if (gradientDirections.IsNull() || ( gradientDirections->Size() == 0) )
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
    if (resampleReference != NULL)
    {
      typedef itk::ResampleImageFilter<ItkImageType, ItkImageType>  ResampleFilterType;

      ItkImageType::Pointer itkReference = ItkImageType::New();
      CastToItkImage(resampleReference,itkReference);

      typedef itk::Function::WelchWindowFunction<4> WelchWindowFunction;
      typedef itk::WindowedSincInterpolateImageFunction< ItkImageType, 4,WelchWindowFunction> WindowedSincInterpolatorType;
      WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();


      typedef itk::LinearInterpolateImageFunction< ItkImageType> LinearInterpolatorType;
      LinearInterpolatorType::Pointer lin_interpolator = LinearInterpolatorType::New();

      typedef itk::NearestNeighborInterpolateImageFunction< ItkImageType, double > NearestNeighborInterpolatorType;
      NearestNeighborInterpolatorType::Pointer nn_interpolator = NearestNeighborInterpolatorType::New();

      typedef itk::BSplineInterpolateImageFunction< ItkImageType, double > BSplineInterpolatorType;
      BSplineInterpolatorType::Pointer bSpline_interpolator = BSplineInterpolatorType::New();


      ResampleFilterType::Pointer resampler = ResampleFilterType::New();
      resampler->SetInput(itkImage);
      resampler->SetReferenceImage( itkReference );
      resampler->UseReferenceImageOn();
      if (binary)
        resampler->SetInterpolator(nn_interpolator);
      else
        resampler->SetInterpolator(lin_interpolator);

      resampler->Update();

      GrabItkImageMemory(resampler->GetOutput(), img);
    }
    else
    {
      // !! CastToItk behaves very differently depending on the original data type
      // if the target type is the same as the original, only a pointer to the data is set
      // and an additional GrabItkImageMemory will cause a segfault when the image is destroyed
      // GrabItkImageMemory - is not necessary in this case since we worked on the original data
      // See Bug 17538.
      if (img->GetPixelType().GetComponentTypeAsString() != "double")
        img = GrabItkImageMemory(itkImage);
    }
  }
  else
  {
    mitk::Image::Pointer diffImages = dynamic_cast<mitk::Image*>(img.GetPointer());

    typedef itk::Euler3DTransform< double > RigidTransformType;
    RigidTransformType::Pointer rtransform = RigidTransformType::New();
    RigidTransformType::ParametersType parameters(RigidTransformType::ParametersDimension);

    for (int i = 0; i<6;++i)
    {
      parameters[i] = transformation[i];
    }

    rtransform->SetParameters( parameters );

    typedef itk::VectorImage<short, 3> ITKDiffusionImageType;
    ITKDiffusionImageType::Pointer itkVectorImagePointer = ITKDiffusionImageType::New();
    mitk::CastToItkImage(diffImages, itkVectorImagePointer);

    mitk::Point3D b0origin = itkVectorImagePointer->GetOrigin();
    b0origin[0]-=offset[0];
    b0origin[1]-=offset[1];
    b0origin[2]-=offset[2];

    mitk::Point3D newOrigin = rtransform->GetInverseTransform()->TransformPoint(b0origin);

    itk::Matrix<double,3,3> dir = itkVectorImagePointer->GetDirection();
    itk::Matrix<double,3,3> transM  ( vnl_inverse(rtransform->GetMatrix().GetVnlMatrix()));
    itk::Matrix<double,3,3> newDirection = transM * dir;

    itkVectorImagePointer->SetOrigin(newOrigin);
    itkVectorImagePointer->SetDirection(newDirection);
    diffImages->Modified();

    mitk::DiffusionImageCorrectionFilter::Pointer correctionFilter = mitk::DiffusionImageCorrectionFilter::New();

    // For Diff. Images: Need to rotate the gradients (works in-place)
    correctionFilter->SetImage(diffImages);
    correctionFilter->CorrectDirections(transM.GetVnlMatrix());
    img = diffImages;
  }
}

void mitk::RegistrationWrapper::GetTransformation(mitk::Image::Pointer fixedImage, mitk::Image::Pointer movingImage, RidgidTransformType transformation,double* offset, bool useSameOrigin, mitk::Image* mask)
{
  // Handle the case that fixed/moving image is a DWI image
  mitk::Image* fixedDwi = dynamic_cast<mitk::Image*> (fixedImage.GetPointer());
  mitk::Image* movingDwi = dynamic_cast<mitk::Image*> (movingImage.GetPointer());
  itk::B0ImageExtractionImageFilter<short,short >::Pointer b0Extraction = itk::B0ImageExtractionImageFilter<short,short>::New();
  offset[0]=offset[1]=offset[2]=0;

  typedef itk::VectorImage<short, 3> ITKDiffusionImageType;
  ITKDiffusionImageType::Pointer itkFixedDwiPointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(fixedDwi, itkFixedDwiPointer);

  ITKDiffusionImageType::Pointer itkMovingDwiPointer = ITKDiffusionImageType::New();
  mitk::CastToItkImage(movingDwi, itkMovingDwiPointer);

  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer fixedGradientDirections =
    static_cast<mitk::GradientDirectionsProperty*>( fixedDwi->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer movingGradientDirections =
    static_cast<mitk::GradientDirectionsProperty*>( movingDwi->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

  if (fixedGradientDirections.IsNull() || ( fixedGradientDirections->Size() == 0))
  { // Set b0 extraction as fixed image
    b0Extraction->SetInput( itkFixedDwiPointer );
    b0Extraction->SetDirections(fixedGradientDirections);
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    fixedImage = tmp;
  }
  if (movingGradientDirections || ( movingGradientDirections->Size() == 0))
  { // Set b0 extraction as moving image
    b0Extraction->SetInput( itkMovingDwiPointer );
    b0Extraction->SetDirections(movingGradientDirections);
    b0Extraction->Update();
    mitk::Image::Pointer tmp = mitk::Image::New();
    tmp->InitializeByItk(b0Extraction->GetOutput());
    tmp->SetVolume(b0Extraction->GetOutput()->GetBufferPointer());
    movingImage = tmp;
  }

  // align the offsets of the two images. this is done to avoid non-overlapping initialization
  Point3D origin = fixedImage->GetGeometry()->GetOrigin();
  Point3D originMoving = movingImage->GetGeometry()->GetOrigin();

  mitk::Image::Pointer tmpImage = movingImage->Clone();
  if (useSameOrigin)
  {
    offset[0] = originMoving[0]-origin[0];
    offset[1] = originMoving[1]-origin[1];
    offset[2] = originMoving[2]-origin[2];
    tmpImage->GetGeometry()->SetOrigin(origin);
  }

  // Start registration
  mitk::PyramidImageRegistrationMethod::Pointer registrationMethod = mitk::PyramidImageRegistrationMethod::New();
  registrationMethod->SetFixedImage( fixedImage );

  if (mask != NULL)
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
