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
#include <itkOrientationDistributionFunction.h>
#include <chrono>

#undef DIFFERENCE
#define VIGRA_STATIC_LIB
#include <vigra/random_forest.hxx>
#include <vigra/multi_array.hxx>
#include <vigra/random_forest_hdf5_impex.hxx>

//#include <mitkVigraRandomForestClassifier.h>
#include <mitkLinearSplitting.h>
#include <mitkThresholdSplit.h>
#include <mitkImpurityLoss.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace mitk
{

/**
* \brief  Manages random forests for fiber tractography. The preparation of the features from the inputa data and the training process are handled here. The data preprocessing and actual prediction for the tracking process is also performed here. The tracking itself is performed in MLBSTrackingFilter. */

template< int ShOrder=6, int NumberOfSignalFeatures=100 >
class TrackingForestHandler
{

public:

    TrackingForestHandler();
    ~TrackingForestHandler();

    typedef itk::Image<short, 3>                                                ItkShortImgType;
    typedef itk::Image<float, 3>                                                ItkFloatImgType;
    typedef itk::Image<unsigned char, 3>                                        ItkUcharImgType;
    //typedef itk::Image< itk::Vector< float, NumberOfSignalFeatures*2 > , 3 >    DwiFeatureImageType;
    typedef itk::Image< itk::Vector< float, NumberOfSignalFeatures > , 3 >      DwiFeatureImageType;

    typedef mitk::ThresholdSplit<mitk::LinearSplitting< mitk::ImpurityLoss<> >,int,vigra::ClassificationTag> DefaultSplitType;

    void SetDwis( std::vector< Image::Pointer > images ){ m_InputDwis = images; }
    void AddDwi( Image::Pointer img ){ m_InputDwis.push_back(img); }
    void SetTractograms( std::vector< FiberBundle::Pointer > tractograms )
    {
        m_Tractograms.clear();
        for (auto fib : tractograms)
        {
            m_Tractograms.push_back(fib->GetDeepCopy());
        }
    }
    void SetMaskImages( std::vector< ItkUcharImgType::Pointer > images ){ m_MaskImages = images; }
    void SetWhiteMatterImages( std::vector< ItkUcharImgType::Pointer > images ){ m_WhiteMatterImages = images; }
    void SetFiberVolumeModImages( std::vector< ItkFloatImgType::Pointer > images ){ m_FiberVolumeModImages = images; }
    void SetAdditionalFeatureImages( std::vector< std::vector< ItkFloatImgType::Pointer > > images ){ m_AdditionalFeatureImages = images; }

    void StartTraining();
    void SaveForest(std::string forestFile);
    void LoadForest(std::string forestFile);

    // training parameters
    void SetNumPreviousDirections( int num ){ m_NumPreviousDirections=num; }
    void SetNumTrees(int num){ m_NumTrees = num; }
    void SetMaxTreeDepth(int depth){ m_MaxTreeDepth = depth; }
    void SetStepSize(double step){ m_WmSampleDistance = step; }
    void SetGrayMatterSamplesPerVoxel(int samples){ m_GmSamplesPerVoxel = samples; }
    void SetSampleFraction(double fraction){ m_SampleFraction = fraction; }
    std::shared_ptr< vigra::RandomForest<int> > GetForest(){ return m_Forest; }

    void InitForTracking();     ///< calls InputDataValidForTracking() and creates feature images
    vnl_vector_fixed<double,3> Classify(itk::Point<double, 3>& pos, int& candidates, std::deque< vnl_vector_fixed<double,3> >& olddirs, double angularThreshold, double& w, ItkUcharImgType::Pointer mask=nullptr);  ///< predicts next progression direction at the given position

    bool IsForestValid();   ///< true is forest is not null, has more than 0 trees and the correct number of features (NumberOfSignalFeatures + 3)

protected:

    // tracking
    void InputDataValidForTracking();                                                   ///< check if raw data is set and tracking forest is valid

    template< class TPixelType >
    TPixelType GetImageValue(itk::Point<float, 3> itkP, itk::Image<TPixelType, 3>* image, bool interpolate);

    // training
    void InputDataValidForTraining();       ///< Check if everything is tehere for training (raw datasets, fiber tracts)
    void InitForTraining();  ///< Generate masks if necessary, resample fibers, spherically interpolate raw DWIs
    void CalculateTrainingSamples();    ///< Calculate GM and WM features using the interpolated raw data, the WM masks and the fibers
    typename DwiFeatureImageType::PixelType GetDwiFeaturesAtPosition(itk::Point<float, 3> itkP, typename DwiFeatureImageType::Pointer image);   ///< get trilinearly interpolated raw image values at given world position


    std::vector< Image::Pointer >                               m_InputDwis;  ///< original input DWI data
    std::shared_ptr< vigra::RandomForest<int> >                 m_Forest;   ///< random forest classifier
    std::chrono::time_point<std::chrono::system_clock>          m_StartTime;
    std::chrono::time_point<std::chrono::system_clock>          m_EndTime;


    std::vector< typename DwiFeatureImageType::Pointer >        m_DwiFeatureImages;
    std::vector< std::vector< ItkFloatImgType::Pointer > >      m_AdditionalFeatureImages;

    std::vector< ItkFloatImgType::Pointer >                     m_FiberVolumeModImages;     ///< used to correct the fiber density
    std::vector< FiberBundle::Pointer >                         m_Tractograms;              ///< training tractograms
    std::vector< ItkUcharImgType::Pointer >                     m_MaskImages;               ///< binary mask images to constrain training to a certain area (e.g. brain mask)
    std::vector< ItkUcharImgType::Pointer >                     m_WhiteMatterImages;        ///< defines white matter voxels. if not set, theses mask images are automatically generated from the input tractograms

    double                                                      m_WmSampleDistance;         ///< deterines the number of white matter samples (distance of sampling points on each fiber).
    int                                                         m_NumTrees;                 ///< number of trees in random forest
    int                                                         m_MaxTreeDepth;             ///< limits the tree depth
    double                                                      m_SampleFraction;           ///< fraction of samples used to train each tree
    unsigned int                                                m_NumberOfSamples;          ///< stores overall number of samples used for training
    std::vector< unsigned int >                                 m_GmSamples;                ///< number of gray matter samples
    int                                                         m_GmSamplesPerVoxel;        ///< number of gray matter samplees per voxel. if -1, then the number is automatically chosen to gain an overall number of GM samples close to the number of WM samples.
    vigra::MultiArray<2, double>                                m_FeatureData;              ///< vigra container for training features
    int                                                         m_NumPreviousDirections;    ///< How many "old" directions should be used as classification features?

    // only for tracking
    vigra::MultiArray<2, double>                                m_LabelData;                ///< vigra container for training labels
    vigra::MultiArray<2, double>                                m_Weights;                  ///< vigra container for training sample weights

    std::vector< vnl_vector_fixed<double,3> >                   m_DirectionContainer;
};

}

#include "mitkTrackingForestHandler.cpp"

#endif
