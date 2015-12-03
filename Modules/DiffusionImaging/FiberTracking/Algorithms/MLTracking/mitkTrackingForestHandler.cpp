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
  template< int ShOrder, int NumberOfSignalFeatures >
  TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::TrackingForestHandler()
    : m_WmSampleDistance(-1)
    , m_NumTrees(30)
    , m_MaxTreeDepth(50)
    , m_SampleFraction(1.0)
    , m_NumberOfSamples(0)
    , m_GmSamplesPerVoxel(50)
  {
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::~TrackingForestHandler()
  {
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  typename TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::InterpolatedRawImageType::PixelType TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::GetImageValues(itk::Point<float, 3> itkP, typename InterpolatedRawImageType::Pointer image)
  {
    // transform physical point to index coordinates
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
      // trilinear interpolation
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

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::InputDataValidForTracking()
  {
    if (m_RawData.empty())
      mitkThrow() << "No diffusion-weighted images set!";
    if (!IsForestValid())
      mitkThrow() << "No or invalid random forest detected!";
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::InitForTracking()
  {
    InputDataValidForTracking();

    MITK_INFO << "Spherically interpolating raw data and creating feature image ...";
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,ShOrder, 2*NumberOfSignalFeatures> InterpolationFilterType;

    typename InterpolationFilterType::Pointer filter = InterpolationFilterType::New();
    filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetGradientContainer(m_RawData.at(0)), mitk::DiffusionPropertyHelper::GetItkVectorImage(m_RawData.at(0)) );
    filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(m_RawData.at(0)));
    filter->SetLambda(0.006);
    filter->SetNormalizationMethod(InterpolationFilterType::QBAR_RAW_SIGNAL);
    filter->Update();

    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    itk::OrientationDistributionFunction< double, NumberOfSignalFeatures*2 > odf;
    m_DirectionIndices.clear();
    for (unsigned int f=0; f<NumberOfSignalFeatures*2; f++)
    {
      if (dot_product(ref, odf.GetDirection(f))>0)    // only used directions on one hemisphere
        m_DirectionIndices.push_back(f);            // store indices for later mapping the classifier output to the actual direction
    }

    m_FeatureImage = FeatureImageType::New();
    m_FeatureImage->SetSpacing(filter->GetOutput()->GetSpacing());
    m_FeatureImage->SetOrigin(filter->GetOutput()->GetOrigin());
    m_FeatureImage->SetDirection(filter->GetOutput()->GetDirection());
    m_FeatureImage->SetLargestPossibleRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->SetBufferedRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->SetRequestedRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->Allocate();

    // get signal values and store them in the feature image
    itk::ImageRegionIterator< typename InterpolationFilterType::OutputImageType > it(filter->GetOutput(), filter->GetOutput()->GetLargestPossibleRegion());
    while(!it.IsAtEnd())
    {
      typename FeatureImageType::PixelType pix;
      for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
        pix[f] = it.Get()[m_DirectionIndices.at(f)];
      m_FeatureImage->SetPixel(it.GetIndex(), pix);
      ++it;
    }

    //m_Forest->multithreadPrediction = false;
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  typename TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::FeatureImageType::PixelType TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::GetFeatureValues(itk::Point<float, 3> itkP)
  {
    // transform physical point to index coordinates
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    m_FeatureImage->TransformPhysicalPointToIndex(itkP, idx);
    m_FeatureImage->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    typename FeatureImageType::PixelType pix; pix.Fill(0.0);
    if ( m_FeatureImage->GetLargestPossibleRegion().IsInside(idx) )
      pix = m_FeatureImage->GetPixel(idx);
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
    if (idx[0] >= 0 && idx[0] < m_FeatureImage->GetLargestPossibleRegion().GetSize(0)-1 &&
      idx[1] >= 0 && idx[1] < m_FeatureImage->GetLargestPossibleRegion().GetSize(1)-1 &&
      idx[2] >= 0 && idx[2] < m_FeatureImage->GetLargestPossibleRegion().GetSize(2)-1)
    {
      // trilinear interpolation
      vnl_vector_fixed<double, 8> interpWeights;
      interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
      interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
      interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
      interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
      interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
      interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
      interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
      interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

      pix = m_FeatureImage->GetPixel(idx) * interpWeights[0];
      typename FeatureImageType::IndexType tmpIdx = idx; tmpIdx[0]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[1];
      tmpIdx = idx; tmpIdx[1]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[2];
      tmpIdx = idx; tmpIdx[2]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[3];
      tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[4];
      tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[5];
      tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[6];
      tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
      pix +=  m_FeatureImage->GetPixel(tmpIdx) * interpWeights[7];
    }

    return pix;
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  vnl_vector_fixed<double,3> TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::Classify(itk::Point<double, 3>& pos, int& candidates, vnl_vector_fixed<double,3>& olddir, double angularThreshold, double& w, ItkUcharImgType::Pointer mask)
  {
    vnl_vector_fixed<double,3> direction; direction.fill(0);

    itk::Index<3> idx;
    m_FeatureImage->TransformPhysicalPointToIndex(pos, idx);
    if (mask.IsNotNull() && ((mask->GetLargestPossibleRegion().IsInside(idx) && mask->GetPixel(idx)<=0) || !mask->GetLargestPossibleRegion().IsInside(idx)) )
      return direction;

    // store feature pixel values in a vigra data type
    vigra::MultiArray<2, double> featureData = vigra::MultiArray<2, double>( vigra::Shape2(1,NumberOfSignalFeatures+3) );
    typename FeatureImageType::PixelType featurePixel = GetFeatureValues(pos);
    for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
      featureData(0,f) = featurePixel[f];

    // append normalized previous direction to feature vector
    int c = 0;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    for (unsigned int f=NumberOfSignalFeatures; f<NumberOfSignalFeatures+3; f++)
    {
      if (dot_product(ref, olddir)<0)
        featureData(0,f) = -olddir[c];
      else
        featureData(0,f) = olddir[c];
      c++;
    }

    // perform classification
    vigra::MultiArray<2, double> probs(vigra::Shape2(1, m_Forest->class_count()));
    m_Forest->predictProbabilities(featureData, probs);

    double pNonFib = 0;     // probability that we left the white matter
    w = 0;                  // weight of the predicted direction
    candidates = 0;         // directions with probability > 0
    for (int i=0; i<m_Forest->class_count(); i++)   // for each class (number of possible directions + out-of-wm class)
    {
      if (probs(0,i)>0)   // if probability of respective class is 0, do nothing
      {
        // get label of class (does not correspond to the loop variable i)
        int classLabel = 0;
        m_Forest->ext_param_.to_classlabel(i, classLabel);

        if (classLabel<m_DirectionIndices.size())   // does class label correspond to a direction or to the out-of-wm class?
        {
          candidates++;   // now we have one direction more with probability > 0 (DO WE NEED THIS???)
          vnl_vector_fixed<double,3> d = m_DirContainer.GetDirection(m_DirectionIndices.at(classLabel));  // get direction vector assiciated with the respective direction index

          if (olddir.magnitude()>0)   // do we have a previous streamline direction or did we just start?
          {
            // TODO: check if hard curvature threshold is necessary.
            // alternatively try square of dot pruduct as weight.
            // TODO: check if additional weighting with dot product as directional prior is necessary. are there alternatives on the classification level?

            double dot = dot_product(d, olddir);    // claculate angle between the candidate direction vector and the previous streamline direction
            if (fabs(dot)>angularThreshold)         // is angle between the directions smaller than our hard threshold?
            {
              if (dot<0)                          // make sure we don't walk backwards
                d *= -1;
              double w_i = probs(0,i)*fabs(dot);
              direction += w_i*d; // weight contribution to output direction with its probability and the angular deviation from the previous direction
              w += w_i;           // increase output weight of the final direction
            }
          }
          else
          {
            direction += probs(0,i)*d;
            w += probs(0,i);
          }
        }
        else
          pNonFib += probs(0,i);  // probability that we are not in the whte matter anymore
      }
    }

    // if we did not find a suitable direction, make sure that we return (0,0,0)
    if (pNonFib>w && w>0)
    {
      candidates = 0;
      w = 0;
      direction.fill(0.0);
    }

    return direction;
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::StartTraining()
  {
    m_StartTime = std::chrono::system_clock::now();
    InputDataValidForTraining();
    PreprocessInputDataForTraining();
    CalculateFeaturesForTraining();
    TrainForest();
    m_EndTime = std::chrono::system_clock::now();
    std::chrono::hours   hh = std::chrono::duration_cast<std::chrono::hours>(m_EndTime - m_StartTime);
    std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(m_EndTime - m_StartTime);
    mm %= 60;
    MITK_INFO << "Training took " << hh.count() << "h and " << mm.count() << "m";
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::InputDataValidForTraining()
  {
    if (m_RawData.empty())
      mitkThrow() << "No diffusion-weighted images set!";
    if (m_Tractograms.empty())
      mitkThrow() << "No tractograms set!";
    if (m_RawData.size()!=m_Tractograms.size())
      mitkThrow() << "Unequal number of diffusion-weighted images and tractograms detected!";
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  bool TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::IsForestValid()
  {
    if(m_Forest && m_Forest->tree_count()>0 && m_Forest->feature_count()==(NumberOfSignalFeatures+3))
      return true;
    return false;
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::PreprocessInputDataForTraining()
  {
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,ShOrder, 2*NumberOfSignalFeatures> InterpolationFilterType;

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

      if (m_MaskImages.at(i)==nullptr)
      {
        m_MaskImages.at(i) = ItkUcharImgType::New();
        m_MaskImages.at(i)->SetSpacing( m_InterpolatedRawImages.at(i)->GetSpacing() );
        m_MaskImages.at(i)->SetOrigin( m_InterpolatedRawImages.at(i)->GetOrigin() );
        m_MaskImages.at(i)->SetDirection( m_InterpolatedRawImages.at(i)->GetDirection() );
        m_MaskImages.at(i)->SetLargestPossibleRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
        m_MaskImages.at(i)->SetBufferedRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
        m_MaskImages.at(i)->SetRequestedRegion( m_InterpolatedRawImages.at(i)->GetLargestPossibleRegion() );
        m_MaskImages.at(i)->Allocate();
        m_MaskImages.at(i)->FillBuffer(1);
      }
    }

    MITK_INFO << "Resampling fibers and calculating number of samples ...";
    m_NumberOfSamples = 0;
    for (unsigned int t=0; t<m_Tractograms.size(); t++)
    {
      ItkUcharImgType::Pointer mask = m_MaskImages.at(t);
      ItkUcharImgType::Pointer wmmask;
      if (t<m_WhiteMatterImages.size() && m_WhiteMatterImages.at(t)!=nullptr)
        wmmask = m_WhiteMatterImages.at(t);
      else
      {
        MITK_INFO << "No white-matter mask found. Using fiber envelope.";
        itk::TractDensityImageFilter< ItkUcharImgType >::Pointer env = itk::TractDensityImageFilter< ItkUcharImgType >::New();
        env->SetFiberBundle(m_Tractograms.at(t));
        env->SetInputImage(mask);
        env->SetBinaryOutput(true);
        env->SetUseImageGeometry(true);
        env->Update();
        wmmask = env->GetOutput();
        if (t>=m_WhiteMatterImages.size())
          m_WhiteMatterImages.push_back(wmmask);
        else
          m_WhiteMatterImages.at(t) = wmmask;
      }

      // Calculate white-matter samples
      if (m_WmSampleDistance<0)
      {
        typename InterpolatedRawImageType::Pointer image = m_InterpolatedRawImages.at(t);
        float minSpacing = 1;
        if(image->GetSpacing()[0]<image->GetSpacing()[1] && image->GetSpacing()[0]<image->GetSpacing()[2])
          minSpacing = image->GetSpacing()[0];
        else if (image->GetSpacing()[1] < image->GetSpacing()[2])
          minSpacing = image->GetSpacing()[1];
        else
          minSpacing = image->GetSpacing()[2];
        m_WmSampleDistance = minSpacing*0.5;
      }

      m_Tractograms.at(t)->ResampleSpline(m_WmSampleDistance);
      unsigned int wmSamples = m_Tractograms.at(t)->GetNumberOfPoints()-2*m_Tractograms.at(t)->GetNumFibers();
      m_NumberOfSamples += wmSamples;
      MITK_INFO << "Samples inside of WM: " << wmSamples;

      // calculate gray-matter samples
      itk::ImageRegionConstIterator<ItkUcharImgType> it(wmmask, wmmask->GetLargestPossibleRegion());
      int OUTOFWM = 0;
      while(!it.IsAtEnd())
      {
        if (it.Get()==0 && mask->GetPixel(it.GetIndex())>0)
          OUTOFWM++;
        ++it;
      }
      MITK_INFO << "Non-white matter voxels: " << OUTOFWM;

      if (m_GmSamplesPerVoxel>0)
      {
        m_GmSamples.push_back(m_GmSamplesPerVoxel);
        m_NumberOfSamples += m_GmSamplesPerVoxel*OUTOFWM;
      }
      else if (OUTOFWM>0)
      {
        m_GmSamples.push_back(0.5+(double)wmSamples/(double)OUTOFWM);
        m_NumberOfSamples += m_GmSamples.back()*OUTOFWM;
        MITK_INFO << "Non-white matter samples per voxel: " << m_GmSamples.back();
      }
      else
      {
        m_GmSamples.push_back(0);
      }
      MITK_INFO << "Samples outside of WM: " << m_GmSamples.back()*OUTOFWM;
    }
    MITK_INFO << "Number of samples: " << m_NumberOfSamples;
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::CalculateFeaturesForTraining()
  {
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    itk::OrientationDistributionFunction< double, 2*NumberOfSignalFeatures > directions;
    std::vector< int > directionIndices;
    for (unsigned int f=0; f<2*NumberOfSignalFeatures; f++)
    {
      if (dot_product(ref, directions.GetDirection(f))>0)
        directionIndices.push_back(f);
    }

    int numDirectionFeatures = 3;

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
          for (int i=1; i<m_GmSamples.at(t); i++)
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
            for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
            {
              double a = fabs(dot_product(dir, directions.GetDirection(directionIndices[f])));
              if (a>angle)
              {
                m_LabelData(sampleCounter,0) = f;
                angle = a;
              }
            }
          }

          dirOld = dir;
          sampleCounter++;
        }
      }
    }
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::TrainForest()
  {
    MITK_INFO << "Maximum tree depths: " << m_MaxTreeDepth;
    MITK_INFO << "Sample fraction per tree: " << m_SampleFraction;
    MITK_INFO << "Number of trees: " << m_NumTrees;

    std::vector< std::shared_ptr< vigra::RandomForest<int> > > trees;
    int count = 0;
#pragma omp parallel for
    for (int i = 0; i < m_NumTrees; ++i)
    {
      std::shared_ptr< vigra::RandomForest<int> > lrf = std::make_shared< vigra::RandomForest<int> >();
      lrf->set_options().use_stratification(vigra::RF_NONE); // How the data should be made equal
      lrf->set_options().sample_with_replacement(true); // if sampled with replacement or not
      lrf->set_options().samples_per_tree(m_SampleFraction); // Fraction of samples that are used to train a tree
      lrf->set_options().tree_count(1); // Number of trees that are calculated;
      lrf->set_options().min_split_node_size(5); // Minimum number of datapoints that must be in a node
      lrf->ext_param_.max_tree_depth = m_MaxTreeDepth;

      lrf->learn(m_FeatureData, m_LabelData);
#pragma omp critical
      {
        count++;
        MITK_INFO << "Tree " << count << " finished training.";
        trees.push_back(lrf);
      }
    }

    for (int i = 1; i < m_NumTrees; ++i)
      trees.at(0)->trees_.push_back(trees.at(i)->trees_[0]);

    m_Forest = trees.at(0);
    m_Forest->options_.tree_count_ = m_NumTrees;
    MITK_INFO << "Training finsihed";
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::SaveForest(std::string forestFile)
  {
    MITK_INFO << "Saving forest to " << forestFile;
    if (IsForestValid())
      vigra::rf_export_HDF5( *m_Forest, forestFile, "" );
    else
      MITK_INFO << "Forest invalid! Could not be saved.";
    MITK_INFO << "Forest saved successfully.";
  }

  template< int ShOrder, int NumberOfSignalFeatures >
  void TrackingForestHandler< ShOrder, NumberOfSignalFeatures >::LoadForest(std::string forestFile)
  {
    MITK_INFO << "Loading forest from " << forestFile;
    m_Forest = std::make_shared< vigra::RandomForest<int> >();
    vigra::rf_import_HDF5( *m_Forest, forestFile);
  }
}

#endif