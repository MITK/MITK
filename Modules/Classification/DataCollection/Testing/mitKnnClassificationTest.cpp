#include <mitkStandaloneDataStorage.h>
#include <mitkIOUtil.h>
#include <mitkClassification.h>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkFlipImageFilter.h>
#include "itkImageDuplicator.h"
#include <stdlib.h>
#include <itkFileTools.h>

#include "ml.h"

#include <mitkClassResultToImageFilter.h>

#include <mitkClassificationKnn.h>
#include <mitkImageDataCollections.h>

#define VDimension 3

// 2 D:\\output\\ D:\\TestImages\\train1.nrrd D:\\TestImages\\train2.nrrd D:\\TestImages\\train3.nrrd  D:\\TestImages\\target1.nrrd D:\\TestImages\\target2.nrrd D:\\TestImages\\target3.nrrd D:\\TestImages\\test1.nrrd D:\\TestImages\\test2.nrrd D:\\TestImages\\test3.nrrd

typedef unsigned char PixelType;
typedef itk::Image< PixelType,VDimension > ImageType;
typedef ImageType::Pointer ImageTypePtr;

typedef itk::ImageFileReader< ImageType> ReaderType;
typedef itk::ImageFileWriter< ImageType> WriterType;
typedef mitk::mitkClassResultToImageFilter<unsigned char,VDimension>  ResultToImageType;
typedef itk::ImageDuplicator< ImageType > DuplicatorType;

//cv::Mat create2DImageFromVect(cv::Size size, cv::Mat& image )
//{
//  unsigned long index;
//  unsigned long x = 0;
//  unsigned long y = 0;
//
//  cv::Mat outputImage(size.height,size.width,CV_32FC1);
//
//  if(image.cols < 0)
//  {
//    std::cout << "Can't create 2D Image" << std::endl;
//    return outputImage;
//
//  }
//
//  for( index = 0; index < size.width*size.height ; index++)
//  {
//    x = (int) index % (size.width);
//    float d = index / (size.width);
//    y = (int) floor(  d);
//
//    unsigned char value;
//    float val = image.at<float>(index,0);
//    value = (unsigned char) val;
//    outputImage.at<unsigned char>(y,x)= value;
//  }
//
//  return outputImage;
//}

int mitKnnClassificationTest(int argc, char* argv[] )
{
    if(argc < 5 )
    {
        MITK_INFO << "Please specify number of NN  as first parameter, as second parameter output path and train, target and test images in abitrary order" << std::endl << " train data must start with 'train*', target with 'target*' and test with 'test.'" << std::endl<< "Usage: 3 D:\\output\\ D:\\TestImages\\train1.nrrd D:\\TestImages\\train2.nrrd D:\\TestImages\\train3.nrrd  D:\\TestImages\\target1.nrrd D:\\TestImages\\target2.nrrd D:\\TestImages\\target3.nrrd D:\\TestImages\\test1.nrrd D:\\TestImages\\test2.nrrd D:\\TestImages\\test3.nrrd" << std::endl;
        exit(-1);
    }

    std::vector<std::string> inputFilesTrain;
    std::vector<std::string> inputFilesTarget;
    std::vector<std::string> inputFilesTest;
    std::string outputFolder;

    int k = atoi(argv[1]);

    outputFolder = argv[2];

    for(int i = 3; i < argc; i++)
    {
        std::string str;
        str = argv[i];
        if(str.find("train") != std::string::npos)
        {
            inputFilesTrain.push_back(str);
        }
        else if (str.find("target") != std::string::npos)
        {
            inputFilesTarget.push_back(str);
        }
        else if(str.find("test") != std::string::npos)
        {
            inputFilesTest.push_back(str);
        }
    }

    if( ( inputFilesTrain.size() || inputFilesTarget.size() ) <= 0)
    {
        MITK_INFO << "No test or training image set! " << std::endl;
        return -1;
    }

    if(inputFilesTrain.size() != inputFilesTarget.size())
    {
        MITK_INFO << "Number of training and target images are not equal - please specify same number for training " << std::endl;
        return -1;
    }

    std::vector<ImageTypePtr> trainImages;
    std::vector<ImageTypePtr> targetImages;
    std::vector<ImageTypePtr> testImages;

    ReaderType::Pointer reader = ReaderType::New();
    WriterType::Pointer writer = WriterType::New();

    DuplicatorType::Pointer duplicator = DuplicatorType::New();

    for(int i = 0; i< inputFilesTrain.size(); i++)
    {
        reader->SetFileName(inputFilesTrain.at(i));
        reader->Update();

        duplicator->SetInputImage(reader->GetOutput());
        duplicator->Update();
        trainImages.push_back(duplicator->GetOutput());
    }

    for(int i = 0; i< inputFilesTarget.size(); i++)
    {
        reader->SetFileName(inputFilesTarget.at(i));
        reader->Update();

        duplicator->SetInputImage(reader->GetOutput());
        duplicator->Update();
        targetImages.push_back(duplicator->GetOutput());
    }

    for(int i = 0; i< inputFilesTest.size(); i++)
    {
        reader->SetFileName(inputFilesTest.at(i));
        reader->Update();

        duplicator->SetInputImage(reader->GetOutput());
        duplicator->Update();
        testImages.push_back(duplicator->GetOutput());
    }

    ResultToImageType::Pointer resultToImage = ResultToImageType::New();

    mitk::mitkClassificationFeatureVectCreator<PixelType,VDimension>::Pointer featureVectCreator = mitk::mitkClassificationFeatureVectCreator<PixelType,VDimension>::New();

    // SET NUMBER OF FeaturesProperly
    featureVectCreator->SetNumberOfFeaturesPerVoxel(1);
    featureVectCreator->SetGenerateResponsEnabled(false);

    cv::Mat output;

    // Image size
    ImageType::SizeType size;

    size = trainImages.at(0)->GetLargestPossibleRegion().GetSize();

    int dim = 1;

    for(int i = 0; i < VDimension; i++)
    {
        dim *= size[i];
    }

    // generate feature matrix from input images

    // generate train data
    for(int i = 0; i < trainImages.size(); i++)
    {
        featureVectCreator->SetPrimaryInput(trainImages.at(i));
        featureVectCreator->Update();
    }

    cv::Mat trainData = featureVectCreator->GetOuputCv();

    //for(int i = 0; i< (trainData.rows /dim) ; i++ )
    //{
    //  cv::Rect roi(0, i * dim,1,dim);
    //  cv::Mat image(trainData,roi);

    //  std::cout << image << std::endl;
    //}

    featureVectCreator->ClearData();

    //generate target data for train data
    for(int i = 0; i < targetImages.size(); i++)
    {
        featureVectCreator->SetPrimaryInput(targetImages.at(i));
        featureVectCreator->Update();
    }

    cv::Mat targetData = featureVectCreator->GetOuputCv();

    //for(int i = 0; i< (targetData.rows /dim) ; i++ )
    //{
    //  cv::Rect roi(0, i * dim,1,dim);
    //  cv::Mat image(targetData,roi);

    //  std::cout << image << std::endl;
    //}

    // generate test data
    featureVectCreator->ClearData();

    for(int i = 0; i < testImages.size(); i++)
    {
        featureVectCreator->SetPrimaryInput(testImages.at(i));
        featureVectCreator->Update();
    }

    cv::Mat testData = featureVectCreator->GetOuputCv();

    //for(int i = 0; i< (testData.rows /dim) ; i++ )
    //{
    //  cv::Rect roi(0, i * dim,1,dim);
    //  cv::Mat image(testData,roi);

    //  std::cout << image << std::endl;
    //}

    // Classification!!

    // KNN

    // number of NearestNeighbors

    CvKNearest knn( trainData, targetData, cv::Mat() ,false, k);

    cv::Mat neighbors(1,k, CV_32FC1);
    cv::Mat result(1,k,CV_32FC1);

    knn.find_nearest(testData,k,result,neighbors, cv::Mat());

    itk::ProcessObject::DataObjectPointerArray array = resultToImage->GetOutputs();

    if( itksys::SystemTools::FileIsDirectory(outputFolder.c_str() ) || !itksys::SystemTools::FileExists(outputFolder.c_str(),false) )
    {
        itksys::SystemTools::MakeDirectory(outputFolder.c_str());
    }

    // Write result images
    for(int i = 0; i< (result.rows /dim) ; i++ )
    {
        cv::Rect roi(0, i * dim,1,dim);
        cv::Mat image(result,roi);

        resultToImage->setOpenCVInput(image,size);

        std::cout << image << std::endl;

        resultToImage->Update();
        ImageTypePtr img = resultToImage->GetOutput(0);

        std::stringstream filename;
        filename << outputFolder << "result" << i << ".nrrd";
        writer->SetFileName(filename.str());
        writer->SetInput(img);
        writer->Update();
    }

    return 0;
}