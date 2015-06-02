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

  typedef Eigen::Matrix<double ,Eigen::Dynamic,Eigen::Dynamic> MatrixDoubleType;
  typedef Eigen::Matrix<int, Eigen::Dynamic,Eigen::Dynamic> MatrixIntType;


  Eigen::MatrixXd m_TrainingMatrixX;
  Eigen::MatrixXi m_TrainingLabelMatrixY;
  Eigen::MatrixXd m_TestXPredict;
  Eigen::MatrixXi m_TestYPredict;


  mitk::LibSVMClassifier::Pointer classifier;

public:

  /* Lieﬂt einen File und somit die Trainings- und Testdatens‰tze ein und
     konvertiert den Inhalt des Files in eine 2dim Matrix.
  */

   template<typename T>
   std::pair<Eigen::Matrix<T ,Eigen::Dynamic,Eigen::Dynamic>,Eigen::Matrix<T ,Eigen::Dynamic,Eigen::Dynamic> >convertCSVToMatrix(const std::string &path, char delimiter,double range, bool isXMatrix)
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
         unsigned int maxrowrange = p->GetMatrix().rows();
         unsigned int c = p->GetMatrix().cols();
         unsigned int percentRange = (unsigned int)(maxrowrange*range);

    if(isXMatrix == true){

      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> trainMatrixX(percentRange,c);
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> testMatrixXPredict(maxrowrange,c);

        for(int row = 0; row < percentRange; row++){
          for(int col = 0; col < c; col++){
             trainMatrixX(row,col) =  p->GetData(row,col);
          }
        }

         for(int row = percentRange; row < maxrowrange; row++){
            for(int col = 0; col < c; col++){
                testMatrixXPredict(row,col) =  p->GetData(row,col);
        }
    }

    return std::make_pair(trainMatrixX,testMatrixXPredict);
    }
    else if(isXMatrix == false){

      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> trainLabelMatrixY(percentRange,c);
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> testMatrixYPredict(maxrowrange,c);

         for(int row = 0; row < percentRange; row++){
           for(int col = 0; col < c; col++){
             trainLabelMatrixY(row,col) =  p->GetData(row,col);
          }
        }

          for(int row = percentRange; row < maxrowrange; row++){
            for(int col = 0; col < c; col++){
                testMatrixYPredict(row,col) =  p->GetData(row,col);
        }
    }

    return std::make_pair(trainLabelMatrixY,testMatrixYPredict);
    }
  }

  void setUp(void)
  {

    std::pair<MatrixDoubleType,MatrixDoubleType> matrixDouble;
    matrixDouble = convertCSVToMatrix<double>("C:/Users/tschlats/Desktop/data.csv",';',0.5,true);
    m_TrainingMatrixX = matrixDouble.first;
    m_TestXPredict = matrixDouble.second;

    std::pair<MatrixIntType,MatrixIntType> matrixInt;
    matrixInt = convertCSVToMatrix<int>("C:/Users/tschlats/Desktop/dataY.csv",';',0.5,false);
    m_TrainingLabelMatrixY = matrixInt.first;
    m_TestYPredict = matrixInt.second;
    classifier = mitk::LibSVMClassifier::New();

  }

  void TrainSVMClassifier()
  {

    MITK_INFO << "start SVM-Training";

    classifier->Train(m_TrainingMatrixX,m_TrainingLabelMatrixY);
  /*  Eigen::MatrixXi classes = classifier->Predict(test_X_predict);
    cout << classes << endl; */

   // mitk::Image::Pointer img = mitk::EigenToImageTransform::transform(classes, classMask);
   // mitk::IOUtil::Save(img,"/Users/jc/prediction.nrrd");
  }

  void TestThreadedDecisionForest()
  {
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkLibSVMClassifier)


