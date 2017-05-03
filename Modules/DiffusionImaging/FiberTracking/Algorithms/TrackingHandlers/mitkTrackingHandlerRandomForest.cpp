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

#include "mitkTrackingHandlerRandomForest.h"
#include <itkTractDensityImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>

namespace mitk
{
template< int ShOrder, int NumberOfSignalFeatures >
TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::TrackingHandlerRandomForest()
  : m_WmSampleDistance(-1)
  , m_NumTrees(30)
  , m_MaxTreeDepth(25)
  , m_SampleFraction(1.0)
  , m_NumberOfSamples(0)
  , m_GmSamplesPerVoxel(-1)
  , m_NumPreviousDirections(1)
  , m_ZeroDirWmFeatures(true)
  , m_BidirectionalFiberSampling(false)
  , m_MaxNumWmSamples(-1)
{
  vnl_vector_fixed<float,3> ref; ref.fill(0); ref[0]=1;

  itk::OrientationDistributionFunction< float, 200 > odf;
  m_DirectionContainer.clear();
  for (unsigned int i = 0; i<odf.GetNumberOfComponents(); i++)
  {
    vnl_vector_fixed<float,3> odf_dir;
    odf_dir[0] = odf.GetDirection(i)[0];
    odf_dir[1] = odf.GetDirection(i)[1];
    odf_dir[2] = odf.GetDirection(i)[2];
    if (dot_product(ref, odf_dir)>0)            // only used directions on one hemisphere
      m_DirectionContainer.push_back(odf_dir);  // store indices for later mapping the classifier output to the actual direction
  }
}

template< int ShOrder, int NumberOfSignalFeatures >
TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::~TrackingHandlerRandomForest()
{
}

template< int ShOrder, int NumberOfSignalFeatures >
typename TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::DwiFeatureImageType::PixelType TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::GetDwiFeaturesAtPosition(itk::Point<float, 3> itkP, typename DwiFeatureImageType::Pointer image, bool interpolate)
{
  // transform physical point to index coordinates
  itk::Index<3> idx;
  itk::ContinuousIndex< float, 3> cIdx;
  image->TransformPhysicalPointToIndex(itkP, idx);
  image->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

  typename DwiFeatureImageType::PixelType pix; pix.Fill(0.0);
  if ( image->GetLargestPossibleRegion().IsInside(idx) )
  {
    pix = image->GetPixel(idx);
    if (!interpolate)
      return pix;
  }
  else
    return pix;

  float frac_x = cIdx[0] - idx[0];
  float frac_y = cIdx[1] - idx[1];
  float frac_z = cIdx[2] - idx[2];
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
    vnl_vector_fixed<float, 8> interpWeights;
    interpWeights[0] = (  frac_x)*(  frac_y)*(  frac_z);
    interpWeights[1] = (1-frac_x)*(  frac_y)*(  frac_z);
    interpWeights[2] = (  frac_x)*(1-frac_y)*(  frac_z);
    interpWeights[3] = (  frac_x)*(  frac_y)*(1-frac_z);
    interpWeights[4] = (1-frac_x)*(1-frac_y)*(  frac_z);
    interpWeights[5] = (  frac_x)*(1-frac_y)*(1-frac_z);
    interpWeights[6] = (1-frac_x)*(  frac_y)*(1-frac_z);
    interpWeights[7] = (1-frac_x)*(1-frac_y)*(1-frac_z);

    pix = image->GetPixel(idx) * interpWeights[0];
    typename DwiFeatureImageType::IndexType tmpIdx = idx; tmpIdx[0]++;
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
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InputDataValidForTracking()
{
  if (m_InputDwis.empty())
    mitkThrow() << "No diffusion-weighted images set!";
  if (!IsForestValid())
    mitkThrow() << "No or invalid random forest detected!";
}

template< int ShOrder, int NumberOfSignalFeatures>
template<typename T>
typename std::enable_if< NumberOfSignalFeatures <=99, T >::type TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InitDwiImageFeatures(mitk::Image::Pointer mitk_dwi)
{
  MITK_INFO << "Calculating spherical harmonics features";
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,ShOrder, 2*NumberOfSignalFeatures> InterpolationFilterType;

  typename InterpolationFilterType::Pointer filter = InterpolationFilterType::New();
  filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(mitk_dwi), mitk::DiffusionPropertyHelper::GetItkVectorImage(mitk_dwi) );
  filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(mitk_dwi));
  filter->SetLambda(0.006);
  filter->SetNormalizationMethod(InterpolationFilterType::QBAR_RAW_SIGNAL);
  filter->Update();

  m_DwiFeatureImages.push_back(filter->GetCoefficientImage());
  return true;
}

template< int ShOrder, int NumberOfSignalFeatures>
template<typename T>
typename std::enable_if< NumberOfSignalFeatures >=100, T >::type TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InitDwiImageFeatures(mitk::Image::Pointer mitk_dwi)
{
  MITK_INFO << "Interpolating raw dwi signal features";
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,ShOrder, 2*NumberOfSignalFeatures> InterpolationFilterType;

  typename InterpolationFilterType::Pointer filter = InterpolationFilterType::New();
  filter->SetGradientImage( mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(mitk_dwi), mitk::DiffusionPropertyHelper::GetItkVectorImage(mitk_dwi) );
  filter->SetBValue(mitk::DiffusionPropertyHelper::GetReferenceBValue(mitk_dwi));
  filter->SetLambda(0.006);
  filter->SetNormalizationMethod(InterpolationFilterType::QBAR_RAW_SIGNAL);
  filter->Update();

  typename DwiFeatureImageType::Pointer dwiFeatureImage = DwiFeatureImageType::New();
  dwiFeatureImage->SetSpacing(filter->GetOutput()->GetSpacing());
  dwiFeatureImage->SetOrigin(filter->GetOutput()->GetOrigin());
  dwiFeatureImage->SetDirection(filter->GetOutput()->GetDirection());
  dwiFeatureImage->SetLargestPossibleRegion(filter->GetOutput()->GetLargestPossibleRegion());
  dwiFeatureImage->SetBufferedRegion(filter->GetOutput()->GetLargestPossibleRegion());
  dwiFeatureImage->SetRequestedRegion(filter->GetOutput()->GetLargestPossibleRegion());
  dwiFeatureImage->Allocate();

  // get signal values and store them in the feature image
  vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
  itk::OrientationDistributionFunction< float, 2*NumberOfSignalFeatures > odf;
  itk::ImageRegionIterator< typename InterpolationFilterType::OutputImageType > it(filter->GetOutput(), filter->GetOutput()->GetLargestPossibleRegion());
  while(!it.IsAtEnd())
  {
    typename DwiFeatureImageType::PixelType pix;
    int f = 0;
    for (unsigned int i = 0; i<odf.GetNumberOfComponents(); i++)
    {
      if (dot_product(ref, odf.GetDirection(i))>0)            // only used directions on one hemisphere
      {
        pix[f] = it.Get()[i];
        f++;
      }
    }
    dwiFeatureImage->SetPixel(it.GetIndex(), pix);
    ++it;
  }
  m_DwiFeatureImages.push_back(dwiFeatureImage);
  return true;
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InitForTracking()
{
  MITK_INFO << "Initializing random forest tracker.";
  InputDataValidForTracking();
  m_DwiFeatureImages.clear();
  InitDwiImageFeatures<>(m_InputDwis.at(0));
}

template< int ShOrder, int NumberOfSignalFeatures >
vnl_vector_fixed<float,3> TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::ProposeDirection(itk::Point<float, 3>& pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3>& oldIndex)
{

  vnl_vector_fixed<float,3> output_direction; output_direction.fill(0);

  itk::Index<3> idx;
  m_DwiFeatureImages.at(0)->TransformPhysicalPointToIndex(pos, idx);

  vnl_vector_fixed<float,3> last_dir;
  if (!olddirs.empty())
    last_dir = olddirs.back();

  if (!m_Interpolate && oldIndex==idx)
    return last_dir;

  // store feature pixel values in a vigra data type
  vigra::MultiArray<2, float> featureData = vigra::MultiArray<2, float>( vigra::Shape2(1,m_Forest->feature_count()) );
  typename DwiFeatureImageType::PixelType dwiFeaturePixel = GetDwiFeaturesAtPosition(pos, m_DwiFeatureImages.at(0), m_Interpolate);
  for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
    featureData(0,f) = dwiFeaturePixel[f];

  vnl_matrix_fixed<double,3,3> direction_matrix = m_DwiFeatureImages.at(0)->GetDirection().GetVnlMatrix();
  vnl_matrix_fixed<double,3,3> inverse_direction_matrix = m_DwiFeatureImages.at(0)->GetInverseDirection().GetVnlMatrix();

  // append normalized previous direction(s) to feature vector
  int i = 0;
  vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;

  if (m_FlipX)
      last_dir[0] *= -1;
  if (m_FlipY)
      last_dir[1] *= -1;
  if (m_FlipZ)
      last_dir[2] *= -1;

  for (auto d : olddirs)
  {
    vnl_vector_fixed<double,3> tempD;
    tempD[0] = d[0]; tempD[1] = d[1]; tempD[2] = d[2];
    tempD = inverse_direction_matrix * tempD;
    last_dir[0] = tempD[0];
    last_dir[1] = tempD[1];
    last_dir[2] = tempD[2];

    int c = 0;
    for (unsigned int f=NumberOfSignalFeatures+3*i; f<NumberOfSignalFeatures+3*(i+1); f++)
    {
      if (dot_product(ref, tempD)<0)
        featureData(0,f) = -tempD[c];
      else
        featureData(0,f) = tempD[c];
      c++;
    }
    i++;
  }

  // additional feature images
  if (m_AdditionalFeatureImages.size()>0)
  {
    int c = 0;
    for (auto img : m_AdditionalFeatureImages.at(0))
    {
      float v = GetImageValue<float>(pos, img, false);
      featureData(0,NumberOfSignalFeatures+m_NumPreviousDirections*3+c) = v;
      c++;
    }
  }

  // perform classification
  vigra::MultiArray<2, float> probs(vigra::Shape2(1, m_Forest->class_count()));
  m_Forest->predictProbabilities(featureData, probs);

  float pNonFib = 0;     // probability that we left the white matter
  float w = 0;           // weight of the predicted direction
  for (int i=0; i<m_Forest->class_count(); i++)   // for each class (number of possible directions + out-of-wm class)
  {
    if (probs(0,i)>0)   // if probability of respective class is 0, do nothing
    {
      // get label of class (does not correspond to the loop variable i)
      int classLabel = 0;
      m_Forest->ext_param_.to_classlabel(i, classLabel);

      if (classLabel<m_DirectionContainer.size())   // does class label correspond to a direction or to the out-of-wm class?
      {
        vnl_vector_fixed<float,3> d = m_DirectionContainer.at(classLabel);  // get direction vector assiciated with the respective direction index

        if (!olddirs.empty() && last_dir.magnitude()>0.5)   // do we have a previous streamline direction or did we just start?
        {
          // TODO: check if hard curvature threshold is necessary.
          // alternatively try square of dot pruduct as weight.
          // TODO: check if additional weighting with dot product as directional prior is necessary. are there alternatives on the classification level?

          float dot = dot_product(d, last_dir);    // claculate angle between the candidate direction vector and the previous streamline direction
          if (fabs(dot)>m_AngularThreshold)         // is angle between the directions smaller than our hard threshold?
          {
            if (dot<0)                          // make sure we don't walk backwards
              d *= -1;
            float w_i = probs(0,i)*fabs(dot);
            output_direction += w_i*d; // weight contribution to output direction with its probability and the angular deviation from the previous direction
            w += w_i;           // increase output weight of the final direction
          }
        }
        else
        {
          output_direction += probs(0,i)*d;
          w += probs(0,i);
        }
      }
      else
        pNonFib += probs(0,i);  // probability that we are not in the white matter anymore
    }
  }

  // if we did not find a suitable direction, make sure that we return (0,0,0)
  if (pNonFib>w && w>0)
    output_direction.fill(0.0);
  else
  {
    vnl_vector_fixed<double,3> tempD;
    tempD[0] = output_direction[0]; tempD[1] = output_direction[1]; tempD[2] = output_direction[2];
    tempD = direction_matrix * tempD;
    output_direction[0] = tempD[0];
    output_direction[1] = tempD[1];
    output_direction[2] = tempD[2];

    if (m_FlipX)
      output_direction[0] *= -1;
    if (m_FlipY)
      output_direction[1] *= -1;
    if (m_FlipZ)
      output_direction[2] *= -1;
  }

  return output_direction * w;
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::StartTraining()
{
  m_StartTime = std::chrono::system_clock::now();
  InputDataValidForTraining();
  InitForTraining();
  CalculateTrainingSamples();

  MITK_INFO << "Maximum tree depths: " << m_MaxTreeDepth;
  MITK_INFO << "Sample fraction per tree: " << m_SampleFraction;
  MITK_INFO << "Number of trees: " << m_NumTrees;

  DefaultSplitType splitter;
  splitter.UsePointBasedWeights(true);
  splitter.SetWeights(m_Weights);
  splitter.UseRandomSplit(false);
  splitter.SetPrecision(mitk::eps);
  splitter.SetMaximumTreeDepth(m_MaxTreeDepth);

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

    lrf->learn(m_FeatureData, m_LabelData,vigra::rf::visitors::VisitorBase(),splitter);
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

  m_EndTime = std::chrono::system_clock::now();
  std::chrono::hours   hh = std::chrono::duration_cast<std::chrono::hours>(m_EndTime - m_StartTime);
  std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(m_EndTime - m_StartTime);
  mm %= 60;
  MITK_INFO << "Training took " << hh.count() << "h and " << mm.count() << "m";
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InputDataValidForTraining()
{
  if (m_InputDwis.empty())
    mitkThrow() << "No diffusion-weighted images set!";
  if (m_Tractograms.empty())
    mitkThrow() << "No tractograms set!";
  if (m_InputDwis.size()!=m_Tractograms.size())
    mitkThrow() << "Unequal number of diffusion-weighted images and tractograms detected!";
}

template< int ShOrder, int NumberOfSignalFeatures >
bool TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::IsForestValid()
{
  int additional_features = 0;
  if (m_AdditionalFeatureImages.size()>0)
    additional_features = m_AdditionalFeatureImages.at(0).size();
  if(m_Forest && m_Forest->tree_count()>0 && m_Forest->feature_count()==(NumberOfSignalFeatures+3*m_NumPreviousDirections+additional_features))
    return true;
  return false;
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::InitForTraining()
{
  typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,ShOrder, 2*NumberOfSignalFeatures> InterpolationFilterType;

  MITK_INFO << "Spherical signal interpolation and sampling ...";
  for (unsigned int i=0; i<m_InputDwis.size(); i++)
  {
    InitDwiImageFeatures<>(m_InputDwis.at(i));

    if (i>=m_AdditionalFeatureImages.size())
    {
      m_AdditionalFeatureImages.push_back(std::vector< ItkFloatImgType::Pointer >());
    }

    if (i>=m_FiberVolumeModImages.size())
    {
      ItkFloatImgType::Pointer img = ItkFloatImgType::New();
      img->SetSpacing( m_DwiFeatureImages.at(i)->GetSpacing() );
      img->SetOrigin( m_DwiFeatureImages.at(i)->GetOrigin() );
      img->SetDirection( m_DwiFeatureImages.at(i)->GetDirection() );
      img->SetLargestPossibleRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      img->SetBufferedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      img->SetRequestedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      img->Allocate();
      img->FillBuffer(1);
      m_FiberVolumeModImages.push_back(img);
    }

    if (m_FiberVolumeModImages.at(i)==nullptr)
    {
      m_FiberVolumeModImages.at(i) = ItkFloatImgType::New();
      m_FiberVolumeModImages.at(i)->SetSpacing( m_DwiFeatureImages.at(i)->GetSpacing() );
      m_FiberVolumeModImages.at(i)->SetOrigin( m_DwiFeatureImages.at(i)->GetOrigin() );
      m_FiberVolumeModImages.at(i)->SetDirection( m_DwiFeatureImages.at(i)->GetDirection() );
      m_FiberVolumeModImages.at(i)->SetLargestPossibleRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_FiberVolumeModImages.at(i)->SetBufferedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_FiberVolumeModImages.at(i)->SetRequestedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_FiberVolumeModImages.at(i)->Allocate();
      m_FiberVolumeModImages.at(i)->FillBuffer(1);
    }

    if (i>=m_MaskImages.size())
    {
      ItkUcharImgType::Pointer newMask = ItkUcharImgType::New();
      newMask->SetSpacing( m_DwiFeatureImages.at(i)->GetSpacing() );
      newMask->SetOrigin( m_DwiFeatureImages.at(i)->GetOrigin() );
      newMask->SetDirection( m_DwiFeatureImages.at(i)->GetDirection() );
      newMask->SetLargestPossibleRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      newMask->SetBufferedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      newMask->SetRequestedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      newMask->Allocate();
      newMask->FillBuffer(1);
      m_MaskImages.push_back(newMask);
    }

    if (m_MaskImages.at(i)==nullptr)
    {
      m_MaskImages.at(i) = ItkUcharImgType::New();
      m_MaskImages.at(i)->SetSpacing( m_DwiFeatureImages.at(i)->GetSpacing() );
      m_MaskImages.at(i)->SetOrigin( m_DwiFeatureImages.at(i)->GetOrigin() );
      m_MaskImages.at(i)->SetDirection( m_DwiFeatureImages.at(i)->GetDirection() );
      m_MaskImages.at(i)->SetLargestPossibleRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_MaskImages.at(i)->SetBufferedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_MaskImages.at(i)->SetRequestedRegion( m_DwiFeatureImages.at(i)->GetLargestPossibleRegion() );
      m_MaskImages.at(i)->Allocate();
      m_MaskImages.at(i)->FillBuffer(1);
    }
  }

  MITK_INFO << "Resampling fibers and calculating number of samples ...";
  m_NumberOfSamples = 0;
  m_SampleUsage.clear();
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
      typename DwiFeatureImageType::Pointer image = m_DwiFeatureImages.at(t);
      float minSpacing = 1;
      if(image->GetSpacing()[0]<image->GetSpacing()[1] && image->GetSpacing()[0]<image->GetSpacing()[2])
        minSpacing = image->GetSpacing()[0];
      else if (image->GetSpacing()[1] < image->GetSpacing()[2])
        minSpacing = image->GetSpacing()[1];
      else
        minSpacing = image->GetSpacing()[2];
      m_WmSampleDistance = minSpacing*0.5;
    }

    m_Tractograms.at(t)->ResampleLinear(m_WmSampleDistance);

    unsigned int wmSamples = m_Tractograms.at(t)->GetNumberOfPoints()-2*m_Tractograms.at(t)->GetNumFibers();
    if (m_BidirectionalFiberSampling)
      wmSamples *= 2;
    if (m_ZeroDirWmFeatures)
      wmSamples *= (m_NumPreviousDirections+1);

    MITK_INFO << "White matter samples available: " << wmSamples;

    // upper limit for samples
    if (m_MaxNumWmSamples>0 && wmSamples>m_MaxNumWmSamples)
    {
      if ((float)m_MaxNumWmSamples/wmSamples > 0.8)
      {
        m_SampleUsage.push_back(std::vector<bool>(wmSamples, true));
        m_NumberOfSamples += wmSamples;
      }
      else
      {
        m_SampleUsage.push_back(std::vector<bool>(wmSamples, false));
        m_NumberOfSamples += m_MaxNumWmSamples;
        wmSamples = m_MaxNumWmSamples;
        MITK_INFO << "Limiting white matter samples to: " << m_MaxNumWmSamples;

        itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randgen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
        randgen->SetSeed();
        unsigned int c = 0;
        while (c<m_MaxNumWmSamples)
        {
          int idx = randgen->GetIntegerVariate(m_MaxNumWmSamples-1);
          if (m_SampleUsage[t][idx]==false)
          {
            m_SampleUsage[t][idx]=true;
            c++;
          }
        }
      }
    }
    else
    {
      m_SampleUsage.push_back(std::vector<bool>(wmSamples, true));
      m_NumberOfSamples += wmSamples;
    }

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
      int gm_per_voxel = 0.5+(float)wmSamples/(float)OUTOFWM;
      if (gm_per_voxel<=0)
        gm_per_voxel = 1;
      m_GmSamples.push_back(gm_per_voxel);
      m_NumberOfSamples += m_GmSamples.back()*OUTOFWM;
      MITK_INFO << "Non-white matter samples per voxel: " << m_GmSamples.back();
    }
    else
    {
      m_GmSamples.push_back(0);
    }
    MITK_INFO << "Non-white matter samples: " << m_GmSamples.back()*OUTOFWM;
  }
  MITK_INFO << "Number of samples: " << m_NumberOfSamples;
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::CalculateTrainingSamples()
{
  vnl_vector_fixed<float,3> ref; ref.fill(0); ref[0]=1;

  m_FeatureData.reshape( vigra::Shape2(m_NumberOfSamples, NumberOfSignalFeatures+m_NumPreviousDirections*3+m_AdditionalFeatureImages.at(0).size()) );
  m_LabelData.reshape( vigra::Shape2(m_NumberOfSamples,1) );
  m_Weights.reshape( vigra::Shape2(m_NumberOfSamples,1) );
  MITK_INFO << "Number of features: " << m_FeatureData.shape(1);

  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  m_RandGen->SetSeed();
  MITK_INFO <<  "Creating training data ...";
  unsigned int sampleCounter = 0;
  for (unsigned int t=0; t<m_Tractograms.size(); t++)
  {
    ItkFloatImgType::Pointer fiber_folume = m_FiberVolumeModImages.at(t);
    typename DwiFeatureImageType::Pointer image = m_DwiFeatureImages.at(t);
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
        typename DwiFeatureImageType::PixelType pix = image->GetPixel(it.GetIndex());

        // random directions
        for (unsigned int i=0; i<m_GmSamples.at(t); i++)
        {
          // diffusion signal features
          for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
          {
            m_FeatureData(sampleCounter,f) = pix[f];
            if(m_FeatureData(sampleCounter,f)!=m_FeatureData(sampleCounter,f))
              m_FeatureData(sampleCounter,f) = 0;
          }

          // direction feature
          int num_zero_dirs = m_RandGen->GetIntegerVariate(m_NumPreviousDirections);  // how many directions should be zero?
          for (unsigned int i=0; i<m_NumPreviousDirections; i++)
          {
            int c=0;
            vnl_vector_fixed<float,3> probe;
            if (i<num_zero_dirs)
              probe.fill(0.0);
            else
            {
              probe[0] = m_RandGen->GetVariate()*2-1;
              probe[1] = m_RandGen->GetVariate()*2-1;
              probe[2] = m_RandGen->GetVariate()*2-1;
              probe.normalize();
              if (dot_product(ref, probe)<0)
                probe *= -1;
            }
            for (unsigned int f=NumberOfSignalFeatures+3*i; f<NumberOfSignalFeatures+3*(i+1); f++)
            {
              m_FeatureData(sampleCounter,f) = probe[c];
              c++;
            }
          }

          // additional feature images
          int add_feat_c = 0;
          for (auto img : m_AdditionalFeatureImages.at(t))
          {
            itk::Point<float, 3> itkP;
            image->TransformIndexToPhysicalPoint(it.GetIndex(), itkP);
            float v = GetImageValue<float>(itkP, img, false);
            m_FeatureData(sampleCounter,NumberOfSignalFeatures+m_NumPreviousDirections*3+add_feat_c) = v;
            add_feat_c++;
          }

          // label and sample weight
          m_LabelData(sampleCounter,0) = m_DirectionContainer.size();
          m_Weights(sampleCounter,0) = 1.0;
          sampleCounter++;
        }
      }
      ++it;
    }

    unsigned int num_gm_samples = sampleCounter;
    MITK_INFO << "WM samples...";
    // white matter samples
    mitk::FiberBundle::Pointer fib = m_Tractograms.at(t);
    vtkSmartPointer< vtkPolyData > polyData = fib->GetFiberPolyData();
    vnl_vector_fixed<float,3> zero_dir; zero_dir.fill(0.0);
    for (int i=0; i<fib->GetNumFibers(); i++)
    {
      vtkCell* cell = polyData->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();
      float fiber_weight = fib->GetFiberWeight(i);

      for (int n = 0; n<=m_NumPreviousDirections; n++)
      {
        if (!m_ZeroDirWmFeatures)
          n = m_NumPreviousDirections;
        for (bool reverse : {false, true})
        {
          for (int j=1; j<numPoints-1; j++)
          {
            if (!m_SampleUsage[t][sampleCounter-num_gm_samples])
              continue;

            itk::Point<float, 3> itkP1, itkP2;

            int num_nonzero_dirs = m_NumPreviousDirections;
            if (!reverse)
              num_nonzero_dirs = std::min(n, j);
            else
              num_nonzero_dirs = std::min(n, numPoints-j-1);

            vnl_vector_fixed<float,3> dir;
            // zero directions
            for (unsigned int k=0; k<m_NumPreviousDirections-num_nonzero_dirs; k++)
            {
              dir.fill(0.0);
              int c = 0;
              for (unsigned int f=NumberOfSignalFeatures+3*k; f<NumberOfSignalFeatures+3*(k+1); f++)
              {
                m_FeatureData(sampleCounter,f) = dir[c];
                c++;
              }
            }

            // nonzero directions
            for (unsigned int k=0; k<num_nonzero_dirs; k++)
            {
              double* p = nullptr;
              int n_idx = num_nonzero_dirs-k;
              if (!reverse)
              {
                p = points->GetPoint(j-n_idx);
                itkP1[0] = p[0]; itkP1[1] = p[1]; itkP1[2] = p[2];
                p = points->GetPoint(j-n_idx+1);
                itkP2[0] = p[0]; itkP2[1] = p[1]; itkP2[2] = p[2];
              }
              else
              {
                p = points->GetPoint(j+n_idx);
                itkP1[0] = p[0]; itkP1[1] = p[1]; itkP1[2] = p[2];
                p = points->GetPoint(j+n_idx-1);
                itkP2[0] = p[0]; itkP2[1] = p[1]; itkP2[2] = p[2];
              }

              dir[0]=itkP1[0]-itkP2[0];
              dir[1]=itkP1[1]-itkP2[1];
              dir[2]=itkP1[2]-itkP2[2];

              if (dir.magnitude()<0.0001)
                mitkThrow() << "streamline error!";
              dir.normalize();
              if (dir[0]!=dir[0] || dir[1]!=dir[1] || dir[2]!=dir[2])
                mitkThrow() << "ERROR: NaN direction!";

              if (dot_product(ref, dir)<0)
                dir *= -1;

              int c = 0;
              for (unsigned int f=NumberOfSignalFeatures+3*(k+m_NumPreviousDirections-num_nonzero_dirs); f<NumberOfSignalFeatures+3*(k+1+m_NumPreviousDirections-num_nonzero_dirs); f++)
              {
                m_FeatureData(sampleCounter,f) = dir[c];
                c++;
              }
            }

            // get target direction
            double* p = points->GetPoint(j);
            itkP1[0] = p[0]; itkP1[1] = p[1]; itkP1[2] = p[2];
            if (reverse)
            {
              p = points->GetPoint(j-1);
              itkP2[0] = p[0]; itkP2[1] = p[1]; itkP2[2] = p[2];
            }
            else
            {
              p = points->GetPoint(j+1);
              itkP2[0] = p[0]; itkP2[1] = p[1]; itkP2[2] = p[2];
            }
            dir[0]=itkP2[0]-itkP1[0];
            dir[1]=itkP2[1]-itkP1[1];
            dir[2]=itkP2[2]-itkP1[2];

            if (dir.magnitude()<0.0001)
              mitkThrow() << "streamline error!";
            dir.normalize();
            if (dir[0]!=dir[0] || dir[1]!=dir[1] || dir[2]!=dir[2])
              mitkThrow() << "ERROR: NaN direction!";
            if (dot_product(ref, dir)<0)
              dir *= -1;

            // image features
            float volume_mod = GetImageValue<float>(itkP1, fiber_folume, false);

            // diffusion signal features
            typename DwiFeatureImageType::PixelType pix = GetDwiFeaturesAtPosition(itkP1, image, m_Interpolate);
            for (unsigned int f=0; f<NumberOfSignalFeatures; f++)
              m_FeatureData(sampleCounter,f) = pix[f];

            // additional feature images
            int add_feat_c = 0;
            for (auto img : m_AdditionalFeatureImages.at(t))
            {
              float v = GetImageValue<float>(itkP1, img, false);
              add_feat_c++;
              m_FeatureData(sampleCounter,NumberOfSignalFeatures+2+add_feat_c) = v;
            }

            // set label values
            float angle = 0;
            float m = dir.magnitude();
            if (m>0.0001)
            {
              int l = 0;
              for (auto d : m_DirectionContainer)
              {
                float a = fabs(dot_product(dir, d));
                if (a>angle)
                {
                  m_LabelData(sampleCounter,0) = l;
                  m_Weights(sampleCounter,0) = fiber_weight*volume_mod;
                  angle = a;
                }
                l++;
              }
            }

            sampleCounter++;
          }
          if (!m_BidirectionalFiberSampling)  // don't sample fibers backward
            break;
        }
      }
    }
  }
  m_Tractograms.clear();
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::SaveForest(std::string forestFile)
{
  MITK_INFO << "Saving forest to " << forestFile;
  if (IsForestValid())
  {
    vigra::rf_export_HDF5( *m_Forest, forestFile, "" );
    MITK_INFO << "Forest saved successfully.";
  }
  else
    MITK_INFO << "Forest invalid! Could not be saved.";
}

template< int ShOrder, int NumberOfSignalFeatures >
void TrackingHandlerRandomForest< ShOrder, NumberOfSignalFeatures >::LoadForest(std::string forestFile)
{
  MITK_INFO << "Loading forest from " << forestFile;
  m_Forest = std::make_shared< vigra::RandomForest<int> >();
  vigra::rf_import_HDF5( *m_Forest, forestFile);
}
}

#endif
