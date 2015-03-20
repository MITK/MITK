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

#ifndef _TrackingForestHandler
#define _TrackingForestHandler

#include "mitkBaseData.h"

#include <mitkFiberBundle.h>
#include <itkAnalyticalDiffusionQballReconstructionImageFilter.h>
#include <itkOrientationDistributionFunction.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// classification includes
//#include "RegressionForestClasses.hxx"
#undef DIFFERENCE
#define VIGRA_STATIC_LIB
#include <vigra/random_forest.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

#define _USE_MATH_DEFINES
#include <math.h>

namespace mitk
{

/**
* \brief  */

template< int NumberOfSignalFeatures=100 >
class TrackingForestHandler
{

public:

    TrackingForestHandler();
    ~TrackingForestHandler();

    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
    typedef itk::Image< itk::Vector< float, NumberOfSignalFeatures*2 > , 3 > InterpolatedRawImageType;

    void SetRawData( std::vector< Image::Pointer > images ){ m_RawData = images; }
    void SetTractograms( std::vector< FiberBundle::Pointer > tractograms )
    {
        m_Tractograms.clear();
        for (int i=0; i<tractograms.size(); i++)
        {
            m_Tractograms.push_back(tractograms.at(i)->GetDeepCopy());
        }
    }
    void SetMaskImages( std::vector< ItkUcharImgType::Pointer > images ){ m_MaskImages = images; }
    void SetWhiteMatterImages( std::vector< ItkUcharImgType::Pointer > images ){ m_WhiteMatterImages = images; }

    void StartTraining();
    void SaveForest(std::string forestFile);
    void LoadForest(std::string forestFile);

    void SetNumTrees(int num){ m_NumTrees = num; }
    void SetMaxTreeDepth(int depth){ m_MaxTreeDepth = depth; }
    void SetUsePreviousDirection(bool use){ m_UsePreviousDirection = use; }
    void SetStepSize(double step){ m_StepSize = step; }
    void SetGrayMatterSamplesPerVoxel(int samples){ m_GrayMatterSamplesPerVoxel = samples; }
    void SetSampleFraction(double fraction){ m_SampleFraction = fraction; }
    vigra::RandomForest<int> GetForest(){ return m_Forest; }

protected:

    void InputDataValidForTracking();
    void InputDataValidForTraining();
    void PreprocessInputData();
    void CalculateFeatures();
    void TrainForest();

    int         m_GrayMatterSamplesPerVoxel;
    double      m_StepSize;
    bool        m_UsePreviousDirection;
    int         m_NumTrees;
    int         m_MaxTreeDepth;
    double      m_SampleFraction;

    std::vector< Image::Pointer > m_RawData;
    std::vector< FiberBundle::Pointer > m_Tractograms;
    std::vector< ItkUcharImgType::Pointer > m_MaskImages;
    std::vector< ItkUcharImgType::Pointer > m_WhiteMatterImages;
    std::vector< ItkUcharImgType::Pointer > m_SeedImages;
    std::vector< ItkUcharImgType::Pointer > m_StopImages;

    int m_NumberOfSamples;
    vigra::RandomForest<int> m_Forest;
    vigra::MultiArray<2, double> m_FeatureData;
    vigra::MultiArray<2, double> m_LabelData;
    std::vector< typename InterpolatedRawImageType::Pointer > m_InterpolatedRawImages;

    typename InterpolatedRawImageType::PixelType GetImageValues(itk::Point<float, 3> itkP, typename InterpolatedRawImageType::Pointer image);
};

}

#include "mitkTrackingForestHandler.cpp"

#endif
