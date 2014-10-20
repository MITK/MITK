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

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "mitkImageCast.h"

#include "mitkDiffusionImage.h"

#include "itkImageDuplicator.h"
#include "itkTestingComparisonImageFilter.h"


template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::DiffusionImage()
  : m_VectorImage(0), m_B_Value(-1.0), m_OriginalDirections(0), m_Directions(0)
{
  MeasurementFrameType mf;
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
      mf[i][j] = 0;
  for(int i=0; i<3; i++)
    mf[i][i] = 1;
  m_MeasurementFrame = mf;
}

template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::DiffusionImage(const DiffusionImage<TPixelType> & orig)
  : mitk::Image(orig),
    m_VectorImage( 0 ),
    m_B_Value(0),
    m_OriginalDirections(0),
    m_Directions(0)
{
  // Deep copy VectorImage
  typename itk::ImageDuplicator<ImageType>::Pointer duplicator = itk::ImageDuplicator<ImageType>::New();
  duplicator->SetInputImage( orig.m_VectorImage );
  duplicator->Update();

  GradientDirectionContainerType::ConstIterator origIt;
  GradientDirectionContainerType::ConstIterator origItEnd;

  // Deep Copy OrignalDirectioncontainer
  GradientDirectionContainerType::Pointer OriginalDirectionscontainer = GradientDirectionContainerType::New();
  origIt = orig.GetDirectionsWithoutMeasurementFrame()->Begin();
  origItEnd = orig.GetDirectionsWithoutMeasurementFrame()->End();

  for(;origIt != origItEnd; ++origIt)
    OriginalDirectionscontainer->push_back(origIt.Value());

  // Deep Copy Directioncontainer
  GradientDirectionContainerType::Pointer DirectionsContainer = GradientDirectionContainerType::New();
  origIt = orig.GetDirections()->Begin();
  origItEnd = orig.GetDirections()->End();

  for(;origIt != origItEnd; ++origIt)
    DirectionsContainer->push_back(origIt.Value());

  // Set member of the new clone
  m_VectorImage = duplicator->GetOutput();
  m_B_Value = orig.GetReferenceBValue();
  m_OriginalDirections = OriginalDirectionscontainer;
  m_Directions = DirectionsContainer;
  m_MeasurementFrame = orig.GetMeasurementFrame();
  ApplyMeasurementFrame();
  UpdateBValueMap();
  InitializeFromVectorImage();

  //  m_VectorImage = duplicator->GetOutput();
  //  m_B_Value = orig.GetB_Value();
  //  m_MeasurementFrame = orig.GetMeasurementFrame();
  //  m_Directions = copyInContainer;
  //  m_OriginalDirections = copyInContainer;


}


template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::~DiffusionImage()
{
  // Remove Observer for m_Directions
  //RemoveObserver();
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>
::InitializeFromVectorImage()
{
  if(!m_VectorImage || !m_Directions || m_B_Value==-1.0)
  {
    MITK_INFO << "DiffusionImage could not be initialized. Set all members first!" << std::endl;
    // Fehjlermedlung updaten
    return;
  }

  // find bzero index
  int firstZeroIndex = -1;
  for(GradientDirectionContainerType::ConstIterator it = m_Directions->Begin();
      it != m_Directions->End(); ++it)
  {
    firstZeroIndex++;
    GradientDirectionType g = it.Value();
    if(g[0] == 0 && g[1] == 0 && g[2] == 0 )
      break;
  }

  typedef itk::Image<TPixelType,3> ImgType;
  typename ImgType::Pointer img = ImgType::New();
  img->SetSpacing( m_VectorImage->GetSpacing() );   // Set the image spacing
  img->SetOrigin( m_VectorImage->GetOrigin() );     // Set the image origin
  img->SetDirection( m_VectorImage->GetDirection() );  // Set the image direction
  img->SetLargestPossibleRegion( m_VectorImage->GetLargestPossibleRegion());
  img->SetBufferedRegion( m_VectorImage->GetLargestPossibleRegion() );
  img->Allocate();

  int vecLength = m_VectorImage->GetVectorLength();
  InitializeByItk( img.GetPointer(), 1, vecLength );

  itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
  itw.GoToBegin();

  itk::ImageRegionConstIterator<ImageType> itr (m_VectorImage, m_VectorImage->GetLargestPossibleRegion() );
  itr.GoToBegin();

  while(!itr.IsAtEnd())
  {
    itw.Set(itr.Get().GetElement(firstZeroIndex));
    ++itr;
    ++itw;
  }

  // init
  SetImportVolume(img->GetBufferPointer());

  m_DisplayIndex = firstZeroIndex;
  MITK_INFO << "Diffusion-Image successfully initialized.";
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>
::SetDisplayIndexForRendering(int displayIndex)
{
  int index = displayIndex;
  int vecLength = m_VectorImage->GetVectorLength();
  index = index > vecLength-1 ? vecLength-1 : index;
  if( m_DisplayIndex != index )
  {
    typedef itk::Image<TPixelType,3> ImgType;
    typename ImgType::Pointer img = ImgType::New();
    CastToItkImage(this, img);

    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw.GoToBegin();

    itk::ImageRegionConstIterator<ImageType> itr (m_VectorImage, m_VectorImage->GetLargestPossibleRegion() );
    itr.GoToBegin();

    while(!itr.IsAtEnd())
    {
      itw.Set(itr.Get().GetElement(index));
      ++itr;
      ++itw;
    }
  }

  m_DisplayIndex = index;
}

template<typename TPixelType>
bool mitk::DiffusionImage<TPixelType>::AreAlike(GradientDirectionType g1,
                                                GradientDirectionType g2,
                                                double precision)
{
  GradientDirectionType diff = g1 - g2;
  GradientDirectionType diff2 = g1 + g2;
  return diff.two_norm() < precision || diff2.two_norm() < precision;
}


template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Pointer
mitk::DiffusionImage<TPixelType>::CalcAveragedDirectionSet(double precision, GradientDirectionContainerType::Pointer directions)
{
  // save old and construct new direction container
  GradientDirectionContainerType::Pointer newDirections = GradientDirectionContainerType::New();

  // fill new direction container
  for(GradientDirectionContainerType::ConstIterator gdcitOld = directions->Begin();
      gdcitOld != directions->End(); ++gdcitOld)
  {
    // already exists?
    bool found = false;
    for(GradientDirectionContainerType::ConstIterator gdcitNew = newDirections->Begin();
        gdcitNew != newDirections->End(); ++gdcitNew)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        found = true;
        break;
      }
    }

    // if not found, add it to new container
    if(!found)
    {
      newDirections->push_back(gdcitOld.Value());
    }
  }

  return newDirections;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::AverageRedundantGradients(double precision)
{

  GradientDirectionContainerType::Pointer newDirs =
      CalcAveragedDirectionSet(precision, m_Directions);

  GradientDirectionContainerType::Pointer newOriginalDirs =
      CalcAveragedDirectionSet(precision, m_OriginalDirections);

  // if sizes equal, we do not need to do anything in this function
  if(m_Directions->size() == newDirs->size() || m_OriginalDirections->size() == newOriginalDirs->size())
    return;

  GradientDirectionContainerType::Pointer oldDirections = m_OriginalDirections;
  m_Directions = newDirs;
  m_OriginalDirections = newOriginalDirs;

  // new image
  typename ImageType::Pointer oldImage = m_VectorImage;
  m_VectorImage = ImageType::New();
  m_VectorImage->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
  m_VectorImage->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
  m_VectorImage->SetDirection( oldImage->GetDirection() );  // Set the image direction
  m_VectorImage->SetLargestPossibleRegion( oldImage->GetLargestPossibleRegion() );
  m_VectorImage->SetVectorLength( m_Directions->size() );
  m_VectorImage->SetBufferedRegion( oldImage->GetLargestPossibleRegion() );
  m_VectorImage->Allocate();

  // average image data that corresponds to identical directions
  itk::ImageRegionIterator< ImageType > newIt(m_VectorImage, m_VectorImage->GetLargestPossibleRegion());
  newIt.GoToBegin();
  itk::ImageRegionIterator< ImageType > oldIt(oldImage, oldImage->GetLargestPossibleRegion());
  oldIt.GoToBegin();

  // initial new value of voxel
  typename ImageType::PixelType newVec;
  newVec.SetSize(m_Directions->size());
  newVec.AllocateElements(m_Directions->size());

  std::vector<std::vector<int> > dirIndices;
  for(GradientDirectionContainerType::ConstIterator gdcitNew = m_Directions->Begin();
      gdcitNew != m_Directions->End(); ++gdcitNew)
  {
    dirIndices.push_back(std::vector<int>(0));
    for(GradientDirectionContainerType::ConstIterator gdcitOld = oldDirections->Begin();
        gdcitOld != oldDirections->End(); ++gdcitOld)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        //MITK_INFO << gdcitNew.Value() << "  " << gdcitOld.Value();
        dirIndices[gdcitNew.Index()].push_back(gdcitOld.Index());
      }
    }
  }

  //int ind1 = -1;
  while(!newIt.IsAtEnd())
  {

    // progress
    //typename ImageType::IndexType ind = newIt.GetIndex();
    //ind1 = ind.m_Index[2];

    // init new vector with zeros
    newVec.Fill(0.0);

    // the old voxel value with duplicates
    typename ImageType::PixelType oldVec = oldIt.Get();

    for(unsigned int i=0; i<dirIndices.size(); i++)
    {
      // do the averaging
      const unsigned int numavg = dirIndices[i].size();
      unsigned int sum = 0;
      for(unsigned int j=0; j<numavg; j++)
      {
        //MITK_INFO << newVec[i] << " << " << oldVec[dirIndices[i].at(j)];
        sum += oldVec[dirIndices[i].at(j)];
      }
      if(numavg == 0)
      {
        MITK_ERROR << "mitkDiffusionImage: Error on averaging. Possibly due to corrupted data";
        return;
      }
      newVec[i] = sum / numavg;
    }

    newIt.Set(newVec);

    ++newIt;
    ++oldIt;
  }
  ApplyMeasurementFrame();
  UpdateBValueMap();
  std::cout << std::endl;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::ApplyMeasurementFrame()
{
  if(m_OriginalDirections.IsNull())
  {
    // original direction container was not set
    return;
  }

  m_Directions = GradientDirectionContainerType::New();
  int c = 0;
  for(GradientDirectionContainerType::ConstIterator gdcit = m_OriginalDirections->Begin();
      gdcit != m_OriginalDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    vec = vec.pre_multiply(m_MeasurementFrame);
    m_Directions->InsertElement(c, vec);
    c++;
  }
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::UpdateBValueMap()
{
  if(!m_B_ValueMap.empty())
    m_B_ValueMap.clear();

  GradientDirectionContainerType::ConstIterator gdcit;
  for( gdcit = this->m_Directions->Begin(); gdcit != this->m_Directions->End(); ++gdcit)
    m_B_ValueMap[this->GetB_Value(gdcit.Index())].push_back(gdcit.Index());
}

template<typename TPixelType>
float mitk::DiffusionImage<TPixelType>::GetB_Value(unsigned int i)
{
  if(i > m_Directions->Size()-1)
    return -1;

  if(m_Directions->ElementAt(i).one_norm() <= 0.0)
  {
    return 0;
  }
  else
  {
    double twonorm = m_Directions->ElementAt(i).two_norm();
    double bval = m_B_Value*twonorm*twonorm;

    if (bval<0)
      bval = ceil(bval - 0.5);
    else
      bval = floor(bval + 0.5);

    return bval;
  }
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::SetDirections( GradientDirectionContainerType::Pointer directions )
{
  this->m_OriginalDirections = directions;
  ApplyMeasurementFrame();
  UpdateBValueMap();
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::SetDirections(const std::vector<itk::Vector<double, 3> > &directions)
{
  GradientDirectionContainerType::Pointer tempContainer = GradientDirectionContainerType::New();
  for(unsigned int i=0; i<directions.size(); i++)
    tempContainer->InsertElement( i, directions[i].GetVnlVector() );
  SetDirections(tempContainer);
}

template<typename TPixelType>
typename mitk::DiffusionImage<TPixelType>::MeasurementFrameType mitk::DiffusionImage<TPixelType>::GetMeasurementFrame() const
{
  return m_MeasurementFrame;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::SetMeasurementFrame( const MeasurementFrameType & mFrame )
{
  m_MeasurementFrame = mFrame;
  ApplyMeasurementFrame();
}

template<typename TPixelType>
bool mitk::DiffusionImage<TPixelType>::GetNumberOfBValues() const
{
  return m_B_ValueMap.size();
}

template<typename TPixelType>
const typename mitk::DiffusionImage<TPixelType>::BValueMap & mitk::DiffusionImage<TPixelType>::GetBValueMap() const
{
  return m_B_ValueMap;
}

template<typename TPixelType>
float mitk::DiffusionImage<TPixelType>::GetReferenceBValue() const
{
  return m_B_Value;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::SetReferenceBValue( float val )
{
  m_B_Value = val;
}

template<typename TPixelType>
typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerTypePointer mitk::DiffusionImage<TPixelType>::GetDirections() const
{
  return m_Directions;
}

template<typename TPixelType>
typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerTypePointer mitk::DiffusionImage<TPixelType>::GetDirectionsWithoutMeasurementFrame() const
{
  return m_OriginalDirections;
}

template<typename TPixelType>
typename itk::VectorImage<TPixelType, 3>::Pointer mitk::DiffusionImage<TPixelType>::GetVectorImage()
{
  return m_VectorImage;
}

template<typename TPixelType>
const typename itk::VectorImage<TPixelType, 3>::Pointer mitk::DiffusionImage<TPixelType>::GetVectorImage() const
{
  return m_VectorImage;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::SetVectorImage(typename ImageType::Pointer image )
{
  m_VectorImage = image;
}

template<class TPixelType>
inline bool mitk::Equal(const mitk::DiffusionImage<TPixelType>& leftHandSide, const mitk::DiffusionImage<TPixelType> &rightHandSide, ScalarType eps, bool verbose )
{

  bool returnValue = true;

  if(leftHandSide.GetReferenceBValue() != rightHandSide.GetReferenceBValue())
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] Reference BValue is not Equal.";
    returnValue = false;
  }

  if(leftHandSide.GetMeasurementFrame() != rightHandSide.GetMeasurementFrame())
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] MeasurementFrame is not Equal.";
    returnValue = false;
  }

  if(leftHandSide.GetBValueMap().size() != rightHandSide.GetBValueMap().size())
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] BValue Map: Size is not Equal.";
    returnValue = false;
  }

  if(leftHandSide.GetNumberOfBValues() != rightHandSide.GetNumberOfBValues())
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] BValue Map (GetNumberOfBValues): Size is not Equal.";
    returnValue = false;
  }

  // Slow testing area

  const mitk::Image* img1 = dynamic_cast<const mitk::Image*>(&leftHandSide);
  const mitk::Image* img2 = dynamic_cast<const mitk::Image*>(&rightHandSide);

  if(mitk::Equal(*img1,*img2,eps,verbose) == false)
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] Base-Class (mitk::Image) is not Equal.";
    returnValue = false;
  }

  {
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator lhsIt = leftHandSide.GetDirectionsWithoutMeasurementFrame()->Begin();
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator lhsItEnd = leftHandSide.GetDirectionsWithoutMeasurementFrame()->End();
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator rhsIt = rightHandSide.GetDirectionsWithoutMeasurementFrame()->Begin();

    for(;lhsIt != lhsItEnd;)
    {
      bool vectorNotEqual = false;
      for(unsigned int i = 0 ; i < lhsIt.Value().size(); i++)
          vectorNotEqual |= !mitk::Equal(lhsIt.Value().get(i),rhsIt.Value().get(i),0.0001);

      if(vectorNotEqual)
      {
        if(verbose)
          MITK_INFO << "[( DiffusionImage )] Original GradientDirections are not Equal.";
        returnValue = false;
        break;
      }
      ++rhsIt;
      ++lhsIt;
    }
  }

  {
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator lhsIt = leftHandSide.GetDirections()->Begin();
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator lhsItEnd = leftHandSide.GetDirections()->End();
    typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Iterator rhsIt = rightHandSide.GetDirections()->Begin();

    for(;lhsIt != lhsItEnd;)
    {
      bool vectorNotEqual = false;
      for(unsigned int i = 0 ; i < lhsIt.Value().size(); i++)
          vectorNotEqual |= !mitk::Equal(lhsIt.Value().get(i),rhsIt.Value().get(i),0.0001);

      if(vectorNotEqual)
      {
        if(verbose)
          MITK_INFO << "[( DiffusionImage )] GradientDirections are not Equal.";
        returnValue = false;
        break;
      }
      ++rhsIt;
      ++lhsIt;
    }
  }

  {

    typename mitk::DiffusionImage<TPixelType>::BValueMap rhsMap = rightHandSide.GetBValueMap();
    typename mitk::DiffusionImage<TPixelType>::BValueMap lhsMap = leftHandSide.GetBValueMap();
    typename mitk::DiffusionImage<TPixelType>::BValueMap::const_iterator lhsIt = lhsMap.begin();
    typename mitk::DiffusionImage<TPixelType>::BValueMap::const_iterator lhsItEnd = lhsMap.end();
    typename mitk::DiffusionImage<TPixelType>::BValueMap::const_iterator rhsIt = rhsMap.begin();

    for(;lhsIt != lhsItEnd;)
    {
      if(lhsIt->first != rhsIt->first)
      {
        if(verbose)
          MITK_INFO << "[( DiffusionImage )] BValue Map: lhsKey " <<  lhsIt->first << " != rhsKey " << rhsIt->first << " is not Equal.";
        returnValue = false;
        break;
      }

      if(lhsIt->second.size() != rhsIt->second.size())
      {
        if(verbose)
          MITK_INFO << "[( DiffusionImage )] BValue Map: Indices vector size is not Equal. (Key: " << lhsIt->first <<")";
        returnValue = false;
        break;
      }


      typename mitk::DiffusionImage<TPixelType>::IndicesVector::const_iterator lhsIndVecIt = lhsIt->second.begin();
      typename mitk::DiffusionImage<TPixelType>::IndicesVector::const_iterator lhsIndVecItEnd = lhsIt->second.end();
      typename mitk::DiffusionImage<TPixelType>::IndicesVector::const_iterator rhsIndVecIt = rhsIt->second.begin();

      for(;lhsIndVecIt != lhsIndVecItEnd;)
      {
        if(*lhsIndVecIt != *rhsIndVecIt)
        {
          if(verbose)
            MITK_INFO << "[( DiffusionImage )] BValue Map: Indices are not Equal. (Key: " << lhsIt->first <<")";
          returnValue = false;
          break;
        }
        ++rhsIndVecIt;
        ++lhsIndVecIt;
      }

      lhsIt++;
      rhsIt++;
    }
  }


  try{
    itk::ImageRegionIterator< itk::VectorImage< TPixelType, 3 > > it1(leftHandSide.GetVectorImage(), leftHandSide.GetVectorImage()->GetLargestPossibleRegion());
    itk::ImageRegionIterator< itk::VectorImage< TPixelType, 3 > > it2(rightHandSide.GetVectorImage(), rightHandSide.GetVectorImage()->GetLargestPossibleRegion());
    while(!it1.IsAtEnd())
    {
      if (it1.Get()!=it2.Get()){
        if(verbose)
          MITK_INFO << "[( DiffusionImage )] Capsulated itk::VectorImage is not Equal.";
        returnValue = false;
        break;
      }
      ++it1;
      ++it2;
    }
  }
  catch(...)
  {
    if(verbose)
      MITK_INFO << "[( DiffusionImage )] Comparision of itk Vector image abort by exception.";
    returnValue = false;
  }

  return returnValue;
}

