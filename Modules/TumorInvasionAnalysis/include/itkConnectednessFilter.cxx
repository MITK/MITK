#pragma warning (disable : 4996)

#ifndef __ConnectednessFilter_hxx
#define __ConnectednessFilter_hxx

#include "itkConnectednessFilter.h"
#include "itkObjectFactory.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include <itkImageRegionConstIteratorWithIndex.h>
// Vector Fields
#include "itkVectorImage.h"
#include <vnl/vnl_vector.h>
//
#include "itkNeighborhoodIterator.h"
#include "itkConstantBoundaryCondition.h"

namespace itk
{
template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetInputImage(const TFeatureImage *image)
{
  this->SetNthInput(0, const_cast<TFeatureImage*>(image));
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetInputSeed(const TSeedImage *image)
{
  this->SetNthInput(1, const_cast<TSeedImage*>(image));
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetInputMask(const TSeedImage *image)
{
  this->SetNthInput(2, const_cast<TSeedImage*>(image));
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetInputVectorFieldConfidenceMap(const TFeatureImage *conf)
{
  // DO NOT set it as nth input, since it is dti derived and therefore does
  // not necessarily occupy the same space as the other images
  m_ConfidenceImage = const_cast<TFeatureImage*>(conf);
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetPropagationImage(const TFeatureImage *image)
{
  this->SetNthInput(3, const_cast<TFeatureImage*>(image));
}


template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
typename TFeatureImage::Pointer ConnectednessFilter<TFeatureImage, TSeedImage, TTensorImage>::GetDistanceImage()
{
  return m_DistanceImage;
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
typename TFeatureImage::Pointer ConnectednessFilter<TFeatureImage, TSeedImage, TTensorImage>::GetEuclideanDistanceImage()
{
  return m_EuclideanDistance;
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::SetInputVectorField(const TTensorImage *vecs)
{
  // DO NOT set it as nth input, since it is dti derived and therefore does
  // not necessary occupy the same space as the other images
  m_TensorImage = const_cast<TTensorImage*>(vecs);
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
void ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::GenerateData()
{
  const char untouchedValue = -10;
  const char inListValue = -30;
  // Get Inputs
  m_InputImage = dynamic_cast <TFeatureImage*> (this->ProcessObject::GetInput(0));
  typename TSeedImage::Pointer seed = dynamic_cast <TSeedImage*> (this->ProcessObject::GetInput(1));
  typename TSeedImage::Pointer mask = dynamic_cast <TSeedImage*> (this->ProcessObject::GetInput(2));
  typename TFeatureImage::Pointer propagationImage = dynamic_cast <TFeatureImage*> (this->ProcessObject::GetInput(3));

  if (propagationImage.IsNull())
    propagationImage = m_InputImage;

  // ---
  // Allocate Output
  typename TFeatureImage::Pointer output = this->GetOutput();
  output->SetRegions(m_InputImage->GetLargestPossibleRegion());
  output->Allocate();
  output->FillBuffer(untouchedValue);

  m_DistanceImage = TFeatureImage::New();
  m_DistanceImage->SetRegions(m_InputImage->GetLargestPossibleRegion());
  m_DistanceImage->Allocate();
  m_DistanceImage->FillBuffer(untouchedValue);
  m_DistanceImage->SetOrigin(m_InputImage->GetOrigin());
  m_DistanceImage->SetDirection(m_InputImage->GetDirection());
  m_DistanceImage->SetSpacing(m_InputImage->GetSpacing());

  m_EuclideanDistance = TFeatureImage::New();
  m_EuclideanDistance->SetRegions(m_InputImage->GetLargestPossibleRegion());
  m_EuclideanDistance->Allocate();
  m_EuclideanDistance->FillBuffer(0.0);
  m_EuclideanDistance->SetOrigin(m_InputImage->GetOrigin());
  m_EuclideanDistance->SetDirection(m_InputImage->GetDirection());
  m_EuclideanDistance->SetSpacing(m_InputImage->GetSpacing());

  // Helper Object - Indicates which voxels are currently in the FIFO,
  // this prevents excessive memory and comptutational overhead
  typename TFeatureImage::Pointer activeSeeds = TFeatureImage::New();
  activeSeeds->SetRegions(m_InputImage->GetLargestPossibleRegion());
  activeSeeds->Allocate();
  activeSeeds->FillBuffer(untouchedValue);

  // Create Iterators
  SeedIteratorType seedIt(seed, seed->GetLargestPossibleRegion());
  FeatureIteratorType outIt(output, output->GetLargestPossibleRegion());
  FeatureIteratorType inputIt(m_InputImage, m_InputImage->GetLargestPossibleRegion());

  // Set up Neighborhood Iterator to use a 3x3x3 neighborhood
  typename itk::NeighborhoodIterator<TSeedImage>::RadiusType radius;
  radius.Fill(1);
  radius[2]=1;

  typename itk::NeighborhoodIterator<TSeedImage> neighbIt  ( radius, seed, seed->GetRequestedRegion() );
  typename itk::NeighborhoodIterator<TFeatureImage> medianIt  ( radius, propagationImage, propagationImage->GetRequestedRegion() );

  // Copy Input Image values from SeedRegion values into Output Image
  // Collect border voxel indices to initialize seed list
  typedef std::list<IndexType> ContainerType;
  ContainerType fifo;
  while (!seedIt.IsAtEnd())
  {
    if (seedIt.Get() != 0)
    {
      outIt.Set(inputIt.Get());
      if (activeSeeds->GetPixel(seedIt.GetIndex()) == untouchedValue)
      {
        neighbIt.SetLocation(seedIt.GetIndex());
        for (unsigned int i = 0; i < neighbIt.Size(); ++i)
        {
          bool isInside = true;
          neighbIt.GetPixel(i, isInside);
          if (!isInside)
            continue;
          if (neighbIt.GetPixel(i) == 0 ) // border voxel
          {
            // add index of border voxel into FIFO
            fifo.push_back(seedIt.GetIndex());
            activeSeeds->SetPixel( seedIt.GetIndex(), inListValue);
            //write propagation value into output image

            if (m_ApplyRankFilter)
            {
              // Calculate median in 3x3x3 neighborhood and use this value to propagate
              medianIt.SetLocation(seedIt.GetIndex());
              std::vector<FeaturePixelType> samples;
              for (unsigned int j = 0; j < medianIt.Size(); ++j)
              {
                if (seed->GetPixel(medianIt.GetIndex(j)) != 0 ) // inside seed region
                  samples.push_back(medianIt.GetPixel(j));
              }
              std::sort (samples.begin(), samples.end());

              output->SetPixel(seedIt.GetIndex(), samples.at(samples.size()/2));
            }
            else
              output->SetPixel(seedIt.GetIndex(), propagationImage->GetPixel(seedIt.GetIndex()));

            m_DistanceImage->SetPixel(seedIt.GetIndex(), 0);
            break;
          }

        }
      }
    }

    ++inputIt;
    ++outIt;
    ++seedIt;
  }
  //
  // Now iterate over items in fifo and check all possible paths starting from these indices
  // to their neighbors and mark those with lowest cost
  while (!fifo.empty()) {
    IndexType index = fifo.front();
    fifo.pop_front();
    // mark voxel as no longer present in fifo
    activeSeeds->SetPixel(index,untouchedValue);

    if (mask->GetPixel(index) == 0)
      continue;

    neighbIt.SetLocation(index);

    FeaturePixelType currDistance = m_DistanceImage->GetPixel(index);
    FeaturePixelType currEuclidDistance = m_EuclideanDistance->GetPixel(index);
    FeaturePixelType propagateValue = output->GetPixel(index);

    // compute cost to get to each neighborhood voxel
    // if we find a lower way to a neighbor voxel, that voxel gets appended to the fifo
    for (unsigned int i = 0; i < neighbIt.Size(); ++i)
    {
      bool isInside = true;
      neighbIt.GetPixel(i, isInside);
      if ( !isInside|| mask->GetPixel(neighbIt.GetIndex(i)) == 0)// || seed->GetPixel(neighbIt.GetIndex(i)) != 0)
        continue;

      if (i == neighbIt.Size()/2) // skip center voxel
        continue;

      FeaturePixelType cost = GetDistanceValue(index, neighbIt.GetIndex(i));
      if (cost == -1)
        continue;
      if (currDistance + cost < m_DistanceImage->GetPixel(neighbIt.GetIndex(i)) || m_DistanceImage->GetPixel(neighbIt.GetIndex(i)) == untouchedValue)
      {
        m_DistanceImage->SetPixel(neighbIt.GetIndex(i), currDistance + cost);

        // Compute Euclidean distance between indices
        FeaturePixelType eDist = sqrt( std::pow(index[0]-neighbIt.GetIndex(i)[0],2) * m_InputImage->GetSpacing()[0] +
            std::pow(index[1]-neighbIt.GetIndex(i)[1],2) * m_InputImage->GetSpacing()[1] +
            std::pow(index[2]-neighbIt.GetIndex(i)[2],2) * m_InputImage->GetSpacing()[2]
            );

        m_EuclideanDistance->SetPixel(neighbIt.GetIndex(i), currEuclidDistance + eDist);


        output->SetPixel(neighbIt.GetIndex(i), propagateValue);
        if (activeSeeds->GetPixel(neighbIt.GetIndex(i)) == untouchedValue)
        {
          fifo.push_back(neighbIt.GetIndex(i));
          activeSeeds->SetPixel(neighbIt.GetIndex(i),inListValue);
        }
      }
    }
  }
}

template < typename TFeatureImage, typename TSeedImage, typename TTensorImage>
double ConnectednessFilter <TFeatureImage, TSeedImage, TTensorImage>::GetDistanceValue(IndexType idxStart, IndexType idxEnd)
{
  FeaturePixelType cost = 0;

  switch (m_Mode)
  {
  case ( FeatureSimilarity ) :
  {
    cost = (-1+exp(std::fabs(m_InputImage->GetPixel(idxStart) - m_InputImage->GetPixel(idxEnd))));
    break;
  }
  case ( VectorAgreement ) :
  {
    // Rational
    // Evaluate Confidence of Tensors (startIdx and endIdx), the higher the lower the cost
    // Evaluate Path direction correspondence with tensor at start and end index
    vnl_vector_fixed<double,3> projection;
    vnl_vector_fixed<double,3> projectionEnd;
    projection.fill(0);
    projectionEnd.fill(0);

    itk::Point<double,3> worldPosStart;
    itk::Point<double,3> worldPosEnd;
    IndexTensorType tensorIdx;
    IndexType featureIdxStart;
    IndexTensorType tensorIdxEnd;
    IndexType featureIdxEnd;
    m_InputImage->TransformIndexToPhysicalPoint(idxStart, worldPosStart);
    m_InputImage->TransformIndexToPhysicalPoint(idxEnd, worldPosEnd);
    m_TensorImage->TransformPhysicalPointToIndex(worldPosStart, tensorIdx);
    m_TensorImage->TransformPhysicalPointToIndex(worldPosEnd, tensorIdxEnd);

    m_ConfidenceImage->TransformPhysicalPointToIndex(worldPosStart, featureIdxStart);
    m_ConfidenceImage->TransformPhysicalPointToIndex(worldPosEnd, featureIdxEnd);

    if (!m_TensorImage->GetLargestPossibleRegion().IsInside(tensorIdxEnd) || !m_TensorImage->GetLargestPossibleRegion().IsInside(tensorIdx))
      return -1;

    if (m_ConfidenceImage->GetPixel(featureIdxStart) < .1)
      return -1;

    vnl_vector_fixed<double,3> direction;
    direction[0] = (-worldPosStart[0] + worldPosEnd[0]);
    direction[1] = (-worldPosStart[1] + worldPosEnd[1]);
    direction[2] = (-worldPosStart[2] + worldPosEnd[2]);

    direction = direction.normalize();

    TensorPixelType tensorEnd = m_TensorImage->GetPixel(tensorIdxEnd);
    TensorPixelType tensorStart = m_TensorImage->GetPixel(tensorIdx);

    typename TensorPixelType::EigenValuesArrayType eigenvalues;

    tensorEnd.ComputeEigenValues(eigenvalues);
    double maxEVEnd  = std::fabs(eigenvalues[2]);

    tensorStart.ComputeEigenValues(eigenvalues);
    double maxEVStart  = std::fabs(eigenvalues[2]);

    if (maxEVEnd == 0.0)
      maxEVEnd =1; // no change then ..
    // Normalize by largest EV
    tensorEnd[0] /=  maxEVEnd;
    tensorEnd[1] /=  maxEVEnd;
    tensorEnd[2] /=  maxEVEnd;
    tensorEnd[3] /=  maxEVEnd;
    tensorEnd[4] /=  maxEVEnd;
    tensorEnd[5] /=  maxEVEnd;


    if (maxEVStart == 0.0)
      maxEVStart =1; // no change then ..
    // Normalize by largest EV
    tensorStart[0] /=  maxEVStart;
    tensorStart[1] /=  maxEVStart;
    tensorStart[2] /=  maxEVStart;
    tensorStart[3] /=  maxEVStart;
    tensorStart[4] /=  maxEVStart;
    tensorStart[5] /=  maxEVStart;


    /* Matrix Style
  *       | 0  1  2  |
  *       | X  3  4  |
  *       | X  X  5  |
  */
    // Multiply vector with tensor, and then take the magnitude of it.
    projection[0] = direction[0]*tensorStart[0]+direction[1]*tensorStart[1]+direction[2]*tensorStart[2];
    projection[1] = direction[0]*tensorStart[1]+direction[1]*tensorStart[3]+direction[2]*tensorStart[4];
    projection[2] = direction[0]*tensorStart[2]+direction[1]*tensorStart[4]+direction[2]*tensorStart[5];

    projectionEnd[0] = direction[0]*tensorEnd[0]+direction[1]*tensorEnd[1]+direction[2]*tensorEnd[2];
    projectionEnd[1] = direction[0]*tensorEnd[1]+direction[1]*tensorEnd[3]+direction[2]*tensorEnd[4];
    projectionEnd[2] = direction[0]*tensorEnd[2]+direction[1]*tensorEnd[4]+direction[2]*tensorEnd[5];

    if (direction.magnitude() == 0.0 || projectionEnd.magnitude() == 0.0)
      return -1;

    // Confidences higher than 1 are considered to be noise (due to the behavior of FA values derived from tensors with negative eigenvalues)
    FeaturePixelType confStart = m_ConfidenceImage->GetPixel(featureIdxStart);
    if (confStart > 1)
      confStart =  .1;
    FeaturePixelType confEnd = m_ConfidenceImage->GetPixel(featureIdxEnd);
    if (confEnd > 1)
      confEnd =  .1;

    // costs
    cost = (1.1-confStart)*(1.0/sqrt(projection.magnitude()));
    cost *=(1.1-confEnd)*(1.0/sqrt(projectionEnd.magnitude()));

    break;
  }
  case ( FeatureVectorAgreement ) :
  {
    // Rational
    // Evaluate Confidence of Tensors (startIdx and endIdx), the higher the lower the cost
    // Evaluate Path direction correspondence with tensor at start and end index
    vnl_vector_fixed<double,3> projection;
    vnl_vector_fixed<double,3> projectionEnd;
    projection.fill(0);
    projectionEnd.fill(0);

    itk::Point<double,3> worldPosStart;
    itk::Point<double,3> worldPosEnd;
    IndexTensorType tensorIdx;
    IndexType featureIdxStart;
    IndexTensorType tensorIdxEnd;
    IndexType featureIdxEnd;
    m_InputImage->TransformIndexToPhysicalPoint(idxStart, worldPosStart);
    m_InputImage->TransformIndexToPhysicalPoint(idxEnd, worldPosEnd);
    m_TensorImage->TransformPhysicalPointToIndex(worldPosStart, tensorIdx);
    m_TensorImage->TransformPhysicalPointToIndex(worldPosEnd, tensorIdxEnd);

    m_ConfidenceImage->TransformPhysicalPointToIndex(worldPosStart, featureIdxStart);
    m_ConfidenceImage->TransformPhysicalPointToIndex(worldPosEnd, featureIdxEnd);

    if (!m_TensorImage->GetLargestPossibleRegion().IsInside(tensorIdxEnd) || !m_TensorImage->GetLargestPossibleRegion().IsInside(tensorIdx))
      return -1;

    if (m_ConfidenceImage->GetPixel(featureIdxStart) < .1)
      return -1;

    vnl_vector_fixed<double,3> direction;
    direction[0] = (-worldPosStart[0] + worldPosEnd[0]);
    direction[1] = (-worldPosStart[1] + worldPosEnd[1]);
    direction[2] = (-worldPosStart[2] + worldPosEnd[2]);

    direction = direction.normalize();

    TensorPixelType tensorEnd = m_TensorImage->GetPixel(tensorIdxEnd);
    TensorPixelType tensorStart = m_TensorImage->GetPixel(tensorIdx);

    typename TensorPixelType::EigenValuesArrayType eigenvalues;

    tensorEnd.ComputeEigenValues(eigenvalues);
    double maxEVEnd  = std::fabs(eigenvalues[2]);

    tensorStart.ComputeEigenValues(eigenvalues);
    double maxEVStart  = std::fabs(eigenvalues[2]);

    if (maxEVEnd == 0.0)
      maxEVEnd =1; // no change then ..
    // Normalize by largest EV
    tensorEnd[0] /=  maxEVEnd;
    tensorEnd[1] /=  maxEVEnd;
    tensorEnd[2] /=  maxEVEnd;
    tensorEnd[3] /=  maxEVEnd;
    tensorEnd[4] /=  maxEVEnd;
    tensorEnd[5] /=  maxEVEnd;


    if (maxEVStart == 0.0)
      maxEVStart =1; // no change then ..
    // Normalize by largest EV
    tensorStart[0] /=  maxEVStart;
    tensorStart[1] /=  maxEVStart;
    tensorStart[2] /=  maxEVStart;
    tensorStart[3] /=  maxEVStart;
    tensorStart[4] /=  maxEVStart;
    tensorStart[5] /=  maxEVStart;


    /* Matrix Style
  *       | 0  1  2  |
  *       | X  3  4  |
  *       | X  X  5  |
  */
    // Multiply vector with tensor, and then take the magnitude of it.
    projection[0] = direction[0]*tensorStart[0]+direction[1]*tensorStart[1]+direction[2]*tensorStart[2];
    projection[1] = direction[0]*tensorStart[1]+direction[1]*tensorStart[3]+direction[2]*tensorStart[4];
    projection[2] = direction[0]*tensorStart[2]+direction[1]*tensorStart[4]+direction[2]*tensorStart[5];

    projectionEnd[0] = direction[0]*tensorEnd[0]+direction[1]*tensorEnd[1]+direction[2]*tensorEnd[2];
    projectionEnd[1] = direction[0]*tensorEnd[1]+direction[1]*tensorEnd[3]+direction[2]*tensorEnd[4];
    projectionEnd[2] = direction[0]*tensorEnd[2]+direction[1]*tensorEnd[4]+direction[2]*tensorEnd[5];

    if (direction.magnitude() == 0.0 || projectionEnd.magnitude() == 0.0)
      return -1;

    // Confidences higher than 1 are considered to be noise (due to the behavior of FA values derived from tensors with negative eigenvalues)
    FeaturePixelType confStart = m_ConfidenceImage->GetPixel(featureIdxStart);
    if (confStart > 1)
      confStart =  .1;
    FeaturePixelType confEnd = m_ConfidenceImage->GetPixel(featureIdxEnd);
    if (confEnd > 1)
      confEnd =  .1;

    //costs
    cost = (1.1-confStart)*(1.0/sqrt(projection.magnitude()));
    cost *=(1.1-confEnd)*(1.0/sqrt(projectionEnd.magnitude()));
    cost *= (-1+exp(std::fabs(m_InputImage->GetPixel(idxStart) - m_InputImage->GetPixel(idxEnd))));

    break;
  }
  }
  return cost;
}

}// end itk namespace


#endif
