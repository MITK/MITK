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

#ifndef __itkMLBSTrackingFilter_txx
#define __itkMLBSTrackingFilter_txx

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "itkMLBSTrackingFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkImageFileWriter.h>
//#include <boost/thread/thread.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template< int NumImageFeatures >
MLBSTrackingFilter< NumImageFeatures >
::MLBSTrackingFilter()
    : m_FiberPolyData(NULL)
    , m_Points(NULL)
    , m_Cells(NULL)
    , m_AngularThreshold(0.7)
    , m_StepSize(0)
    , m_MaxLength(10000)
    , m_MinTractLength(20.0)
    , m_MaxTractLength(400.0)
    , m_SeedsPerVoxel(1)
    , m_UseDirection(true)
    , m_NumberOfSamples(50)
    , m_SamplingDistance(-1)
    , m_SeedImage(NULL)
    , m_MaskImage(NULL)
    , m_DecisionForest(NULL)
    , m_StoppingRegions(NULL)
    , m_DemoMode(false)
    , m_PauseTracking(false)
    , m_AbortTracking(false)
    , m_RemoveWmEndFibers(false)
    , m_AposterioriCurvCheck(false)
    , m_AvoidStop(true)
{
    this->SetNumberOfRequiredInputs(1);
}

template< int NumImageFeatures >
double MLBSTrackingFilter< NumImageFeatures >
::RoundToNearest(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::BeforeThreadedGenerateData()
{
    m_InputImage = const_cast<InputImageType*>(this->GetInput(0));
    PreprocessRawData();

    m_FiberPolyData = PolyDataType::New();
    m_Points = vtkSmartPointer< vtkPoints >::New();
    m_Cells = vtkSmartPointer< vtkCellArray >::New();

    m_ImageSize.resize(3);
    m_ImageSize[0] = m_FeatureImage->GetLargestPossibleRegion().GetSize()[0];
    m_ImageSize[1] = m_FeatureImage->GetLargestPossibleRegion().GetSize()[1];
    m_ImageSize[2] = m_FeatureImage->GetLargestPossibleRegion().GetSize()[2];

    m_ImageSpacing.resize(3);
    m_ImageSpacing[0] = m_FeatureImage->GetSpacing()[0];
    m_ImageSpacing[1] = m_FeatureImage->GetSpacing()[1];
    m_ImageSpacing[2] = m_FeatureImage->GetSpacing()[2];

    double minSpacing;
    if(m_ImageSpacing[0]<m_ImageSpacing[1] && m_ImageSpacing[0]<m_ImageSpacing[2])
        minSpacing = m_ImageSpacing[0];
    else if (m_ImageSpacing[1] < m_ImageSpacing[2])
        minSpacing = m_ImageSpacing[1];
    else
        minSpacing = m_ImageSpacing[2];

    m_StepSize *= minSpacing;
    if (m_StepSize<mitk::eps)
        m_StepSize = 0.5*minSpacing;

    m_SamplingDistance *= minSpacing;
    if (m_SamplingDistance<mitk::eps)
        m_SamplingDistance = minSpacing*0.5;

    m_PolyDataContainer.clear();
    for (unsigned int i=0; i<this->GetNumberOfThreads(); i++)
    {
        PolyDataType poly = PolyDataType::New();
        m_PolyDataContainer.push_back(poly);
    }

    m_NotWmImage = ItkDoubleImgType::New();
    m_NotWmImage->SetSpacing( m_FeatureImage->GetSpacing() );
    m_NotWmImage->SetOrigin( m_FeatureImage->GetOrigin() );
    m_NotWmImage->SetDirection( m_FeatureImage->GetDirection() );
    m_NotWmImage->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
    m_NotWmImage->Allocate();
    m_NotWmImage->FillBuffer(0);

    m_WmImage = ItkDoubleImgType::New();
    m_WmImage->SetSpacing( m_FeatureImage->GetSpacing() );
    m_WmImage->SetOrigin( m_FeatureImage->GetOrigin() );
    m_WmImage->SetDirection( m_FeatureImage->GetDirection() );
    m_WmImage->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
    m_WmImage->Allocate();
    m_WmImage->FillBuffer(0);

    m_AvoidStopImage = ItkDoubleImgType::New();
    m_AvoidStopImage->SetSpacing( m_FeatureImage->GetSpacing() );
    m_AvoidStopImage->SetOrigin( m_FeatureImage->GetOrigin() );
    m_AvoidStopImage->SetDirection( m_FeatureImage->GetDirection() );
    m_AvoidStopImage->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
    m_AvoidStopImage->Allocate();
    m_AvoidStopImage->FillBuffer(0);

    if (m_StoppingRegions.IsNull())
    {
        m_StoppingRegions = ItkUcharImgType::New();
        m_StoppingRegions->SetSpacing( m_FeatureImage->GetSpacing() );
        m_StoppingRegions->SetOrigin( m_FeatureImage->GetOrigin() );
        m_StoppingRegions->SetDirection( m_FeatureImage->GetDirection() );
        m_StoppingRegions->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
        m_StoppingRegions->Allocate();
        m_StoppingRegions->FillBuffer(0);
    }

    if (m_SeedImage.IsNull())
    {
        m_SeedImage = ItkUcharImgType::New();
        m_SeedImage->SetSpacing( m_FeatureImage->GetSpacing() );
        m_SeedImage->SetOrigin( m_FeatureImage->GetOrigin() );
        m_SeedImage->SetDirection( m_FeatureImage->GetDirection() );
        m_SeedImage->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
        m_SeedImage->Allocate();
        m_SeedImage->FillBuffer(1);
    }

    if (m_MaskImage.IsNull())
    {
        // initialize mask image
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( m_FeatureImage->GetSpacing() );
        m_MaskImage->SetOrigin( m_FeatureImage->GetOrigin() );
        m_MaskImage->SetDirection( m_FeatureImage->GetDirection() );
        m_MaskImage->SetRegions( m_FeatureImage->GetLargestPossibleRegion() );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    else
        std::cout << "MLBSTrackingFilter: using mask image" << std::endl;

    if (m_AngularThreshold<0.0)
        m_AngularThreshold = 0.5*minSpacing;

    m_BuildFibersReady = 0;
    m_BuildFibersFinished = false;
    m_Threads = 0;
    m_Tractogram.clear();

    std::cout << "MLBSTrackingFilter: Angular threshold: " << m_AngularThreshold << std::endl;
    std::cout << "MLBSTrackingFilter: Stepsize: " << m_StepSize << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Seeds per voxel: " << m_SeedsPerVoxel << std::endl;
    std::cout << "MLBSTrackingFilter: Max. sampling distance: " << m_SamplingDistance << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Number of samples: " << m_NumberOfSamples << std::endl;
    std::cout << "MLBSTrackingFilter: Max. tract length: " << m_MaxTractLength << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Min. tract length: " << m_MinTractLength << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Starting streamline tracking using " << this->GetNumberOfThreads() << " threads." << std::endl;
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::PreprocessRawData()
{
    typedef itk::AnalyticalDiffusionQballReconstructionImageFilter<short,short,float,6, 2*NumImageFeatures> InterpolationFilterType;

    std::cout << "MLBSTrackingFilter: Spherical signal interpolation and sampling ..." << std::endl;
    typename InterpolationFilterType::Pointer filter = InterpolationFilterType::New();
    filter->SetGradientImage( m_GradientDirections, m_InputImage );
    filter->SetBValue( m_B_Value );
    filter->SetLambda(0.006);
    filter->SetNormalizationMethod(InterpolationFilterType::QBAR_RAW_SIGNAL);
    filter->Update();
    //      FeatureImageType::Pointer itkFeatureImage = qballfilter->GetCoefficientImage();
    //      featureImageVector.push_back(itkFeatureImage);

    std::cout << "MLBSTrackingFilter: Creating feature image ..." << std::endl;
    vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
    itk::OrientationDistributionFunction< double, NumImageFeatures*2 > odf;
    m_DirectionIndices.clear();
    for (unsigned int f=0; f<NumImageFeatures*2; f++)
    {
        if (dot_product(ref, odf.GetDirection(f))>0)    // only used directions on one hemisphere
            m_DirectionIndices.push_back(f);
    }

    m_FeatureImage = FeatureImageType::New();
    m_FeatureImage->SetSpacing(filter->GetOutput()->GetSpacing());
    m_FeatureImage->SetOrigin(filter->GetOutput()->GetOrigin());
    m_FeatureImage->SetDirection(filter->GetOutput()->GetDirection());
    m_FeatureImage->SetLargestPossibleRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->SetBufferedRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->SetRequestedRegion(filter->GetOutput()->GetLargestPossibleRegion());
    m_FeatureImage->Allocate();

    itk::ImageRegionIterator< typename InterpolationFilterType::OutputImageType > it(filter->GetOutput(), filter->GetOutput()->GetLargestPossibleRegion());
    while(!it.IsAtEnd())
    {
        typename FeatureImageType::PixelType pix;
        for (unsigned int f=0; f<NumImageFeatures; f++)
            pix[f] = it.Get()[m_DirectionIndices.at(f)];
        m_FeatureImage->SetPixel(it.GetIndex(), pix);
        ++it;
    }
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::CalculateNewPosition(itk::Point<double, 3>& pos, vnl_vector_fixed<double, 3>& dir)
{
    //    vnl_matrix_fixed< double, 3, 3 > rot = m_FeatureImage->GetDirection().GetTranspose();
    //    dir = rot*dir;

    dir *= m_StepSize;
    pos[0] += dir[0];
    pos[1] += dir[1];
    pos[2] += dir[2];
}

template< int NumImageFeatures >
bool MLBSTrackingFilter< NumImageFeatures >
::IsValidPosition(itk::Point<double, 3> &pos)
{
    typename FeatureImageType::IndexType idx;
    m_FeatureImage->TransformPhysicalPointToIndex(pos, idx);
    if (!m_FeatureImage->GetLargestPossibleRegion().IsInside(idx) || m_MaskImage->GetPixel(idx)==0)
        return false;

    return true;
}

template< int NumImageFeatures >
typename MLBSTrackingFilter< NumImageFeatures >::FeatureImageType::PixelType MLBSTrackingFilter< NumImageFeatures >::GetImageValues(itk::Point<float, 3> itkP)
{
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
}

template< int NumImageFeatures >
vnl_vector_fixed<double,3> MLBSTrackingFilter< NumImageFeatures >::Classify(itk::Point<double, 3>& pos, int& candidates, vnl_vector_fixed<double,3>& olddir, double angularThreshold, double& prob, bool avoidStop)
{
    vnl_vector_fixed<double,3> direction; direction.fill(0);

    vigra::MultiArray<2, double> featureData;
    if(m_UseDirection)
        featureData = vigra::MultiArray<2, double>( vigra::Shape2(1,NumImageFeatures+3) );
    else
        featureData = vigra::MultiArray<2, double>( vigra::Shape2(1,NumImageFeatures) );

    typename FeatureImageType::PixelType featurePixel = GetImageValues(pos);

    // pixel values
    for (unsigned int f=0; f<NumImageFeatures; f++)
        featureData(0,f) = featurePixel[f];

    // direction features
    int c = 0;
    if (m_UseDirection)
    {
        vnl_vector_fixed<double,3> ref; ref.fill(0); ref[0]=1;
        for (unsigned int f=NumImageFeatures; f<NumImageFeatures+3; f++)
        {
            if (dot_product(ref, olddir)<0)
                featureData(0,f) = -olddir[c];
            else
                featureData(0,f) = olddir[c];
            c++;
        }
    }

    vigra::MultiArray<2, double> probs(vigra::Shape2(1, m_DecisionForest->class_count()));
    m_DecisionForest->predictProbabilities(featureData, probs);

    double outProb = 0;
    prob = 0;
    candidates = 0; // directions with probability > 0
    for (int i=0; i<m_DecisionForest->class_count(); i++)
    {
        if (probs(0,i)>0)
        {
            int classLabel = 0;
            m_DecisionForest->ext_param_.to_classlabel(i, classLabel);

            if (classLabel<m_DirectionIndices.size())
            {
                candidates++;

                vnl_vector_fixed<double,3> d = m_ODF.GetDirection(m_DirectionIndices.at(classLabel));
                double dot = dot_product(d, olddir);

                if (olddir.magnitude()>0)
                {
                    if (fabs(dot)>angularThreshold)
                    {
                        if (dot<0)
                            d *= -1;
                        dot = fabs(dot);
                        direction += probs(0,i)*dot*d;
                        prob += probs(0,i)*dot;
                    }
                }
                else
                {
                    direction += probs(0,i)*d;
                    prob += probs(0,i);
                }
            }
            else
                outProb += probs(0,i);
        }
    }

    ItkDoubleImgType::IndexType idx;
    m_NotWmImage->TransformPhysicalPointToIndex(pos, idx);
    if (m_NotWmImage->GetLargestPossibleRegion().IsInside(idx))
    {
        m_NotWmImage->SetPixel(idx, m_NotWmImage->GetPixel(idx)+outProb);
        m_WmImage->SetPixel(idx, m_WmImage->GetPixel(idx)+prob);
    }
    if (outProb>prob && prob>0)
    {
        candidates = 0;
        prob = 0;
        direction.fill(0.0);
    }
    if (avoidStop && m_AvoidStopImage->GetLargestPossibleRegion().IsInside(idx) && candidates>0 && direction.magnitude()>0.001)
        m_AvoidStopImage->SetPixel(idx, m_AvoidStopImage->GetPixel(idx)+0.1);

    return direction;
}


template< int NumImageFeatures >
double MLBSTrackingFilter< NumImageFeatures >::GetRandDouble(double min, double max)
{
    return (double)(rand()%((int)(10000*(max-min))) + 10000*min)/10000;
}

template< int NumImageFeatures >
vnl_vector_fixed<double,3> MLBSTrackingFilter< NumImageFeatures >::GetNewDirection(itk::Point<double, 3> &pos, vnl_vector_fixed<double, 3>& olddir)
{
    vnl_vector_fixed<double,3> direction; direction.fill(0);

    ItkUcharImgType::IndexType idx;
    m_StoppingRegions->TransformPhysicalPointToIndex(pos, idx);
    if (m_StoppingRegions->GetPixel(idx)>0)
        return direction;

    if (olddir.magnitude()>0)
        olddir.normalize();

    int candidates = 0; // number of directions with probability > 0
    double prob = 0;
    direction = Classify(pos, candidates, olddir, m_AngularThreshold, prob); // sample neighborhood
    direction *= prob;


    itk::OrientationDistributionFunction< double, 50 >  probeVecs;

    for (int i=0; i<m_NumberOfSamples; i++)
    {
        vnl_vector_fixed<double,3> d;
//        probe[0] = GetRandDouble()*m_SamplingDistance;
//        probe[1] = GetRandDouble()*m_SamplingDistance;
//        probe[2] = GetRandDouble()*m_SamplingDistance;

        d = probeVecs.GetDirection(i)*m_SamplingDistance;

        itk::Point<double, 3> sample_pos;
        sample_pos[0] = pos[0] + d[0];
        sample_pos[1] = pos[1] + d[1];
        sample_pos[2] = pos[2] + d[2];

        candidates = 0;
        vnl_vector_fixed<double,3> tempDir = Classify(sample_pos, candidates, olddir, m_AngularThreshold, prob); // sample neighborhood
        if (candidates>0 && tempDir.magnitude()>0.001)
        {
            direction += tempDir*prob;
        }
        else if (m_AvoidStop && candidates==0 && olddir.magnitude()>0) // out of white matter
        {
            double dot = dot_product(d, olddir);
            if (dot >= 0.0) // in front of plane defined by pos and olddir
                d = -d + 2*dot*olddir; // reflect
            else
                d = -d; // invert

            // look a bit further into the other direction
            sample_pos[0] = pos[0] + d[0]*2;
            sample_pos[1] = pos[1] + d[1]*2;
            sample_pos[2] = pos[2] + d[2]*2;
            candidates = 0;
            vnl_vector_fixed<double,3> tempDir = Classify(sample_pos, candidates, olddir, m_AngularThreshold, prob, true); // sample neighborhood

            if (candidates>0 && tempDir.magnitude()>0.001)  // are we back in the white matter?
            {
                direction += d;         // go into the direction of the white matter
                direction += tempDir*prob;  // go into the direction of the white matter direction at this location
            }
        }
    }

    if (direction.magnitude()>0.001)
    {
        direction.normalize();
        olddir[0] = direction[0];
        olddir[1] = direction[1];
        olddir[2] = direction[2];
    }
    else
        direction.fill(0);

    return direction;
}

template< int NumImageFeatures >
double MLBSTrackingFilter< NumImageFeatures >::FollowStreamline(ThreadIdType threadId, itk::Point<double, 3> pos, vnl_vector_fixed<double,3> dir, FiberType* fib, double tractLength, bool front)
{
    vnl_vector_fixed<double,3> dirOld = dir;
    dirOld = dir;

    for (int step=0; step< m_MaxLength/2; step++)
    {
        while (m_PauseTracking){}
        if (m_DemoMode)
        {
            m_Mutex.Lock();
            m_BuildFibersReady++;
            m_Tractogram.push_back(*fib);
            BuildFibers(true);
            m_Stop = true;
            m_Mutex.Unlock();
            while (m_Stop){}
        }

        // get new position
        CalculateNewPosition(pos, dir);

        // is new position inside of image and mask
        if (!IsValidPosition(pos) || m_AbortTracking)   // if not end streamline
        {
            return tractLength;
        }
        else    // if yes, add new point to streamline
        {
            tractLength +=  m_StepSize;
            if (front)
                fib->push_front(pos);
            else
                fib->push_back(pos);

            if (m_AposterioriCurvCheck)
            {
                int curv = CheckCurvature(fib, front);  // TODO: Move into classification ???
                if (curv>0)
                {
                    tractLength -= m_StepSize*curv;
                    while (curv>0)
                    {
                        if (front)
                            fib->pop_front();
                        else
                            fib->pop_back();
                        curv--;
                    }
                    return tractLength;
                }
            }

            if (tractLength>m_MaxTractLength)
                return tractLength;
        }

        dir = GetNewDirection(pos, dirOld);

        if (dir.magnitude()<0.0001)
            return tractLength;
    }
    return tractLength;
}

template< int NumImageFeatures >
int MLBSTrackingFilter<NumImageFeatures>::CheckCurvature(FiberType* fib, bool front)
{
    double m_Distance = 5;
    if (fib->size()<3)
        return 0;

    double dist = 0;
    std::vector< vnl_vector_fixed< float, 3 > > vectors;
    vnl_vector_fixed< float, 3 > meanV; meanV.fill(0);
    double dev = 0;

    if (front)
    {
        int c=0;
        while(dist<m_Distance && c<fib->size()-1)
        {
            itk::Point<double> p1 = fib->at(c);
            itk::Point<double> p2 = fib->at(c+1);

            vnl_vector_fixed< float, 3 > v;
            v[0] = p2[0]-p1[0];
            v[1] = p2[1]-p1[1];
            v[2] = p2[2]-p1[2];
            dist += v.magnitude();
            v.normalize();
            vectors.push_back(v);
            if (c==0)
                meanV += v;
            c++;
        }
    }
    else
    {
        int c=fib->size()-1;
        while(dist<m_Distance && c>0)
        {
            itk::Point<double> p1 = fib->at(c);
            itk::Point<double> p2 = fib->at(c-1);

            vnl_vector_fixed< float, 3 > v;
            v[0] = p2[0]-p1[0];
            v[1] = p2[1]-p1[1];
            v[2] = p2[2]-p1[2];
            dist += v.magnitude();
            v.normalize();
            vectors.push_back(v);
            if (c==fib->size()-1)
                meanV += v;
            c--;
        }
    }
    meanV.normalize();

    for (int c=0; c<vectors.size(); c++)
    {
        double angle = dot_product(meanV, vectors.at(c));
        if (angle>1.0)
            angle = 1.0;
        if (angle<-1.0)
            angle = -1.0;
        dev += acos(angle)*180/M_PI;
    }
    if (vectors.size()>0)
        dev /= vectors.size();

    if (dev<30)
        return 0;
    else
        return vectors.size();
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::ThreadedGenerateData(const InputImageRegionType &regionForThread, ThreadIdType threadId)
{
    m_Mutex.Lock();
    m_Threads++;
    m_Mutex.Unlock();
    typedef ImageRegionConstIterator< ItkUcharImgType >     MaskIteratorType;
    MaskIteratorType    sit(m_SeedImage, regionForThread );
    MaskIteratorType    mit(m_MaskImage, regionForThread );

    sit.GoToBegin();
    mit.GoToBegin();
    itk::Point<double> worldPos;
    while( !sit.IsAtEnd() )
    {
        if (sit.Value()==0 || mit.Value()==0)
        {
            ++sit;
            ++mit;
            continue;
        }

        for (int s=0; s<m_SeedsPerVoxel; s++)
        {
            FiberType fib;
            double tractLength = 0;
            typename FeatureImageType::IndexType index = sit.GetIndex();
            itk::ContinuousIndex<double, 3> start;
            unsigned int counter = 0;

            if (m_SeedsPerVoxel>1)
            {
                start[0] = index[0]+GetRandDouble(-0.5, 0.5);
                start[1] = index[1]+GetRandDouble(-0.5, 0.5);
                start[2] = index[2]+GetRandDouble(-0.5, 0.5);
            }
            else
            {
                start[0] = index[0];
                start[1] = index[1];
                start[2] = index[2];
            }

            // get staring position
            m_SeedImage->TransformContinuousIndexToPhysicalPoint( start, worldPos );

            // get starting direction
            int candidates = 0;
            double prob = 0;
            vnl_vector_fixed<double,3> dirOld; dirOld.fill(0.0);
            vnl_vector_fixed<double,3> dir = Classify(worldPos, candidates, dirOld, 0, prob);
            if (dir.magnitude()<0.0001)
                continue;

            // forward tracking
            tractLength = FollowStreamline(threadId, worldPos, dir, &fib, 0, false);
            fib.push_front(worldPos);

            if (m_RemoveWmEndFibers)
            {
                itk::Point<double> check = fib.back();
                dirOld.fill(0.0);
                vnl_vector_fixed<double,3> check2 = GetNewDirection(check, dirOld);
                if (check2.magnitude()>0.001)
                {
                    MITK_INFO << "Detected WM ending. Discarding fiber.";
                    continue;
                }
            }

            // backward tracking
            tractLength = FollowStreamline(threadId, worldPos, -dir, &fib, tractLength, true);
            counter = fib.size();

            if (m_RemoveWmEndFibers)
            {
                itk::Point<double> check = fib.front();
                dirOld.fill(0.0);
                vnl_vector_fixed<double,3> check2 = GetNewDirection(check, dirOld);
                if (check2.magnitude()>0.001)
                {
                    MITK_INFO << "Detected WM ending. Discarding fiber.";
                    continue;
                }
            }

            if (tractLength<m_MinTractLength || counter<2)
                continue;

            m_Mutex.Lock();
            m_Tractogram.push_back(fib);
            m_Mutex.Unlock();

            if (m_AbortTracking)
                break;
        }
        if (m_AbortTracking)
            break;
        ++sit;
        ++mit;
    }
    m_Threads--;
    std::cout << "Thread " << threadId << " finished tracking" << std::endl;
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::BuildFibers(bool check)
{
    if (m_BuildFibersReady<m_Threads && check)
        return;

    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

    for (int i=0; i<m_Tractogram.size(); i++)
    {
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        FiberType fib = m_Tractogram.at(i);
        for (FiberType::iterator it = fib.begin(); it!=fib.end(); it++)
        {
            vtkIdType id = vNewPoints->InsertNextPoint((*it).GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
        }
        vNewLines->InsertNextCell(container);
    }
    if (check)
        for (int i=0; i<m_BuildFibersReady; i++)
            m_Tractogram.pop_back();
    m_BuildFibersReady = 0;

    m_FiberPolyData->SetPoints(vNewPoints);
    m_FiberPolyData->SetLines(vNewLines);
    m_BuildFibersFinished = true;
}

template< int NumImageFeatures >
void MLBSTrackingFilter< NumImageFeatures >::AfterThreadedGenerateData()
{
    MITK_INFO << "Generating polydata ";
    BuildFibers(false);
    MITK_INFO << "done";
}

}

#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
