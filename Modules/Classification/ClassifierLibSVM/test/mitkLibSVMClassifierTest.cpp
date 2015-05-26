/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include "mitkIOUtil.h"
#include "itkArray2D.h"


//#include <vigra/random_forest_hdf5_impex.hxx>
#include <mitkLibSVMClassifier.h>
#include <itkLabelSampler.h>
#include <mitkImageCast.h>
#include <mitkStandaloneDataStorage.h>
#include <itkCSVArray2DFileReader.h>
#include <itkCSVArray2DDataObject.h>

//#include <boost/algorithm/string.hpp>

class mitkLibSVMClassifierTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLibSVMClassifierTestSuite  );

  MITK_TEST(TrainSVMClassifier);
//  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:

 // typedef mitk::ImageToEigenTransform::VectorType EigenVectorType;
//  static std::string GetTestDataFilePath(const std::string& testData)
//  {
//    if (itksys::SystemTools::FileIsFullPath(testData.c_str())) return testData;
//    return std::string(MITK_MBI_DATA_DIR) + "/" + testData;
//  }

//  mitk::DecisionForest::Pointer m_EmptyDecisionForest, m_LoadedDecisionForest;
//  mitk::DataCollection::Pointer m_TrainDatacollection;
//  mitk::DataCollection::Pointer m_TestDatacollection;
  mitk::Image::Pointer inputImage;
  mitk::Image::Pointer classMask;
  mitk::Image::Pointer F1;
  mitk::Image::Pointer F2;
  mitk::Image::Pointer F3;
  Eigen::MatrixXd Matrix_X;
  Eigen::MatrixXi Matrix_y;
  Eigen::MatrixXd X_predict;

  mitk::LibSVMClassifier::Pointer classifier;

public:



  void MergeLabels(std::map<unsigned int, unsigned int> map, mitk::Image::Pointer img)
  {

    itk::Image<unsigned int, 3>::Pointer classImage;
    mitk::CastToItkImage(img, classImage);
    auto it = itk::ImageRegionIterator<itk::Image<unsigned int, 3> >(classImage,classImage->GetLargestPossibleRegion());
    it.GoToBegin();

    while(!it.IsAtEnd())
    {
      if(map.find(it.Value())!=map.end())
        it.Set( map[it.Value()] );
      ++it;
    }

    mitk::CastToMitkImage(classImage,img);

  }

  /* Ließt einen File und somit die Trainings- und Testdatensätze ein und
     konvertiert den Inhalt des Files in eine 2dim Matrix.
  */
  template<typename T>
  Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> convertCSVToMatrix(const std::string &path, char delimiter)
  {
    itk::CSVArray2DFileReader<T>::Pointer fr = itk::CSVArray2DFileReader<T>::New();
    fr->SetFileName(path);
    fr->SetFieldDelimiterCharacter(delimiter);
    fr->HasColumnHeadersOff();
    fr->HasRowHeadersOff();
    fr->Parse();
    try{
        fr->Update();
    }catch(itk::ExceptionObject& ex){
        cout << "Exception caught!" << std::endl;
        cout << ex << std::endl;
    }

    itk::CSVArray2DDataObject<T>::Pointer p = fr->GetOutput();
    Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> out_mat;
    unsigned int r = p->GetMatrix().rows();
    unsigned int c = p->GetMatrix().cols();
    out_mat = Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic>(r,c);

    for(int row = 0; row < r; row++){
        for(int col = 0; col < c; col++){
        out_mat(row,col) =  p->GetData(row,col);
        }
    }

    return out_mat;
  }

  void setUp(void)
  {

    /*  itk::CSVArray2DFileReader<double>::Pointer fr = itk::CSVArray2DFileReader<double>::New();
    itk::CSVArray2DFileReader<int>::Pointer fr2 = itk::CSVArray2DFileReader<int>::New();

    fr->SetFileName("C:/Users/tschlats/Desktop/data.csv");
    fr->SetFieldDelimiterCharacter(';');
    fr->HasColumnHeadersOff();
    fr->HasRowHeadersOff();
    fr->Parse();
    try{
        fr->Update();
    }catch(itk::ExceptionObject& ex){
    cout << "Exception caught!" << std::endl;
    cout << ex << std::endl;
    }

    fr2->SetFileName("C:/Users/tschlats/Desktop/dataY.csv");
    fr2->SetFieldDelimiterCharacter(' ');
    fr2->HasColumnHeadersOff();
    fr2->HasRowHeadersOff();
    fr2->Parse();
    try{
        fr2->Update();
    }catch(itk::ExceptionObject& ex){
    cout << "Exception caught!" << std::endl;
    cout << ex << std::endl;
    }

    itk::CSVArray2DDataObject<double>::ConstPointer p = fr->GetOutput();
    itk::CSVArray2DDataObject<int>::ConstPointer p2 = fr2->GetOutput();
    Matrix_X = Eigen::MatrixXd(683,10); // Trainingsdatenmatrix
    Matrix_y = Eigen::MatrixXi(683,1);  // Testdatenmatrix   */


    Eigen::Matrix<double,Eigen::Dynamic,Eigen::Dynamic> trainingMatrix = convertCSVToMatrix<double>("C:/Users/tschlats/Desktop/data.csv",';');
    Eigen::Matrix<int,Eigen::Dynamic,Eigen::Dynamic> testMatrix = convertCSVToMatrix<int>("C:/Users/tschlats/Desktop/dataY.csv",';');
    cout << trainingMatrix << endl;


    /*  // liefert double Werte zurück da MatrixXd
    for(int row = 0; row < Matrix_X.rows() ; row++){
        for(int col = 0; col < Matrix_X.cols() ; col++){
        Matrix_X(row,col) =  p->GetData(row,col);
        }
    }

    // liefert int Werte zurück da MatrixXi
    for(int i = 0; i < Matrix_y.rows(); i++){
    Matrix_y(i,0) = p2->GetData(i,0);
    }   */


    //  cout << Matrix_X << endl;
    //  cout << Matrix_y << endl;



    /*
    // Load Image Data
    inputImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/InputImage.nrrd"));
    classMask = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/ClassMask.nrrd"));
    F1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Classification/Classifier/F1.nrrd"));

    std::map<unsigned int, unsigned int> map;
    map[0] = 0;
    map[1] = 1;
    map[2] = 1;
    map[3] = 2;
    MergeLabels(map,classMask);

    itk::Image<unsigned char,3>::Pointer itkClass;
    mitk::CastToItkImage(classMask,itkClass);

    itk::LabelSampler<itk::Image<unsigned char,3> >::Pointer filter = itk::LabelSampler<itk::Image<unsigned char,3> >::New();
    filter->SetInput(itkClass);
    filter->SetAcceptRate(0.01);
    filter->Update();

    mitk::Image::Pointer sampledClassMask;
    mitk::CastToMitkImage(filter->GetOutput(), sampledClassMask); */

//    // Initialize X
//    Eigen::MatrixXd vec = mitk::ImageToEigenTransform::transform(inputImage,sampledClassMask);
//    unsigned int n_features = 4; // F1,F2,F3 and inputImage
//    unsigned int n_samples = vec.rows();

//    X = Eigen::MatrixXd(n_samples,n_features);
//    X.col(0) = vec;
//    X.col(1) = mitk::ImageToEigenTransform::transform(F1,sampledClassMask);

//    y = mitk::ImageToEigenTransform::transform(classMask,sampledClassMask);

//    vec = mitk::ImageToEigenTransform::transform(inputImage,classMask);
//    n_samples = vec.rows();

//    X_predict = Eigen::MatrixXd(n_samples,n_features);
//    X_predict.col(0) = vec;
//    X_predict.col(1) = mitk::ImageToEigenTransform::transform(F1,classMask);

//    MITK_INFO << "Shape of X [" << X.rows() << " " << X.cols() << "]";
//    MITK_INFO << "Shape of X_predict [" << X_predict.rows() << " " << X_predict.cols() << "]";
//    MITK_INFO << "Shape of Y [" << y.rows() << "]";

//    classifier = mitk::LibSVMClassifier::New();

  }

  /* Task: Läd ein 3dim-Bild und zählt alle Voxel, die > 0 sind und gibt das
     Ergebnis auf der Konsole aus.
  */
  void TrainSVMClassifier()
  {
    /* mitk::Image::Pointer mitkfirstImage;
    typedef itk::Image<unsigned int,3> ImageType;
    itk::Image<unsigned int,3>::Pointer itkfirstImage; // 3dim Bild.

    mitkfirstImage = mitk::IOUtil::LoadImage("c:/bachelor/bin/MITK-Data/Pic3D.nrrd");
    mitk::CastToItkImage(mitkfirstImage,itkfirstImage);

    // a) mit dem Iterator

    // auto = autom. Datentyp hier ist auto = itk::ImageRegionIterator<itk::Image<unsigned int, 3>
    //                                                                Bild ,        Region
    // allg: itk::ImageRegionIterator<ImageType> imageIterator(image,region);
    auto it = itk::ImageRegionIterator<itk::Image<unsigned int, 3> >(itkfirstImage,itkfirstImage->GetLargestPossibleRegion());
    it.GoToBegin();
    unsigned int greyValues = 0;

    while(!it.IsAtEnd())
    {
    greyValues += it.Get();   // unsigned = ohne Vorzeichen.

      ++it;
    }
    MITK_INFO << "Loading of the Image was successfully with iterator" << greyValues;

    // GetImageDimension ansehen !!

    // b) mit dreifachgeschachtelter FOR-Schleife
    unsigned int a =  0;
//  cout << itkfirstImage->GetLargestPossibleRegion().GetSize()[0] << endl; // 256 xDim
//  cout << itkfirstImage->GetLargestPossibleRegion().GetSize()[1] << endl; // 256 yDim
//  cout << itkfirstImage->GetLargestPossibleRegion().GetSize()[2] << endl; //49  zDim

    for(int z = 0; z < itkfirstImage->GetLargestPossibleRegion().GetSize()[2]   ; z++){
        for(int y = 0; y < itkfirstImage->GetLargestPossibleRegion().GetSize()[1]  ; y++){
            for(int x = 0; x < itkfirstImage->GetLargestPossibleRegion().GetSize()[0]  ; x++){
                // do something
                    ImageType::IndexType pixelIndex;
                    pixelIndex[0] = x;
                    pixelIndex[1] = y;
                    pixelIndex[2] = z;
                    a += itkfirstImage->GetPixel(pixelIndex);
    }
        }
            }


    MITK_INFO << "Loading of the Image was successfully with For Loops" << a; */

    classifier->Train(Matrix_X,Matrix_y);
     Eigen::MatrixXi classes = classifier->Predict(X_predict);
    cout << classes << endl;

   // mitk::Image::Pointer img = mitk::EigenToImageTransform::transform(classes, classMask);
   // mitk::IOUtil::Save(img,"/Users/jc/prediction.nrrd");
  }

  void TestThreadedDecisionForest()
  {
//    m_LoadedDecisionForest->SetCollection(m_TrainDatacollection);
//    m_LoadedDecisionForest->SetModalities(m_Selected_items);
//    m_LoadedDecisionForest->SetResultProb("ResultProb");
//    m_LoadedDecisionForest->TestThreaded();

    //    mitk::DataCollection::Pointer res_col = dynamic_cast<mitk::DataCollection *>(dynamic_cast<mitk::DataCollection *>(m_TrainDatacollection->GetData("Test-Study").GetPointer())->GetData("Test-Subject").GetPointer());
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultMask"),"/Users/jc/res_mask.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-0"),"/Users/jc/res_prob_0.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-1"),"/Users/jc/res_prob_1.nrrd");
    //    mitk::IOUtil::Save(res_col->GetMitkImage("ResultProb_Class-2"),"/Users/jc/res_prob_2.nrrd");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLibSVMClassifier)


