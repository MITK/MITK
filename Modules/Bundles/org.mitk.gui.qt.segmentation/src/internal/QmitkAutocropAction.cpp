#include "QmitkAutocropAction.h"

#include "mitkAutoCropImageFilter.h"
#include "mitkImageCast.h"

#include "mitkProgressBar.h"

#include <itkConstantPadImageFilter.h>
#include <berryQtViewPart.h>

//needed for qApp 
#include <qcoreapplication.h>

QmitkAutocropAction::QmitkAutocropAction()
{
}

QmitkAutocropAction::~QmitkAutocropAction()
{
}

void QmitkAutocropAction::Run( const std::vector<mitk::DataNode*>& selectedNodes )
{
   NodeList selection = selectedNodes;

  for ( NodeList::iterator iter = selection.begin(); iter != selection.end(); ++iter )
  {
    mitk::DataNode* node = *iter;

    if (node)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNull()) return;

      mitk::ProgressBar::GetInstance()->AddStepsToDo(10);
      mitk::ProgressBar::GetInstance()->Progress(2);

      qApp->processEvents();

      mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
      cropFilter->SetInput( image );
      cropFilter->SetBackgroundValue( 0 );
      try
      {
        cropFilter->Update();

        image = cropFilter->GetOutput();

        if (image.IsNotNull())
        {
          node->SetData( this->IncreaseCroppedImageSize(image) ); // bug fix 3145
        }
      }
      catch(...)
      {
        MITK_ERROR << "Cropping image failed...";
      }
      mitk::ProgressBar::GetInstance()->Progress(8);
    }
    else
    {
      MITK_INFO << "   a NULL node selected";
    }
  }
}

mitk::Image::Pointer QmitkAutocropAction::IncreaseCroppedImageSize( mitk::Image::Pointer image )
{
  typedef itk::Image< short, 3 > ImageType;
  typedef itk::Image< unsigned char, 3 > PADOutputImageType;
  ImageType::Pointer itkTransformImage = ImageType::New();
  mitk::CastToItkImage( image, itkTransformImage );

  typedef itk::ConstantPadImageFilter< ImageType, PADOutputImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();

  unsigned long upperPad[3];
  unsigned long lowerPad[3];
  int borderLiner = 6;

  mitk::Point3D mitkOriginPoint;
  double origin[3];
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkTransformImage->SetOrigin(origin);

  lowerPad[0]=borderLiner/2;
  lowerPad[1]=borderLiner/2;
  lowerPad[2]=borderLiner/2;

  upperPad[0]=borderLiner/2;
  upperPad[1]=borderLiner/2;
  upperPad[2]=borderLiner/2;

  padFilter->SetInput(itkTransformImage);
  padFilter->SetConstant(0);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);
  padFilter->UpdateLargestPossibleRegion();


  mitk::Image::Pointer paddedImage = mitk::Image::New();
  mitk::CastToMitkImage(padFilter->GetOutput(), paddedImage);

  paddedImage->SetGeometry(image->GetGeometry());

  //calculate translation vector according to padding to get the new origin
  mitk::Vector3D transVector = image->GetGeometry()->GetSpacing();
  transVector[0] = -(borderLiner/2);
  transVector[1] = -(borderLiner/2);
  transVector[2] = -(borderLiner/2);

  mitk::Vector3D newTransVectorInmm = image->GetGeometry()->GetSpacing();

  image->GetGeometry()->IndexToWorld(mitkOriginPoint, transVector, newTransVectorInmm);
  paddedImage->GetGeometry()->Translate(newTransVectorInmm);
  //paddedImage->SetRequestedRegionToLargestPossibleRegion();

  return paddedImage;
}

void QmitkAutocropAction::SetSmoothed(bool smoothed)
{
 //not needed
}

void QmitkAutocropAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  //not needed
}

void QmitkAutocropAction::SetFunctionality(berry::QtViewPart *functionality)
{
  //not needed
}
