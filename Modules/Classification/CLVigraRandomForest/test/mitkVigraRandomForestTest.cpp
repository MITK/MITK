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

#include <itkCSVArray2DFileReader.h>
#include <itkCSVArray2DDataObject.h>
#include <mitkVigraRandomForestClassifier.h>
#include <itkLabelSampler.h>
#include <itkAddImageFilter.h>
#include <mitkImageCast.h>
#include <mitkStandaloneDataStorage.h>

class mitkVigraRandomForestTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVigraRandomForestTestSuite  );
  //  MITK_TEST(Load_RandomForestBaseDataUsingIOUtil_shouldReturnTrue);
  //  MITK_TEST(Save_RandomForestBaseDataUsingIOUtil_shouldReturnTrue);

  //  MITK_TEST(LoadWithMitkOptions_RandomForestBaseDataUsingIOUtil_shouldReturnTrue);
  //  MITK_TEST(SaveWithMitkOptions_RandomForestBaseDataUsingIOUtil_shouldReturnTrue);

  MITK_TEST(TrainThreadedDecisionForest_MatlabDataSet_shouldReturnTrue);
  MITK_TEST(PredictWeightedDecisionForest_SetWeightsToZero_shouldReturnTrue);
  MITK_TEST(TrainThreadedDecisionForest_BreastCancerDataSet_shouldReturnTrue);
  CPPUNIT_TEST_SUITE_END();

private:

  typedef Eigen::Matrix<double ,Eigen::Dynamic,Eigen::Dynamic> MatrixDoubleType;
  typedef Eigen::Matrix<int, Eigen::Dynamic,Eigen::Dynamic> MatrixIntType;

  std::pair<MatrixDoubleType, MatrixDoubleType> FeatureData_Cancer;
  std::pair<MatrixIntType, MatrixIntType> LabelData_Cancer;

  std::pair<MatrixDoubleType, MatrixDoubleType> FeatureData_Matlab;
  std::pair<MatrixIntType, MatrixIntType> LabelData_Matlab;

  mitk::VigraRandomForestClassifier::Pointer classifier;

public:

  // ------------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------------


  void setUp()
  {
    FeatureData_Cancer = convertCSVToMatrix<double>(GetTestDataFilePath("Classification/FeaturematrixBreastcancer.csv"),';',0.5,true);
    LabelData_Cancer = convertCSVToMatrix<int>(GetTestDataFilePath("Classification/LabelmatrixBreastcancer.csv"),';',0.5,false);
    FeatureData_Matlab = convertCSVToMatrix<double>(GetTestDataFilePath("Classification/FeaturematrixMatlab.csv"),';',0.5,true);
    LabelData_Matlab = convertCSVToMatrix<int>(GetTestDataFilePath("Classification/LabelmatrixMatlab.csv"),';',0.5,false);
    classifier = mitk::VigraRandomForestClassifier::New();
  }

  void tearDown()
  {
    classifier = nullptr;
  }

  // ------------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------------
  /*
  Train the classifier with an exampledataset of mattlab.
  Note: The included data are gaußan normaldistributed.
  */
  void TrainThreadedDecisionForest_MatlabDataSet_shouldReturnTrue()
  {

    auto & Features_Training = FeatureData_Matlab.first;
    auto & Labels_Training = LabelData_Matlab.first;

    auto & Features_Testing = FeatureData_Matlab.second;
    auto & Labels_Testing = LabelData_Matlab.second;

    /* Train the classifier, by giving trainingdataset for the labels and features.
    The result in an colunmvector of the labels.*/
    classifier->Train(Features_Training,Labels_Training);
    Eigen::MatrixXi classes = classifier->Predict(Features_Testing);

    /* Testing the matching between the calculated colunmvector and the result of the RandomForest */
    unsigned int testmatrix_rows = classes.rows();

    unsigned int correctly_classified_rows = 0;
    for(unsigned int i= 0; i < testmatrix_rows; i++){
      if(classes(i,0) == Labels_Testing(i,0)){
        correctly_classified_rows++;
      }
    }

    MITK_TEST_CONDITION(correctly_classified_rows == testmatrix_rows, "Matlab Data correctly classified");
  }

  // ------------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------------
  /*
  Train the classifier with the dataset of breastcancer patients from the
  LibSVM Libary
  */
  void TrainThreadedDecisionForest_BreastCancerDataSet_shouldReturnTrue()
  {

    auto & Features_Training = FeatureData_Cancer.first;
    auto & Features_Testing = FeatureData_Cancer.second;
    auto & Labels_Training = LabelData_Cancer.first;
    auto & Labels_Testing = LabelData_Cancer.second;


    /* Train the classifier, by giving trainingdataset for the labels and features.
    The result in an colunmvector of the labels.*/
    classifier->Train(Features_Training,Labels_Training);
    Eigen::MatrixXi classes = classifier->Predict(Features_Testing);

    /* Testing the matching between the calculated colunmvector and the result of the RandomForest */
    unsigned int maxrows = classes.rows();

    bool isYPredictVector = false;
    int count = 0;

    for(unsigned int i= 0; i < maxrows; i++){
      if(classes(i,0) == Labels_Testing(i,0)){
        isYPredictVector = true;
        count++;
      }
    }
    MITK_TEST_CONDITION(isIntervall<int>(Labels_Testing,classes,98,99),"Testvalue of cancer data set is in range.");
  }

  // ------------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------------

  void PredictWeightedDecisionForest_SetWeightsToZero_shouldReturnTrue()
  {

    auto & Features_Training = FeatureData_Matlab.first;
    auto & Features_Testing = FeatureData_Matlab.second;
    auto & Labels_Training = LabelData_Matlab.first;
//    auto & Labels_Testing = LabelData_Matlab.second;

    classifier->Train(Features_Training,Labels_Training);

    // get weights type resize it and set all weights to zero
    auto weights = classifier->GetTreeWeights();
    weights.resize(classifier->GetRandomForest().tree_count(),1);
    weights.fill(0);

    classifier->SetTreeWeights(weights);

    // if all wieghts zero the missclassification rate mus be high
    Eigen::MatrixXi classes = classifier->PredictWeighted(Features_Testing);

    /* Testing the matching between the calculated colunmvector and the result of the RandomForest */
    unsigned int maxrows = classes.rows();
    unsigned int count = 0;

    // check if all predictions are of class 1
    for(unsigned int i= 0; i < maxrows; i++)
      if(classes(i,0) == 1)
        count++;

    MITK_TEST_CONDITION( (count == maxrows) ,"Weighted prediction - weights applied (all weights = 0).");
  }


  // ------------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------------
  /*Reading an file, which includes the trainingdataset and the testdataset, and convert the
  content of the file into an 2dim matrixpair.
  There are an delimiter, which separates the matrix into an trainingmatrix and testmatrix */
  template<typename T>
  std::pair<Eigen::Matrix<T ,Eigen::Dynamic,Eigen::Dynamic>,Eigen::Matrix<T ,Eigen::Dynamic,Eigen::Dynamic> >convertCSVToMatrix(const std::string &path, char delimiter,double range, bool isXMatrix)
  {
    typename itk::CSVArray2DFileReader<T>::Pointer fr = itk::CSVArray2DFileReader<T>::New();
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

    typename itk::CSVArray2DDataObject<T>::Pointer p = fr->GetOutput();
    unsigned int maxrowrange = p->GetMatrix().rows();
    unsigned int c = p->GetMatrix().cols();
    unsigned int percentRange = (unsigned int)(maxrowrange*range);

    if(isXMatrix == true){
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> trainMatrixX(percentRange,c);
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> testMatrixXPredict(maxrowrange-percentRange,c);

      for(unsigned int row = 0; row < percentRange; row++){
        for(unsigned int col = 0; col < c; col++){
          trainMatrixX(row,col) =  p->GetData(row,col);
        }
      }

      for(unsigned int row = percentRange; row < maxrowrange; row++){
        for(unsigned int col = 0; col < c; col++){
          testMatrixXPredict(row-percentRange,col) =  p->GetData(row,col);
        }
      }

      return std::make_pair(trainMatrixX,testMatrixXPredict);
    }
    else{
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> trainLabelMatrixY(percentRange,c);
      Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> testMatrixYPredict(maxrowrange-percentRange,c);

      for(unsigned int row = 0; row < percentRange; row++){
        for(unsigned int col = 0; col < c; col++){
          trainLabelMatrixY(row,col) =  p->GetData(row,col);
        }
      }

      for(unsigned int row = percentRange; row < maxrowrange; row++){
        for(unsigned int col = 0; col < c; col++){
          testMatrixYPredict(row-percentRange,col) =  p->GetData(row,col);
        }
      }
      return std::make_pair(trainLabelMatrixY,testMatrixYPredict);
    }

  }

  /*
  Reading an csv-data and transfer the included datas into an matrix.
  */
  template<typename T>
  Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> readCsvData(const std::string &path, char delimiter)
  {
    typename itk::CSVArray2DFileReader<T>::Pointer fr = itk::CSVArray2DFileReader<T>::New();
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

    typename itk::CSVArray2DDataObject<T>::Pointer p = fr->GetOutput();
    unsigned int maxrowrange = p->GetMatrix().rows();
    unsigned int maxcols = p->GetMatrix().cols();
    Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> matrix(maxrowrange,maxcols);

    for(unsigned int rows = 0; rows < maxrowrange; rows++){
      for(unsigned int cols = 0; cols < maxcols; cols++ ){
        matrix(rows,cols) = p->GetData(rows,cols);
      }
    }

    return matrix;
  }

  /*
  Write the content of the array into an own csv-data in the following sequence:
  root.csv:   1    2    3    0   0    4
  writen.csv:     1  1:2  2:3  3:0 4:0  5:4
  */
  template<typename T>
  void writeMatrixToCsv(Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> paramMatrix,const std::string &path)
  {
    std::ofstream outputstream (path,std::ofstream::out);   // 682

    if(outputstream.is_open()){
      for(int i = 0; i < paramMatrix.rows(); i++){
        outputstream << paramMatrix(i,0);
        for(int j = 1; j < 11; j++){
          outputstream  << " " << j << ":" << paramMatrix(i,j);
        }
        outputstream << endl;
      }
      outputstream.close();
    }
    else{
      cout << "Unable to write into CSV" << endl;
    }
  }

  // Method for intervalltesting
  template<typename T>
  bool isIntervall(Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> expected, Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> actual, double lowrange, double toprange)
  {
    bool isInIntervall = false;
    int count = 0;
    unsigned int rowRange = expected.rows();
    unsigned int colRange = expected.cols();
    for(unsigned int i = 0; i < rowRange; i++){
      for(unsigned int j = 0; j < colRange; j++){
        if(expected(i,j) == actual(i,j)){
          count++;
        }
      }

      double valueOfMatch = 100*count/(double)(rowRange);
      if((lowrange <= valueOfMatch) && (toprange >= valueOfMatch)){
        isInIntervall = true;
      }
    }
    return isInIntervall;
  }


};

MITK_TEST_SUITE_REGISTRATION(mitkVigraRandomForest)
