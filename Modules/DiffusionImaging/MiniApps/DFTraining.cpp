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

#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

#include <mitkFiberBundle.h>
#include <itkDwiNormilzationFilter.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <vtkCell.h>
#include <itkOrientationDistributionFunction.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

#include <vigra/random_forest.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

#define _USE_MATH_DEFINES
#include <math.h>

const int numOdfSamples = 200;  // ODF is sampled in 200 directions but actuyll only 100 are used (symmetric)
typedef itk::Image< itk::Vector< float, numOdfSamples > , 3 > SampledShImageType;

void TrainForest( vigra::RandomForest<int> &rf, vigra::MultiArray<2, double> &labelData, vigra::MultiArray<2, double> &featureData, int numTrees, int max_tree_depth, double sample_fraction )
{
    MITK_INFO << "Maximum tree depths: " << max_tree_depth;
    MITK_INFO << "Sample fraction per tree: " << sample_fraction;
    MITK_INFO << "Number of trees: " << numTrees;
    vigra::rf::visitors::OOB_Error oob_v;

//    rf.set_options().use_stratification(vigra::RF_NONE); // How the data should be made equal
//    rf.set_options().sample_with_replacement(true); // if sampled with replacement or not
//    rf.set_options().samples_per_tree(sample_fraction); // Fraction of samples that are used to train a tree
//    rf.set_options().tree_count(1); // Number of trees that are calculated;
//    rf.set_options().min_split_node_size(5); // Minimum number of datapoints that must be in a node
//    rf.ext_param_.max_tree_depth = max_tree_depth;
//    //    rf.set_options().features_per_node(10);
//    rf.learn(featureData, labelData, vigra::rf::visitors::create_visitor(oob_v));

    std::vector< vigra::RandomForest<int> > trees;
    int count = 0;
#pragma omp parallel for
    for (int i = 0; i < numTrees; ++i)
    {
        vigra::RandomForest<int> lrf;
        vigra::rf::visitors::OOB_Error loob_v;

        lrf.set_options().use_stratification(vigra::RF_NONE); // How the data should be made equal
        lrf.set_options().sample_with_replacement(true); // if sampled with replacement or not
        lrf.set_options().samples_per_tree(sample_fraction); // Fraction of samples that are used to train a tree
        lrf.set_options().tree_count(1); // Number of trees that are calculated;
        lrf.set_options().min_split_node_size(5); // Minimum number of datapoints that must be in a node
        lrf.ext_param_.max_tree_depth = max_tree_depth;
        //        lrf.set_options().features_per_node(10);

        lrf.learn(featureData, labelData);//, vigra::rf::visitors::create_visitor(loob_v));
#pragma omp critical
        {
            count++;
            MITK_INFO << "Tree " << count << " finished training.";
            trees.push_back(lrf);
            //rf.trees_.push_back(lrf.trees_[0]);
        }
    }

    for (int i = 1; i < numTrees; ++i)
        trees.at(0).trees_.push_back(trees.at(i).trees_[0]);

    rf = trees.at(0);
    rf.options_.tree_count_ = numTrees;
    MITK_INFO << "Training finsihed";
    //MITK_INFO << "The out-of-bag error is: " << oob_v.oob_breiman << std::endl;
}

SampledShImageType::PixelType GetImageValues(itk::Point<float, 3> itkP, SampledShImageType::Pointer image)
{
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    image->TransformPhysicalPointToIndex(itkP, idx);
    image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    SampledShImageType::PixelType pix; pix.Fill(0.0);
    if ( image->GetLargestPossibleRegion().IsInside(idx) )
        pix = image->GetPixel(idx);
    else
        return pix;

    double frac_x = cIdx[0] - idx[0];
    double frac_y = cIdx[1] - idx[1];
    double frac_z = cIdx[2] - idx[2];
    if (frac_x<0)
    {
        idx[0] -= 1;
        frac_x += 1;
    }
    if (frac_y<0)
    {
        idx[1] -= 1;
        frac_y += 1;
    }
    if (frac_z<0)
    {
        idx[2] -= 1;
        frac_z += 1;
    }
    frac_x = 1-frac_x;
    frac_y = 1-frac_y;
    frac_z = 1-frac_z;

    // int coordinates inside image?
    if (idx[0] >= 0 && idx[0] < image->GetLargestPossibleRegion().GetSize(0)-1 &&
            idx[1] >= 0 && idx[1] < image->GetLargestPossibleRegion().GetSize(1)-1 &&
            idx[2] >= 0 && idx[2] < image->GetLargestPossibleRegion().GetSize(2)-1)
    {
        vnl_vector_fixed<double, 8> interpWeights;
        interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
        interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
        interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
        interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
        interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
        interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);


        pix = image->GetPixel(idx) * interpWeights[0];
        SampledShImageType::IndexType tmpIdx = idx; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[1];
        tmpIdx = idx; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[2];
        tmpIdx = idx; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[3];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[4];
        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[5];
        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[6];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  image->GetPixel(tmpIdx) * interpWeights[7];
    }

    return pix;
}

int main(int argc, char* argv[])
{
    MITK_INFO << "DFTraining";
    mitkCommandLineParser parser;

    parser.setTitle("Machine Learning Based Streamline Tractography");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("images", "i", mitkCommandLineParser::StringList, "DWIs:", "input diffusion-weighted images", us::Any(), false);
    parser.addArgument("wmmasks", "w", mitkCommandLineParser::StringList, "WM-Masks:", "white matter mask images", us::Any(), false);
    parser.addArgument("tractograms", "t", mitkCommandLineParser::StringList, "Tractograms:", "input tractograms (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("masks", "m", mitkCommandLineParser::StringList, "Masks:", "mask images", us::Any());
    parser.addArgument("forest", "f", mitkCommandLineParser::OutputFile, "Forest:", "output forest", us::Any(), false);

    parser.addArgument("stepsize", "s", mitkCommandLineParser::Float, "Stepsize:", "stepsize", us::Any());
    parser.addArgument("gmsamples", "g", mitkCommandLineParser::Int, "Number of gray matter samples per voxel:", "Number of gray matter samples per voxel", us::Any());
    parser.addArgument("numtrees", "n", mitkCommandLineParser::Int, "Number of trees:", "number of trees", us::Any());
    parser.addArgument("max_tree_depth", "d", mitkCommandLineParser::Int, "Max. tree depth:", "maximum tree depth", us::Any());
    parser.addArgument("sample_fraction", "sf", mitkCommandLineParser::Float, "Sample fraction:", "fraction of samples used per tree", us::Any());
    parser.addArgument("usedirection", "ud", mitkCommandLineParser::Bool, "bla:", "bla", us::Any());

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType imageFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["images"]);
    mitkCommandLineParser::StringContainerType wmMaskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["wmmasks"]);

    mitkCommandLineParser::StringContainerType maskFiles;
    if (parsedArgs.count("masks"))
        maskFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["masks"]);

    string forestFile = us::any_cast<string>(parsedArgs["forest"]);

    mitkCommandLineParser::StringContainerType tractogramFiles;
    if (parsedArgs.count("tractograms"))
        tractogramFiles = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["tractograms"]);

    int numTrees = 30;
    if (parsedArgs.count("numtrees"))
        numTrees = us::any_cast<int>(parsedArgs["numtrees"]);

    int gmsamples = 50;
    if (parsedArgs.count("gmsamples"))
        gmsamples = us::any_cast<int>(parsedArgs["gmsamples"]);

    float stepsize = -1;
    if (parsedArgs.count("stepsize"))
        stepsize = us::any_cast<float>(parsedArgs["stepsize"]);

    int max_tree_depth = 50;
    if (parsedArgs.count("max_tree_depth"))
        max_tree_depth = us::any_cast<int>(parsedArgs["max_tree_depth"]);

    double sample_fraction = 1.0;
    if (parsedArgs.count("sample_fraction"))
        sample_fraction = us::any_cast<float>(parsedArgs["sample_fraction"]);

    // load DWI images
    if (imageFiles.size()<tractogramFiles.size())
    {
        MITK_INFO << "Missing training images!";
        return EXIT_FAILURE;
    }
    bool useDirection = false;
    if (parsedArgs.count("usedirection"))
        useDirection = true;

    const int order = 6;
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,order,numOdfSamples> QballFilterType;

    MITK_INFO << "loading diffusion-weighted images and reconstructing feature images";
    std::vector< SampledShImageType::Pointer > sampledShImages;
    for (unsigned int i=0; i<imageFiles.size(); i++)
    {
        mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(imageFiles.at(i)).GetPointer());

        QballFilterType::Pointer qballfilter = QballFilterType::New();
        qballfilter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi), mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi) );
        qballfilter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi));
        qballfilter->SetLambda(0.006);
        qballfilter->SetNormalizationMethod(QballFilterType::QBAR_RAW_SIGNAL);
        qballfilter->Update();
        //            FeatureImageType::Pointer itkFeatureImage = qballfilter->GetCoefficientImage();
        //            featureImageVector.push_back(itkFeatureImage);
        sampledShImages.push_back(qballfilter->GetOutput());
    }


    typedef itk::Image<unsigned char, 3> ItkUcharImgType;
    std::vector< ItkUcharImgType::Pointer > maskImageVector;
    std::vector< ItkUcharImgType::Pointer > wmMaskImageVector;

    MITK_INFO << "loading tractograms";
    int numSamples = 0;
    std::vector< mitk::FiberBundle::Pointer > tractograms;
    for (unsigned int t=0; t<tractogramFiles.size(); t++)
    {
        ItkUcharImgType::Pointer mask;
        if (t<maskFiles.size())
        {
            mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(maskFiles.at(t)).GetPointer());
            mask = ItkUcharImgType::New();
            mitk::CastToItkImage(img, mask);
            maskImageVector.push_back(mask);
        }
        mitk::Image::Pointer img2 = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(wmMaskFiles.at(t)).GetPointer());
        ItkUcharImgType::Pointer wmmask = ItkUcharImgType::New();
        mitk::CastToItkImage(img2, wmmask);
        wmMaskImageVector.push_back(wmmask);

        itk::ImageRegionConstIterator<ItkUcharImgType> it(wmmask, wmmask->GetLargestPossibleRegion());
        int OUTOFWM = 0;    // count voxels outside of the white matter mask
        while(!it.IsAtEnd())
        {
            if (it.Get()==0)
                if (mask.IsNull() || (mask.IsNotNull() && mask->GetPixel(it.GetIndex())>0))
                    OUTOFWM++;
            ++it;
        }
        numSamples += gmsamples*OUTOFWM;    // for each of the non-white matter voxels we add a certain number of sampling points. these sampling points are used to tell the classifier where to recognize non-WM tissue

        MITK_INFO << "Samples outside of WM: " << numSamples << " (" << gmsamples << " per non-WM voxel)";

        // load and resample training tractograms
        mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(tractogramFiles.at(t)).at(0).GetPointer());
        if (stepsize<0)
        {
            SampledShImageType::Pointer image = sampledShImages.at(t);
            float minSpacing = 1;
            if(image->GetSpacing()[0]<image->GetSpacing()[1] && image->GetSpacing()[0]<image->GetSpacing()[2])
                minSpacing = image->GetSpacing()[0];
            else if (image->GetSpacing()[1] < image->GetSpacing()[2])
                minSpacing = image->GetSpacing()[1];
            else
                minSpacing = image->GetSpacing()[2];
            stepsize = minSpacing*0.5;
        }
        fib->ResampleSpline(stepsize);
        tractograms.push_back(fib);
        numSamples += fib->GetNumberOfPoints(); // each point of the fiber gives us a training direction
        numSamples -= 2*fib->GetNumFibers();    // we don't use the first and last point because there we do not have a previous direction, which is needed as feature
    }
    MITK_INFO << "Number of samples: " << numSamples;

    // get ODF directions and number of features
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    itk::OrientationDistributionFunction< double, numOdfSamples > odf;
    std::vector< int > directionIndices;
    for (unsigned int f=0; f<numOdfSamples; f++)
    {
        if (dot_product(ref, odf.GetDirection(f))>0)    // we only use directions on one hemisphere (symmetric)
            directionIndices.push_back(f);  // remember indices that are on the desired hemisphere
    }
    const int numSignalFeatures = numOdfSamples/2;
    int numDirectionFeatures = 0;
    if (useDirection)
        numDirectionFeatures = 3;

    vigra::MultiArray<2, double> featureData( vigra::Shape2(numSamples,numSignalFeatures+numDirectionFeatures) );
    MITK_INFO << "Number of features: " << featureData.shape(1);
    vigra::MultiArray<2, double> labelData( vigra::Shape2(numSamples,1) );

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();
    MITK_INFO <<  "Creating training data from tractograms and feature images";
    int sampleCounter = 0;
    for (unsigned int t=0; t<tractograms.size(); t++)
    {
        SampledShImageType::Pointer image = sampledShImages.at(t);  // raw data
        ItkUcharImgType::Pointer wmMask = wmMaskImageVector.at(t);  // white matter mask
        ItkUcharImgType::Pointer mask;                              // mask image. only train inside mask (if set)
        if (t<maskImageVector.size())
            mask = maskImageVector.at(t);

        // get training data for non-WM tissue
        itk::ImageRegionConstIterator<ItkUcharImgType> it(wmMask, wmMask->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            if (it.Get()==0 && (mask.IsNull() || (mask.IsNotNull() && mask->GetPixel(it.GetIndex())>0)))
            {
                SampledShImageType::PixelType pix = image->GetPixel(it.GetIndex());
                if (useDirection)
                {
                    // null direction
                    for (unsigned int f=0; f<numSignalFeatures; f++)
                    {
                        featureData(sampleCounter,f) = pix[directionIndices[f]];
                        if(featureData(sampleCounter,f)!=featureData(sampleCounter,f))
                            featureData(sampleCounter,f) = 0;
                    }
                    featureData(sampleCounter,numSignalFeatures) = 0;
                    featureData(sampleCounter,numSignalFeatures+1) = 0;
                    featureData(sampleCounter,numSignalFeatures+2) = 0;
                    labelData(sampleCounter,0) = directionIndices.size();
                    sampleCounter++;

                    // we use random "previous" directions for the non-WM samples
                    for (int i=1; i<gmsamples; i++)
                    {
                        for (unsigned int f=0; f<numSignalFeatures; f++)
                        {
                            featureData(sampleCounter,f) = pix[directionIndices[f]];
                            if(featureData(sampleCounter,f)!=featureData(sampleCounter,f))
                                featureData(sampleCounter,f) = 0;
                        }
                        int c=0;
                        vnl_vector_fixed<double,3> probe;
                        probe[0] = m_RandGen->GetVariate()*2-1;
                        probe[1] = m_RandGen->GetVariate()*2-1;
                        probe[2] = m_RandGen->GetVariate()*2-1;
                        probe.normalize();
                        if (dot_product(ref, probe)<0)
                            probe *= -1;
                        for (unsigned int f=numSignalFeatures; f<numSignalFeatures+3; f++)
                        {
                            featureData(sampleCounter,f) = probe[c];
                            c++;
                        }
                        labelData(sampleCounter,0) = directionIndices.size();
                        sampleCounter++;
                    }
                }
                else
                {
                    itk::ContinuousIndex<double, 3> idx;
                    idx[0] = it.GetIndex()[0];
                    idx[1] = it.GetIndex()[1];
                    idx[2] = it.GetIndex()[2];
                    itk::Point<float, 3> itkP1;
                    image->TransformContinuousIndexToPhysicalPoint(idx, itkP1);
                    SampledShImageType::PixelType pix = GetImageValues(itkP1, image);;
                    for (unsigned int f=0; f<numSignalFeatures; f++)
                    {
                        featureData(sampleCounter,f) = pix[directionIndices[f]];
                        if(featureData(sampleCounter,f)!=featureData(sampleCounter,f))
                            featureData(sampleCounter,f) = 0;
                    }
                    labelData(sampleCounter,0) = directionIndices.size();
                    sampleCounter++;

                    for (int i=1; i<gmsamples; i++)
                    {
                        itk::ContinuousIndex<double, 3> idx;
                        idx[0] = it.GetIndex()[0] + m_RandGen->GetVariate()-0.5;
                        idx[1] = it.GetIndex()[1] + m_RandGen->GetVariate()-0.5;
                        idx[2] = it.GetIndex()[2] + m_RandGen->GetVariate()-0.5;
                        itk::Point<float, 3> itkP1;
                        image->TransformContinuousIndexToPhysicalPoint(idx, itkP1);
                        SampledShImageType::PixelType pix = GetImageValues(itkP1, image);;
                        for (unsigned int f=0; f<numSignalFeatures; f++)
                        {
                            featureData(sampleCounter,f) = pix[directionIndices[f]];
                            if(featureData(sampleCounter,f)!=featureData(sampleCounter,f))
                                featureData(sampleCounter,f) = 0;
                        }
                        labelData(sampleCounter,0) = directionIndices.size();
                        sampleCounter++;
                    }
                }
            }
            ++it;
        }

        mitk::FiberBundle::Pointer fib = tractograms.at(t);
        vtkSmartPointer< vtkPolyData > polyData = fib->GetFiberPolyData();
        for (int i=0; i<fib->GetNumFibers(); i++)
        {
            vtkCell* cell = polyData->GetCell(i);
            int numPoints = cell->GetNumberOfPoints();
            vtkPoints* points = cell->GetPoints();

            vnl_vector_fixed<double,3> dirOld; dirOld.fill(0.0);

            for (int j=0; j<numPoints-1; j++)
            {
                // calculate direction
                double* p1 = points->GetPoint(j);
                itk::Point<float, 3> itkP1;
                itkP1[0] = p1[0]; itkP1[1] = p1[1]; itkP1[2] = p1[2];

                vnl_vector_fixed<double,3> dir; dir.fill(0.0);

                itk::Point<float, 3> itkP2;
                double* p2 = points->GetPoint(j+1);
                itkP2[0] = p2[0]; itkP2[1] = p2[1]; itkP2[2] = p2[2];
                dir[0]=itkP2[0]-itkP1[0];
                dir[1]=itkP2[1]-itkP1[1];
                dir[2]=itkP2[2]-itkP1[2];

                if (dir.magnitude()<0.0001)
                {
                    MITK_INFO << "streamline error!";
                    continue;
                }
                dir.normalize();
                if (dir[0]!=dir[0] || dir[1]!=dir[1] || dir[2]!=dir[2])
                {
                    MITK_INFO << "ERROR: NaN direction!";
                    continue;
                }

                if (j==0)
                {
                    dirOld = dir;
                    continue;
                }

                // get voxel values
                SampledShImageType::PixelType pix = GetImageValues(itkP1, image);
                for (unsigned int f=0; f<numSignalFeatures; f++)
                    featureData(sampleCounter,f) = pix[directionIndices[f]];

                // direction training features
                int c = 0;
                if (dot_product(ref, dirOld)<0)
                    dirOld *= -1;

                if (useDirection)
                    for (unsigned int f=numSignalFeatures; f<numSignalFeatures+3; f++)
                    {
                        featureData(sampleCounter,f) = dirOld[c];
                        c++;
                    }

                // set label values
                double angle = 0;
                double m = dir.magnitude();
                if (m>0.0001)
                {
                    int label = 0;
                    for (unsigned int f=0; f<numSignalFeatures; f++)
                    {
                        double a = fabs(dot_product(dir, odf.GetDirection(directionIndices[f])));
                        if (a>angle)
                        {
                            labelData(sampleCounter,0) = f;
                            angle = a;
                            label = f;
                        }
                    }
                }

                dirOld = dir;
                sampleCounter++;
            }
        }
    }

    MITK_INFO << "Training forest";
    vigra::RandomForest<int> rf;
    TrainForest( rf, labelData, featureData, numTrees, max_tree_depth, sample_fraction );
    MITK_INFO << "Writing forest";
    vigra::rf_export_HDF5( rf, forestFile, "" );
    MITK_INFO << "Finished training";

    return EXIT_SUCCESS;
}
