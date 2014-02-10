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

#include "itkTestingComparisonImageFilter.h"


template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::DiffusionImage()
    : m_VectorImage(0), m_Directions(0), m_OriginalDirections(0), m_B_Value(-1.0), m_VectorImageAdaptor(0)
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
mitk::DiffusionImage<TPixelType>::~DiffusionImage()
{
    // Remove Observer for m_Directions
    RemoveDirectionsContainerObserver();
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>
::InitializeFromVectorImage()
{
    if(!m_VectorImage || !m_Directions || m_B_Value==-1.0)
    {
        MITK_INFO << "DiffusionImage could not be initialized. Set all members first!" << std::endl;
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
        CastToItkImage<ImgType>(this, img);

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
void mitk::DiffusionImage<TPixelType>::CorrectDKFZBrokenGradientScheme(double precision)
{
    GradientDirectionContainerType::Pointer directionSet = CalcAveragedDirectionSet(precision, m_Directions);
    if(directionSet->size() < 7)
    {
        MITK_INFO << "Too few directions, assuming and correcting DKFZ-bogus sequence details.";

        double v [7][3] =
        {{ 0,         0,         0        },
         {-0.707057,  0,         0.707057 },
         { 0.707057,  0,         0.707057 },
         { 0,         0.707057,  0.707057 },
         { 0,         0.707057, -0.707057 },
         {-0.707057,  0.707057,  0        },
         { 0.707057,  0.707057,  0        } };

        int i=0;

        for(GradientDirectionContainerType::Iterator it = m_OriginalDirections->Begin();
            it != m_OriginalDirections->End(); ++it)
        {
            it.Value().set(v[i++%7]);
        }
        ApplyMeasurementFrame();
    }
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
    std::cout << std::endl;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::ApplyMeasurementFrame()
{
    RemoveDirectionsContainerObserver();

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

    UpdateBValueMap();
    AddDirectionsContainerObserver();
}

// returns number of gradients
template<typename TPixelType>
int mitk::DiffusionImage<TPixelType>::GetNumDirections()
{
    int gradients = m_OriginalDirections->Size();
    for (int i=0; i<m_OriginalDirections->Size(); i++)
        if (GetB_Value(i)<=0)
        {
            gradients--;
        }
    return gradients;
}

// returns number of not diffusion weighted images
template<typename TPixelType>
int mitk::DiffusionImage<TPixelType>::GetNumB0()
{
    int b0 = 0;
    for (int i=0; i<m_OriginalDirections->Size(); i++)
        if (GetB_Value(i)<=0)
        {
            b0++;
        }
    return b0;
}

// returns a list of indices belonging to the not diffusion weighted images
template<typename TPixelType>
typename mitk::DiffusionImage<TPixelType>::IndicesVector mitk::DiffusionImage<TPixelType>::GetB0Indices()
{
    IndicesVector indices;
    for (unsigned int i=0; i<m_OriginalDirections->Size(); i++)
        if (GetB_Value(i)<=0)
        {
            indices.push_back(i);
        }
    return indices;
}

template<typename TPixelType>
bool mitk::DiffusionImage<TPixelType>::IsMultiBval()
{
    int gradients = m_OriginalDirections->Size();

    for (int i=0; i<gradients; i++)
        if (GetB_Value(i)>0 && std::fabs(m_B_Value-GetB_Value(i))>50)
            return true;
    return false;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::UpdateBValueMap()
{
    m_B_ValueMap.clear();

    GradientDirectionContainerType::ConstIterator gdcit;
    for( gdcit = this->m_Directions->Begin(); gdcit != this->m_Directions->End(); ++gdcit)
        m_B_ValueMap[GetB_Value(gdcit.Index())].push_back(gdcit.Index());
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
void mitk::DiffusionImage<TPixelType>::SetDirections(const std::vector<itk::Vector<double,3> > directions)
{
    m_OriginalDirections = GradientDirectionContainerType::New();
    for(unsigned int i=0; i<directions.size(); i++)
    {
        m_OriginalDirections->InsertElement( i, directions[i].GetVnlVector() );
    }
    this->ApplyMeasurementFrame();
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::AddDirectionsContainerObserver()
{
    // Add Modified Observer to invoke an UpdateBValueList by modifieng the DirectionsContainer (m_Directions)
    typedef DiffusionImage< TPixelType > Self;
    typedef itk::SimpleMemberCommand< Self >  DCCommand ;
    typename DCCommand::Pointer command = DCCommand::New();
    command->SetCallbackFunction(this, &Self::UpdateBValueMap);
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::RemoveDirectionsContainerObserver()
{
    if(m_Directions){
        m_Directions->RemoveAllObservers();
    }
}

template<typename TPixelType>
bool mitk::Equal(const mitk::DiffusionImage<TPixelType>* rightHandSide, const mitk::DiffusionImage<TPixelType>* leftHandSide, ScalarType eps, bool verbose)
{

    MITK_INFO << "[( DiffusionImage )] EQUAL METHOD.";
    bool returnValue = true;
    if( rightHandSide == NULL )
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] rightHandSide is NULL.";
        return false;
    }

    if( leftHandSide == NULL )
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] leftHandSide is NULL.";
        return false;
    }

    if(mitk::Equal(static_cast<mitk::Image*>(rightHandSide),static_cast<mitk::Image*>(leftHandSide),eps,verbose) == false)
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] Base-Class (mitk::Image) is not Equal.";
        return false;
    }

    if(leftHandSide->GetB_Value() != rightHandSide->GetB_Value())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] Reference BValue is not Equal.";
        return false;
    }

    typename itk::Testing::ComparisonImageFilter<TPixelType,TPixelType>::Pointer EqualFilter = itk::Testing::ComparisonImageFilter<TPixelType, TPixelType>::New();
    EqualFilter->SetInput(rightHandSide->GetVectorImage());
    EqualFilter->SetInput(leftHandSide->GetVectorImage());
    EqualFilter->Update();

    if(EqualFilter->GetNumberOfPixelsWithDifferences() != 0)
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] Capsulated itk::VectorImage is not Equal.";
        return false;
    }

    if(leftHandSide->GetMeasurementFrame() != rightHandSide->GetMeasurementFrame())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] MeasurementFrame is not Equal.";
        return false;
    }


    itk::VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > > * rhsCont = rightHandSide->GetDirections().GetPointer();
    itk::VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > > * lhsCont = leftHandSide->GetDirections().GetPointer();

    if(lhsCont->Size() != rhsCont->Size())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] GradientDirectionContainer size is not Equal.";
        return false;
    }

    if(!std::equal(lhsCont->Begin(),lhsCont->End(),rhsCont->Begin()))
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] GradientDirections are not Equal.";
        return false;
    }

    if(leftHandSide->IsMultiBval() != rightHandSide->IsMultiBval())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] IsMultiBval properie is not Equal.";
        return false;
    }



    if(leftHandSide->GetNumB0() != rightHandSide->GetNumB0())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] Number of B0Values is not Equal.";
        return false;
    }

    std::vector<unsigned int> rhsB0Indices = rightHandSide->GetB0Indices();
    std::vector<unsigned int> lhsB0Indices = leftHandSide->GetB0Indices();

    if(!std::equal(lhsB0Indices.begin(), lhsB0Indices.end(), rhsB0Indices.begin()))
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] Number of B0Values is not Equal.";
        return false;
    }


    std::map<unsigned int, std::vector<unsigned int> > rhsBValueMap = rightHandSide->GetB_ValueMap();
    std::map<unsigned int, std::vector<unsigned int> > lhsBValueMap = leftHandSide->GetB_ValueMap();

    if(lhsBValueMap.size() != rhsBValueMap.size())
    {
        if(verbose)
            MITK_INFO << "[( DiffusionImage )] BValue Map: Size is not Equal.";
        return false;
    }

    std::map<unsigned int, std::vector<unsigned int> >::iterator rhsBValueMapIterator = rhsBValueMap.begin();
    std::map<unsigned int, std::vector<unsigned int> >::iterator lhsBValueMapIterator = lhsBValueMap.begin();
    for(;lhsBValueMapIterator != lhsBValueMap.end();)
    {
        if(lhsBValueMapIterator->first != rhsBValueMapIterator->first)
        {
            if(verbose)
                MITK_INFO << "[( DiffusionImage )] BValue Map: lhsKey " <<  lhsBValueMapIterator->first << " != rhsKey " << rhsBValueMapIterator->first << " is not Equal.";
            return false;
        }

        if(lhsBValueMapIterator->second.size() != rhsBValueMapIterator->second.size())
        {
            if(verbose)
                MITK_INFO << "[( DiffusionImage )] BValue Map: Indices vector size is not Equal. (Key: " << lhsBValueMapIterator->first <<")";
            return false;
        }

        if(!std::equal(lhsBValueMapIterator->second.begin(),lhsBValueMapIterator->second.end(), rhsBValueMapIterator->second.begin()))
        {
            if(verbose)
                MITK_INFO << "[( DiffusionImage )] BValue Map: Indices are not Equal. (Key: " << lhsBValueMapIterator->first <<")";
            return false;
        }

        rhsBValueMapIterator++;
        lhsBValueMapIterator++;
    }

    return returnValue;
}

