#include "QmitkAutocropAction.h"

#include "mitkAutoCropImageFilter.h"
#include "mitkImageCast.h"
#include "mitkRenderingManager.h"
#include "mitkProgressBar.h"

#include <itkConstantPadImageFilter.h>

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
          // Reinit node
          mitk::RenderingManager::GetInstance()->InitializeViews(
            node->GetData()->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();

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
  int borderLiner = 3;

  mitk::Point3D mitkOriginPoint;
  double origin[3];
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkTransformImage->SetOrigin(origin);

  lowerPad[0]=borderLiner;
  lowerPad[1]=borderLiner;
  lowerPad[2]=borderLiner;

  upperPad[0]=borderLiner;
  upperPad[1]=borderLiner;
  upperPad[2]=borderLiner;

  padFilter->SetInput(itkTransformImage);
  padFilter->SetConstant(0);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);
  padFilter->UpdateLargestPossibleRegion();


  mitk::Image::Pointer paddedImage = mitk::Image::New();
  paddedImage->InitializeByItk(padFilter->GetOutput());
  mitk::CastToMitkImage(padFilter->GetOutput(), paddedImage);

  //calculate translation according to padding to get the new origin
  mitk::Point3D paddedOrigin = image->GetGeometry()->GetOrigin();
  mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  paddedOrigin[0] -= (borderLiner)*spacing[0];
  paddedOrigin[1] -= (borderLiner)*spacing[1];
  paddedOrigin[2] -= (borderLiner)*spacing[2];

  paddedImage->GetGeometry()->SetOrigin( paddedOrigin );

  return paddedImage;
}

void QmitkAutocropAction::SetSmoothed(bool smoothed)
{
 //not needed
}

void QmitkAutocropAction::SetDecimated(bool decimated)
{
  //not needed
}

void QmitkAutocropAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  //not needed
}

void QmitkAutocropAction::SetStdMultiWidget(QmitkStdMultiWidget *)
{
  //not needed
}

void QmitkAutocropAction::SetFunctionality(berry::QtViewPart *functionality)
{
  //not needed
}
