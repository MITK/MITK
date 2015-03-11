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

#ifndef _TrackingForestHandler_cpp
#define _TrackingForestHandler_cpp

#include "mitkTrackingForestHandler.h"
#include <itkTractDensityImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>

namespace mitk
{

template< int NumberOfSignalFeatures >
TrackingForestHandler< NumberOfSignalFeatures >::TrackingForestHandler()
    : m_GrayMatterSamplesPerVoxel(50)
    , m_StepSize(-1)
    , m_UsePreviousDirection(true)
    , m_NumTrees(30)
    , m_MaxTreeDepth(50)
    , m_SampleFraction(1.0)
{

}

template< int NumberOfSignalFeatures >
TrackingForestHandler< NumberOfSignalFeatures >::~TrackingForestHandler()
{

}

template< int NumberOfSignalFeatures >
typename TrackingForestHandler< NumberOfSignalFeatures >::InterpolatedRawImageType::PixelType TrackingForestHandler< NumberOfSignalFeatures >::GetImageValues(itk::Point<float, 3> itkP, typename InterpolatedRawImageType::Pointer image)
{
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    image->TransformPhysicalPointToIndex(itkP, idx);
    image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    typename InterpolatedRawImageType::PixelType pix; pix.Fill(0.0);
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
        typename InterpolatedRawImageType::IndexType tmpIdx = idx; tmpIdx[0]++;
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

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::InputDataValidForTracking()
{
    if (m_RawData.empty())
        mitkThrow() << "No diffusion-weighted images set!";
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::StartTraining()
{
    InputDataValidForTraining();
    PreprocessInputData();
    CalculateFeatures();
    TrainForest();
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::InputDataValidForTraining()
{
    if (m_RawData.empty())
        mitkThrow() << "No diffusion-weighted images set!";
    if (m_Tractograms.empty())
        mitkThrow() << "No tractograms set!";
    if (m_RawData.size()!=m_Tractograms.size())
        mitkThrow() << "Unequal number of diffusion-weighted images and tractograms detected!";
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::PreprocessInputData()
{
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,6, 2*NumberOfSignalFeatures> InterpolationFilterType;

    MITK_INFO << "Spherical signal interpolation and sampling ...";
    for (unsigned int i=0; i<m_RawData.size(); i++)
    {
        typename InterpolationFilterType::Pointer qballfilter = InterpolationFilterType::New();
        qballfilter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(m_RawData.at(i)), mitk::DiffusionPropertyHelper::GetItkVectorImage(m_RawData.at(i)) );
        qballfilter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(m_RawData.at(i)));
        qballfilter->SetLambda(0.006);
        qballfilter->SetNormalizationMethod(InterpolationFilterType::QBAR_RAW_SIGNAL);
        qballfilter->Update();
        //      FeatureImageType::Pointer itkFeatureImage = qballfilter->GetCoefficientImage();
        //      featureImageVector.push_back(itkFeatureImage);
        m_InterpolatedRawImages.push_back(qballfilter->GetOutput());

        if (i>=m_MaskImages.size())
        {
            ItkUcharImgType::Pointer newMask = ItkUcharImgType::New();
            newMask->SetSpacing( m_InterpolatedRawImages.at(i)->GetSpacing() );
            newMask->SetOrigin( m_InterpolatedRawImages.at(i)->GetOrigin() );
            newMask->SetDirection( m_InterpolatedRawImages.at(i)->GetDirection() );
            newMask->SetLargestPossibleRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
            newMask->SetBufferedRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
            newMask->SetRequestedRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
            newMask->Allocate();
            newMask->FillBuffer(1);
            m_MaskImages.push_back(newMask);
        }
    }

    MITK_INFO << "Resampling fibers and calculating number of samples ...";
    m_NumberOfSamples = 0;
    for (unsigned int t=0; t<m_Tractograms.size(); t++)
    {
        ItkUcharImgType::Pointer mask = m_MaskImages.at(t);
        ItkUcharImgType::Pointer wmmask;
        if (t<m_WhiteMatterImages.size())
            wmmask = m_WhiteMatterImages.at(t);
        else
        {
            MITK_INFO << "Calculating fiber envelope";
            itk::TractDensityImageFilter< ItkUcharImgType >::Pointer env = itk::TractDensityImageFilter< ItkUcharImgType >::New();
            env->SetFiberBundle(m_Tractograms.at(t));
            env->SetInputImage(mask);
            env->SetBinaryOutput(true);
            env->SetUseImageGeometry(true);
            env->Update();
            wmmask = env->GetOutput();
            m_WhiteMatterImages.push_back(wmmask);
        }

        itk::ImageRegionConstIterator<ItkUcharImgType> it(wmmask, wmmask->GetLargestPossibleRegion());
        int OUTOFWM = 0;
        while(!it.IsAtEnd())
        {
            if (it.Get()==0 && mask->GetPixel(it.GetIndex())>0)
                OUTOFWM++;
            ++it;
        }
        m_NumberOfSamples += m_GrayMatterSamplesPerVoxel*OUTOFWM;
        MITK_INFO << "Samples outside of WM: " << m_NumberOfSamples;

        if (m_StepSize<0)
        {
            typename InterpolatedRawImageType::Pointer image = m_InterpolatedRawImages.at(t);
            float minSpacing = 1;
            if(image->GetSpacing()[0]<image->GetSpacing()[1] && image->GetSpacing()[0]<image->GetSpacing()[2])
                minSpacing = image->GetSpacing()[0];
            else if (image->GetSpacing()[1] < image->GetSpacing()[2])
                minSpacing = image->GetSpacing()[1];
            else
                minSpacing = image->GetSpacing()[2];
            m_StepSize = minSpacing*0.5;
        }

        m_Tractograms.at(t)->ResampleSpline(m_StepSize);
        m_NumberOfSamples += m_Tractograms.at(t)->GetNumberOfPoints();
        m_NumberOfSamples -= 2*m_Tractograms.at(t)->GetNumFibers();
    }
    MITK_INFO << "Number of samples: " << m_NumberOfSamples;
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::CalculateFeatures()
{
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    itk::OrientationDistributionFunction< double, 2*NumberOfSignalFeatures > directions;
    std::vector< int > directionIndices;
    for (unsigned int f=0; f<2*NumberOfSignalFeatures; f++)
    {
        if (dot_product(ref, directions.GetDirection(f))>0)
            directionIndices.push_back(f);
    }

    int numDirectionFeatures = 0;
    if (m_UsePreviousDirection)
        numDirectionFeatures = 3;

    m_FeatureData.reshape( vigra::Shape2(m_NumberOfSamples, NumberOfSignalFeatures+numDirectionFeatures) );
    m_LabelData.reshape( vigra::Shape2(m_NumberOfSamples,1) );
    MITK_INFO << "Number of features: " << m_FeatureData.shape(1);

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();
    MITK_INFO <<  "Creating training data ...";
    int sampleCounter = 0;
    for (unsigned int t=0; t<m_Tractograms.size(); t++)
    {
        typename InterpolatedRawImageType::Pointer image = m_InterpolatedRawImages.at(t);
        ItkUcharImgType::Pointer wmMask = m_WhiteMatterImages.at(t);
        ItkUcharImgType::Pointer mask;
        if (t<m_MaskImages.size())
            mask = m_MaskImages.at(t);

        // non-white matter samples
        itk::ImageRegionConstIterator<ItkUcharImgType> it(wmMask, wmMask->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            if (it.Get()==0 && (mask.IsNull() || (mask.IsNotNull() && mask->GetPixel(it.GetIndex())>0)))
            {
                typename InterpolatedRawImageType::PixelType pix = image->GetPixel(it.GetIndex());
                if (m_UsePreviousDirection)
                {
                    // null direction
                    for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                    {
                        m_FeatureData(sampleCounter,f) = pix[directionIndices[f]];
                        if(m_FeatureData(sampleCounter,f)!=m_FeatureData(sampleCounter,f))
                            m_FeatureData(sampleCounter,f) = 0;
                    }
                    m_FeatureData(sampleCounter,NumberOfSignalFeatures) = 0;
                    m_FeatureData(sampleCounter,NumberOfSignalFeatures+1) = 0;
                    m_FeatureData(sampleCounter,NumberOfSignalFeatures+2) = 0;
                    m_LabelData(sampleCounter,0) = directionIndices.size();
                    sampleCounter++;

                    // random directions
                    for (int i=1; i<m_GrayMatterSamplesPerVoxel; i++)
                    {
                        for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                        {
                            m_FeatureData(sampleCounter,f) = pix[directionIndices[f]];
                            if(m_FeatureData(sampleCounter,f)!=m_FeatureData(sampleCounter,f))
                                m_FeatureData(sampleCounter,f) = 0;
                        }
                        int c=0;
                        vnl_vector_fixed<double,3> probe;
                        probe[0] = m_RandGen->GetVariate()*2-1;
                        probe[1] = m_RandGen->GetVariate()*2-1;
                        probe[2] = m_RandGen->GetVariate()*2-1;
                        probe.normalize();
                        if (dot_product(ref, probe)<0)
                            probe *= -1;
                        for (unsigned int f=NumberOfSignalFeatures; f<NumberOfSignalFeatures+3; f++)
                        {
                            m_FeatureData(sampleCounter,f) = probe[c];
                            c++;
                        }
                        m_LabelData(sampleCounter,0) = directionIndices.size();
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
                    typename InterpolatedRawImageType::PixelType pix = GetImageValues(itkP1, image);;
                    for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                    {
                        m_FeatureData(sampleCounter,f) = pix[directionIndices[f]];
                        if(m_FeatureData(sampleCounter,f)!=m_FeatureData(sampleCounter,f))
                            m_FeatureData(sampleCounter,f) = 0;
                    }
                    m_LabelData(sampleCounter,0) = directionIndices.size();
                    sampleCounter++;

                    for (int i=1; i<m_GrayMatterSamplesPerVoxel; i++)
                    {
                        itk::ContinuousIndex<double, 3> idx;
                        idx[0] = it.GetIndex()[0] + m_RandGen->GetVariate()-0.5;
                        idx[1] = it.GetIndex()[1] + m_RandGen->GetVariate()-0.5;
                        idx[2] = it.GetIndex()[2] + m_RandGen->GetVariate()-0.5;
                        itk::Point<float, 3> itkP1;
                        image->TransformContinuousIndexToPhysicalPoint(idx, itkP1);
                        typename InterpolatedRawImageType::PixelType pix = GetImageValues(itkP1, image);;
                        for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                        {
                            m_FeatureData(sampleCounter,f) = pix[directionIndices[f]];
                            if(m_FeatureData(sampleCounter,f)!=m_FeatureData(sampleCounter,f))
                                m_FeatureData(sampleCounter,f) = 0;
                        }
                        m_LabelData(sampleCounter,0) = directionIndices.size();
                        sampleCounter++;
                    }
                }
            }
            ++it;
        }

        // white matter samples
        mitk::FiberBundle::Pointer fib = m_Tractograms.at(t);
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
                typename InterpolatedRawImageType::PixelType pix = GetImageValues(itkP1, image);
                for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                    m_FeatureData(sampleCounter,f) = pix[directionIndices[f]];

                // direction training features
                int c = 0;
                if (dot_product(ref, dirOld)<0)
                    dirOld *= -1;

                if (m_UsePreviousDirection)
                    for (unsigned int f=NumberOfSignalFeatures; f<NumberOfSignalFeatures+3; f++)
                    {
                        m_FeatureData(sampleCounter,f) = dirOld[c];
                        c++;
                    }

                // set label values
                double angle = 0;
                double m = dir.magnitude();
                if (m>0.0001)
                {
                    int label = 0;
                    for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
                    {
                        double a = fabs(dot_product(dir, directions.GetDirection(directionIndices[f])));
                        if (a>angle)
                        {
                            m_LabelData(sampleCounter,0) = f;
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
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::TrainForest()
{
    MITK_INFO << "Maximum tree depths: " << m_MaxTreeDepth;
    MITK_INFO << "Sample fraction per tree: " << m_SampleFraction;
    MITK_INFO << "Number of trees: " << m_NumTrees;
    bool random_split = false;
    vigra::rf::visitors::OOB_Error oob_v;
    MITK_INFO << "Create Split Function";
    //   typedef ThresholdSplit<int, vigra::BestGiniOfColumn<vigra::GiniCriterion>, vigra::ClassificationTag> DefaultSplitType;

    m_Forest.set_options().use_stratification(vigra::RF_NONE); // How the data should be made equal
    m_Forest.set_options().sample_with_replacement(true); // if sampled with replacement or not
    m_Forest.set_options().samples_per_tree(m_SampleFraction); // Fraction of samples that are used to train a tree
    m_Forest.set_options().tree_count(1); // Number of trees that are calculated;
    m_Forest.set_options().min_split_node_size(5); // Minimum number of datapoints that must be in a node
    //    rf.set_options().features_per_node(10);

    m_Forest.learn(m_FeatureData, m_LabelData, vigra::rf::visitors::create_visitor(oob_v));

    // Prepare parallel VariableImportance Calculation
    int numMod =  m_FeatureData.shape(1);
    const int numClass = 2 + 2;

    float** varImp = new float*[numMod];

    for(int i = 0; i < numMod; i++)
        varImp[i] = new float[numClass];

    for (int i = 0; i < numMod; ++i)
        for (int j = 0; j < numClass; ++j)
            varImp[i][j] = 0.0;

#pragma omp parallel for
    for (int i = 0; i < m_NumTrees - 1; ++i)
    {
        vigra::RandomForest<int> lrf;
        vigra::rf::visitors::OOB_Error loob_v;

        lrf.set_options().use_stratification(vigra::RF_NONE); // How the data should be made equal
        lrf.set_options().sample_with_replacement(true); // if sampled with replacement or not
        lrf.set_options().samples_per_tree(m_SampleFraction); // Fraction of samples that are used to train a tree
        lrf.set_options().tree_count(1); // Number of trees that are calculated;
        lrf.set_options().min_split_node_size(5); // Minimum number of datapoints that must be in a node
        //        lrf.set_options().features_per_node(10);

        vigra::rf::visitors::VariableImportanceVisitor lvariableImportance;
        lrf.learn(m_FeatureData, m_LabelData, vigra::rf::visitors::create_visitor(loob_v));

#pragma omp critical
        {
            m_Forest.trees_.push_back(lrf.trees_[0]);
        }
    }

    m_Forest.options_.tree_count_ = m_NumTrees;
    MITK_INFO << "Training finsihed";
    MITK_INFO << "The out-of-bag error is: " << oob_v.oob_breiman << std::endl;
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::SaveForest(std::string forestFile)
{
    MITK_INFO << "Saving forest to " << forestFile;
    vigra::rf_export_HDF5( m_Forest, forestFile, "" );
}

template< int NumberOfSignalFeatures >
void TrackingForestHandler< NumberOfSignalFeatures >::LoadForest(std::string forestFile)
{
    MITK_INFO << "Loading forest from " << forestFile;
    vigra::rf_import_HDF5(m_Forest, forestFile);
}

//// superclass implementations
//template< int NumberOfSignalFeatures >
//void TrackingForestHandler< NumberOfSignalFeatures >::UpdateOutputInformation()
//{

//}
//template< int NumberOfSignalFeatures >
//void TrackingForestHandler< NumberOfSignalFeatures >::SetRequestedRegionToLargestPossibleRegion()
//{

//}
//template< int NumberOfSignalFeatures >
//bool TrackingForestHandler< NumberOfSignalFeatures >::RequestedRegionIsOutsideOfTheBufferedRegion()
//{
//    return false;
//}
//template< int NumberOfSignalFeatures >
//bool TrackingForestHandler< NumberOfSignalFeatures >::VerifyRequestedRegion()
//{
//    return true;
//}
//template< int NumberOfSignalFeatures >
//void TrackingForestHandler< NumberOfSignalFeatures >::SetRequestedRegion(const itk::DataObject* )
//{

//}

}

#endif
