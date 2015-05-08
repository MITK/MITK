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
#include <boost/lexical_cast.hpp>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

// MITK
#include <mitkIOUtil.h>

// Classification
#include <mitkCLUtil.h>
#include <mitkVigraRandomForestClassifier.h>

#include <QDir>
#include <QString>
#include <QstringList>


using namespace mitk;

/**
 *
 */
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;
    parser.setArgumentPrefix("--", "-");

    // Params
    parser.addArgument("inputdir", "i", mitkCommandLineParser::InputDirectory, "Input Directory", "Contains input feature files.", us::Any(), false);
    parser.addArgument("outputdir", "o", mitkCommandLineParser::OutputDirectory, "Output Directory", "Destination of output files.", us::Any(), false);
    parser.addArgument("classmask", "m", mitkCommandLineParser::InputFile, "Class mask image", "Contains several classes.", us::Any(), false);

    // Miniapp Infos
    parser.setCategory("Classification Tools");
    parser.setTitle("Random Forest");
    parser.setDescription("Vigra RF impl.");
    parser.setContributor("MBI");

    // Params parsing
    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    std::string inputdir = us::any_cast<string>(parsedArgs["inputdir"]);
    std::string outputdir = us::any_cast<string>(parsedArgs["outputdir"]);
    std::string classmask = us::any_cast<string>(parsedArgs["classmask"]);

    // Get nrrd files
    QString filter = "*.nrrd";
    QDir dir(inputdir.c_str());
    auto strl = dir.entryList(filter.split(" "),QDir::Files);

    // load class mask
    mitk::Image::Pointer mask = mitk::IOUtil::LoadImage(classmask);
    unsigned int num_samples = 0;
    AccessFixedDimensionByItk_1(mask,mitk::CLUtil::CountLabledVoxels, 3, num_samples)

    // initialize featurematrix [num_samples, num_featureimages]
    Eigen::MatrixXd X(num_samples, strl.size());

    for(int i = 0 ; i < strl.size(); i++)
    {
      // load feature image
      mitk::Image::Pointer img = mitk::IOUtil::LoadImage(inputdir + strl[i].toStdString());
      // transfom it into a [num_samples, 1] vector depending on the classmask
      Eigen::MatrixXd _x = mitk::CLUtil::Transform<double>(img,mask);
      // replace i-th col with _x
      X.block(0,i,num_samples,1) = _x;
    }

    // transform classmask into the label-vector [num_samples, 1]
    Eigen::MatrixXi Y = mitk::CLUtil::Transform<int>(mask,mask);

    mitk::VigraRandomForestClassifier::Pointer classifier = mitk::VigraRandomForestClassifier::New();
    classifier->PrintParameter(std::cout);
    classifier->Train(X,Y);

    MITK_INFO << classifier->IsEmpty();

    // no metainformations are saved currently
    // only the raw vigra rf data
    mitk::IOUtil::Save(classifier, outputdir + "RandomForest.hdf5");

//    Eigen::MatrixXi Y_pred = classifier->Predict(X);

//    mitk::Image::Pointer prediction = mitk::CLUtil::Transform(Y_pred,mask);
//    mitk::IOUtil::Save(prediction, outputdir + "test.nrrd");

//    int threshold = 0;
//    if (parsedArgs.count("b0Threshold"))
//        threshold = us::any_cast<int>(parsedArgs["b0Threshold"]);

//    int shOrder = 4;
//    if (parsedArgs.count("shOrder"))
//        shOrder = us::any_cast<int>(parsedArgs["shOrder"]);

//    float lambda = 0.006;
//    if (parsedArgs.count("lambda"))
//        lambda = us::any_cast<float>(parsedArgs["lambda"]);

//    int normalization = 0;
//    if (parsedArgs.count("csa") && us::any_cast<bool>(parsedArgs["csa"]))
//        normalization = 6;

//    bool outCoeffs = false;
//    if (parsedArgs.count("outputCoeffs"))
//        outCoeffs = us::any_cast<bool>(parsedArgs["outputCoeffs"]);

//    bool mrTrix = false;
//    if (parsedArgs.count("mrtrix"))
//        mrTrix = us::any_cast<bool>(parsedArgs["mrtrix"]);

  // Do something
    return EXIT_SUCCESS;
}
