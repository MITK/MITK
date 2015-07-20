#include <mitkDataNodeFactory.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>
#include <mitkClassification.h>
//#include <mitkOpenCVToMitkImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkFlipImageFilter.h>
#include "itkImageDuplicator.h"

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// 2D Image
//#define filename "D:\\gray.jpg"
//#define VDimension 2

// 3D Image
//#define filename "D:\\pic3d.nrrd"
#define VDimension 3

int mitkGenerateFeatureVectFromImageTest(int argc, char* argv[])
{
  typedef unsigned char PixelType;
  typedef itk::Image< PixelType,VDimension > ImageType;
  typedef ImageType::Pointer ImageTypePtr;
  typedef itk::ImageDuplicator< ImageType > DuplicatorType;

  typedef itk::ImageFileReader< ImageType> ReaderType;
  typedef itk::ImageFileWriter< ImageType> WriterType;

  if(argc < 3)
  {
    MITK_INFO << "Please specify number of features per Voxel per Input (MUST BE IMPLEMENTED) and at least one input image" << std::endl;
    MITK_INFO << " Usage: 1 d:\\pic3d.nrrd [d:\\pic3d.nrrd] .." << std::endl;
    return -1;
  }

  int k = atoi(argv[1]);

  std::vector<std::string> inputFile;

  for(int i = 2; i < argc; i++)
  {
    inputFile.push_back(argv[i]);
  }

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  std::vector<ImageTypePtr> inputImages;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();

  for(int i = 0; i< inputFile.size(); i++)
  {
    reader->SetFileName(inputFile.at(i));
    reader->Update();

    duplicator->SetInputImage(reader->GetOutput());
    duplicator->Update();
    inputImages.push_back(duplicator->GetOutput());
  }

  mitk::mitkClassificationFeatureVectCreator<PixelType,VDimension>::Pointer featureVectCreator = mitk::mitkClassificationFeatureVectCreator<PixelType,VDimension>::New();

  // SET NUMBER OF FeaturesProperly
  featureVectCreator->SetNumberOfFeaturesPerVoxel(k);

  for(int i = 0; i < inputImages.size(); i++)
  {
    featureVectCreator->SetNthInput(i,inputImages.at(i));
  }

  featureVectCreator->Update();

  std::cout << "Number of Inputs: " << featureVectCreator->GetNumberOfInputs() << std::endl;

  cv::Mat output = featureVectCreator->GetOuputCv();

  float f1 = output.at<float>(0,0);
  float f2 = output.at<float>(0,1);

  MITK_INFO << "OutputVector :" << std::endl << output << std::endl;

  cv::waitKey();

  //// Test with 2D Image for "visual" verification
  //if(VDimension == 2)
  //{
  //  ImageTypePtr image = inputImages.at(0);

  //  unsigned int width;
  //  unsigned int height;
  //  unsigned int x,y;

  //  width = image->GetLargestPossibleRegion().GetSize(0);
  //  height = 2* image->GetLargestPossibleRegion().GetSize(1);

  //  cv::Mat testImage(height,width, CV_8UC1);
  //  cv::Mat testImage2(height,width, CV_8UC1);
  //  cv::Mat testImage3(height,width, CV_8UC1);

  //  unsigned int index = 0;

  //  for( index = 0; index < width*height ; index++)
  //  {
  //    x = (int) index % (width);
  //    float d = index / (width);
  //    y = (int) floor(  d);

  //    unsigned char value;
  //    float val = output.at<float>(index,0);
  //    value = (unsigned char) val;
  //    testImage.at<unsigned char>(y,x)= value;

  //    if(featureVectCreator->GetIsGenerateResponseEnabled() == false)

  //    {
  //      value = (unsigned char) output.at<float>(index,1);
  //      testImage2.at<unsigned char>(y,x)= value;

  //      value = (unsigned char) output.at<float>(index,2);
  //      testImage3.at<unsigned char>(y,x)= value;
  //    }
  //  }

  //  cv::imshow("testImage1", testImage);
  //  cv::imshow("testImage2", testImage2);
  //  cv::imshow("testImage3", testImage3);
  //  cv::waitKey();

  //}

  //if(VDimension == 3)
  //{
  // }

  return 0;
}