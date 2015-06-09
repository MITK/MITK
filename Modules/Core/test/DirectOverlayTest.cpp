#include <mitkTestingMacros.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>

#include <itkTestingComparisonImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageFileWriter.h>

#include <mitkImagePixelReadAccessor.h>

//#include <boost/lexical_cast.hpp>

class DirectOverlayTestClass
{
public:
  template<typename TPixel, unsigned int VDimensions>
  static void InternalThreshold(
    const itk::Image<TPixel, VDimensions>* image,
    mitk::Image::Pointer& output,
    const double th[])
  {
    typedef itk::Image<TPixel, VDimensions> InputImageType;
    typedef itk::Image<unsigned int, VDimensions> OutputImageType;
    typedef itk::BinaryThresholdImageFilter<
      InputImageType, OutputImageType> BinaryThresholdFilterType;

    typename BinaryThresholdFilterType::Pointer thresholder =
      BinaryThresholdFilterType::New();
    thresholder->SetInput(image);
    thresholder->SetLowerThreshold(th[0]);
    thresholder->SetUpperThreshold(th[1]);
    thresholder->SetInsideValue(255);
    thresholder->SetOutsideValue(0);
    thresholder->Update();

    output = mitk::ImportItkImage( thresholder->GetOutput() );
   // mitk::IOUtil::SaveImage( output, "/tmp/out.nii" );
    std::cout << "extra line";
  }

  template<typename TPixel, unsigned int VDimensions>
  static void InternalThreshold2(
    const itk::Image<TPixel, VDimensions>* image,
    itk::Image< unsigned int, 3>::Pointer& output,
    const double th[])
  {
    typedef itk::Image<TPixel, VDimensions> InputImageType;
    typedef itk::Image<unsigned int, VDimensions> OutputImageType;
    typedef itk::BinaryThresholdImageFilter<
      InputImageType, OutputImageType> BinaryThresholdFilterType;

    typename BinaryThresholdFilterType::Pointer thresholder =
      BinaryThresholdFilterType::New();
    thresholder->SetInput(image);
    thresholder->SetLowerThreshold(th[0]);
    thresholder->SetUpperThreshold(th[1]);
    thresholder->SetInsideValue(255);
    thresholder->SetOutsideValue(0);
    thresholder->Update();

    output = thresholder->GetOutput();
  }

  static void TestOverlay(mitk::Image::Pointer original,
    mitk::Image::Pointer truth,
    const double lower,
    const double upper)
  {
    mitk::Image::Pointer overlayImage;
    const double th[] = {lower, upper};
    typedef itk::Image< unsigned int, 3> ImageType;
    ImageType::Pointer itkOverlayImage = ImageType::New();

    AccessByItk_2(original, InternalThreshold,
      overlayImage,
      th);
/*
    AccessFixedDimensionByItk_2( original, InternalThreshold2,
                                 3, itkOverlayImage,
                                 th );

    overlayImage = mitk::ImportItkImage( itkOverlayImage );
*/
    //mitk::IOUtil::SaveImage(truth, "/tmp/truth_TestOverlay.nii");
    try
    {
      //mitk::Image::Pointer temp = overlayImage;
      mitk::IOUtil::SaveImage(overlayImage, "/tmp/overlayImage_TestOverlay.nrrd");
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Save image: exception : " << e.what();
    }

    typedef itk::Image<unsigned int, 3> InputImageType;

    InputImageType::Pointer overlayItk;
    try
    {
      mitk::CastToItkImage(overlayImage, overlayItk);
    }
    catch( const mitk::Exception &e)
    {
      MITK_ERROR << "(CAST) Catched exception while creating accessor "<< e.what();
      //MITK_TEST_FAILED_MSG("Exception for ouverlay image");
    }


/*
    typedef  itk::ImageFileWriter< InputImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName("/tmp/overlayITK_TestOverlay.nii");
    writer->SetInput(overlayItk);
    writer->Update();
*/
    InputImageType::Pointer truthItk;
    mitk::CastToItkImage(truth, truthItk);

    bool difference = false;
/*
    try
    {
      typedef unsigned int TPixel;

      itk::ImageRegionConstIteratorWithIndex< InputImageType > iter( truthItk, truthItk->GetLargestPossibleRegion() );
      iter.GoToBegin();

      mitk::ImagePixelReadAccessor< TPixel, 3 > readAccessor( overlayImage, overlayImage->GetVolumeData(0), mitk::ImageAccessorBase::ExceptionIfLocked );

      while( !iter.IsAtEnd() )
      {
        TPixel ref = iter.Get();
        TPixel val = readAccessor.GetPixelByIndex( iter.GetIndex() );

        difference |= ( ref != val );
        //if( difference )
        //{
          std::cout << iter.GetIndex() << ":" << ref << " ? " << val << "\n";
        //}
        ++iter;
      }
    }
    catch( const mitk::Exception &e)
    {
      MITK_ERROR << "Catched exception while creating accessor "<< e.what();
      //MITK_TEST_FAILED_MSG("Exception for ouverlay image");
    }



*/


/*
    typedef itk::Testing::ComparisonImageFilter
      <InputImageType, InputImageType> ComparisonImageFilterType;
    ComparisonImageFilterType::Pointer comp =
      ComparisonImageFilterType::New();
    comp->SetValidInput(truthItk);
    comp->SetTestInput(overlayItk);
    try
    {
      comp->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "ITK Exception: " << e.what();
    }
*/
    typedef unsigned int TPixel;

    itk::ImageRegionConstIteratorWithIndex< InputImageType > iter( truthItk, truthItk->GetLargestPossibleRegion() );
    itk::ImageRegionConstIteratorWithIndex< InputImageType > iter2( overlayItk, overlayItk->GetLargestPossibleRegion() );
    iter.GoToBegin();

    unsigned int counter = 0;
    while( !iter.IsAtEnd() && !iter2.IsAtEnd() )
    {
      TPixel ref = iter.Get();
      TPixel val = iter2.Get();

      if( ref != val )
      {
         counter++;
   //     std::cout << iter.GetIndex() << ":" << ref << " ? " << val << "\n";
      }

      ++iter;
      ++iter2;
    }

    std::cout << "Differs in " << counter << "voxels" << std::endl;


    MITK_TEST_CONDITION_REQUIRED(
     //comp->GetNumberOfPixelsWithDifferences() == 0,
      counter == 0,
      "Comparing overlay with ground truth")


  }

  static void TestDirectOverlay(char* in, char* gt,
    const int lower, const int upper)
  {
    mitk::Image::Pointer original = mitk::IOUtil::LoadImage(in);
    mitk::Image::Pointer truth = mitk::IOUtil::LoadImage(gt);

    if(original.IsNotNull() &&  original->GetDimension() == 3
      && truth.IsNotNull() && truth->GetDimension() == 3
      && upper > lower)
    {
      TestOverlay(original, truth, lower, upper);
    }
    else
    {
      MITK_TEST_FAILED_MSG(<<"Invalid parameters");
    }
  }
};

int DirectOverlayTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DirectOverlay")

  MITK_TEST_CONDITION_REQUIRED(argc >= 5,
    "File to load has been specified on the command line");

  unsigned int lower = 0,
    upper = 0;


    try
  {
    sscanf( argv[3], "%d", &lower);
    sscanf( argv[4], "%d", &upper);
    //lower = boost::lexical_cast<double>(argv[3]);
    //upper = boost::lexical_cast<double>(argv[4]);

    MITK_INFO << "Got values: " << lower << " : " << upper ;
  }
  catch(std::exception& e)
  {
    MITK_TEST_FAILED_MSG(<<e.what());
  }

  DirectOverlayTestClass::TestDirectOverlay(argv[1], argv[2], lower, upper);

  MITK_TEST_END()
}

