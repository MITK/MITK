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

#include <mitkCoreObjectFactory.h>
#include "mitkImage.h"
#include <mitkLexicalCast.h>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

// ITK
#include <itkImageRegionIterator.h>

// MITK
#include <mitkIOUtil.h>

// Classification
#include <mitkCLUtil.h>
#include <mitkVigraRandomForestClassifier.h>

#include <QDir>
#include <QString>
#include <QStringList>


using namespace mitk;

/**
 *
 */
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  // required params
  parser.addArgument("inputdir", "i", mitkCommandLineParser::Directory, "Input Directory", "Contains input feature files.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("outputdir", "o", mitkCommandLineParser::Directory, "Output Directory", "Destination of output files.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("classmask", "m", mitkCommandLineParser::File, "Class mask image", "Contains several classes.", us::Any(), false, false, false, mitkCommandLineParser::Input);

  // optional params
  parser.addArgument("select", "s", mitkCommandLineParser::String, "Item selection", "Using Regular expression, seperated by space e.g.: '*.nrrd *.vtk *test*'",std::string("*.nrrd"),true);
  parser.addArgument("treecount", "tc", mitkCommandLineParser::Int, "Treecount", "Number of trees.",50,true);
  parser.addArgument("treedepth", "td", mitkCommandLineParser::Int, "Treedepth", "Maximal tree depth.",50,true);
  parser.addArgument("minsplitnodesize", "min", mitkCommandLineParser::Int, "Minimum split node size.", "Minimum split node size.",2,true);
  parser.addArgument("precision", "p", mitkCommandLineParser::Float, "Split precision.", "Precision.", mitk::eps,true);
  parser.addArgument("fraction", "f", mitkCommandLineParser::Float, "Fraction of samples per tree.", "Fraction of samples per tree.", 0.6f,true);
  parser.addArgument("replacment", "r", mitkCommandLineParser::Bool, "Sample with replacement.", "Sample with replacement.", true,true);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Random Forest Training");
  parser.setDescription("Vigra RF impl.");
  parser.setContributor("MBI");

  // Params parsing
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inputdir = us::any_cast<std::string>(parsedArgs["inputdir"]);
  std::string outputdir = us::any_cast<std::string>(parsedArgs["outputdir"]);
  std::string classmask = us::any_cast<std::string>(parsedArgs["classmask"]);

  int treecount = parsedArgs.count("treecount") ? us::any_cast<int>(parsedArgs["treecount"]) : 50;
  int treedepth = parsedArgs.count("treedepth") ? us::any_cast<int>(parsedArgs["treedepth"]) : 50;
  int minsplitnodesize = parsedArgs.count("minsplitnodesize") ? us::any_cast<int>(parsedArgs["minsplitnodesize"]) : 2;
  float precision = parsedArgs.count("precision") ? us::any_cast<float>(parsedArgs["precision"]) : mitk::eps;
  float fraction = parsedArgs.count("fraction") ? us::any_cast<float>(parsedArgs["fraction"]) : 0.6;
  bool withreplacement = parsedArgs.count("replacment") ? us::any_cast<float>(parsedArgs["replacment"]) : true;
  std::string filt_select =/* parsedArgs.count("select") ? us::any_cast<std::string>(parsedArgs["select"]) :*/ "*.nrrd";

  QString filter(filt_select.c_str());

  // **** in principle repeat this block to create a feature matrix X_all for all subjects (in dir)
  // Get nrrd filepath
  QDir dir(inputdir.c_str());
  auto strl = dir.entryList(filter.split(" "),QDir::Files);

  // load class mask
  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(classmask);
  unsigned int num_samples = 0;
  mitk::CLUtil::CountVoxel(mask,num_samples);

  // initialize featurematrix [num_samples, num_featureimages]
  Eigen::MatrixXd X(num_samples, strl.size());

  for(int i = 0 ; i < strl.size(); i++)
  {
    // load feature image
    mitk::Image::Pointer img = mitk::IOUtil::Load<mitk::Image>(inputdir + strl[i].toStdString());
    // transfom it into a [num_samples, 1] vector depending on the classmask
    Eigen::MatrixXd _x = mitk::CLUtil::Transform<double>(img,mask);
    // replace i-th (empty) col with feature vector in _x
    X.block(0,i,num_samples,1) = _x;
  }
  // ****

  // transform classmask into the label-vector [num_samples, 1]
  Eigen::MatrixXi Y = mitk::CLUtil::Transform<int>(mask,mask);

  mitk::VigraRandomForestClassifier::Pointer classifier = mitk::VigraRandomForestClassifier::New();
  classifier->SetTreeCount(treecount);
  classifier->SetMaximumTreeDepth(treedepth);
  classifier->SetMinimumSplitNodeSize(minsplitnodesize);
  classifier->SetPrecision(precision);
  classifier->SetSamplesPerTree(fraction);
  classifier->UseSampleWithReplacement(withreplacement);

  classifier->PrintParameter();
  classifier->Train(X,Y);

  MITK_INFO << classifier->IsEmpty();

  // no metainformations are saved currently
  // only the raw vigra rf data
  mitk::IOUtil::Save(classifier, outputdir + "RandomForest.hdf5");

  Eigen::MatrixXi Y_pred = classifier->Predict(X);
  Eigen::MatrixXd Probs = classifier->GetPointWiseProbabilities();

  MITK_INFO << Y_pred.rows() << " " << Y_pred.cols();
  MITK_INFO << Probs.rows() << " " << Probs.cols();

  //    mitk::Image::Pointer prediction = mitk::CLUtil::Transform<int>(Y_pred,mask);
  mitk::Image::Pointer probs_1 = mitk::CLUtil::Transform<double>(Probs.col(0),mask);
  mitk::Image::Pointer probs_2 = mitk::CLUtil::Transform<double>(Probs.col(1),mask);
  mitk::Image::Pointer probs_3 = mitk::CLUtil::Transform<double>(Probs.col(2),mask);

  mitk::IOUtil::Save(probs_1, outputdir + "probs_1.nrrd");
  mitk::IOUtil::Save(probs_2, outputdir + "probs_2.nrrd");
  mitk::IOUtil::Save(probs_3, outputdir + "probs_3.nrrd");
  //    mitk::IOUtil::Save(probs_2, outputdir + "test.nrrd");

  return EXIT_SUCCESS;
}
