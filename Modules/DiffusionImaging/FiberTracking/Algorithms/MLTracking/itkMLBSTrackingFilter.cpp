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

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {

template<  int ShOrder, int NumImageFeatures >
MLBSTrackingFilter<  ShOrder, NumImageFeatures >
::MLBSTrackingFilter()
    : m_PauseTracking(false)
    , m_AbortTracking(false)
    , m_FiberPolyData(NULL)
    , m_Points(NULL)
    , m_Cells(NULL)
    , m_AngularThreshold(0.7)
    , m_StepSize(0)
    , m_MaxLength(10000)
    , m_MinTractLength(20.0)
    , m_MaxTractLength(400.0)
    , m_SeedsPerVoxel(1)
    , m_RandomSampling(false)
    , m_SamplingDistance(-1)
    , m_NumberOfSamples(50)
    , m_StoppingRegions(NULL)
    , m_SeedImage(NULL)
    , m_MaskImage(NULL)
    , m_AposterioriCurvCheck(false)
    , m_RemoveWmEndFibers(false)
    , m_AvoidStop(true)
    , m_DemoMode(false)
{
    this->SetNumberOfRequiredInputs(1);
}

template<  int ShOrder, int NumImageFeatures >
double MLBSTrackingFilter<  ShOrder, NumImageFeatures >
::RoundToNearest(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

template<  int ShOrder, int NumImageFeatures >
void MLBSTrackingFilter<  ShOrder, NumImageFeatures >::BeforeThreadedGenerateData()
{
    m_InputImage = const_cast<InputImageType*>(this->GetInput(0));
    m_ForestHandler.InitForTracking();

    m_FiberPolyData = PolyDataType::New();
    m_Points = vtkSmartPointer< vtkPoints >::New();
    m_Cells = vtkSmartPointer< vtkCellArray >::New();

    std::vector< double > m_ImageSpacing; m_ImageSpacing.resize(3);
    m_ImageSpacing[0] = m_InputImage->GetSpacing()[0];
    m_ImageSpacing[1] = m_InputImage->GetSpacing()[1];
    m_ImageSpacing[2] = m_InputImage->GetSpacing()[2];

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

    if (m_StoppingRegions.IsNull())
    {
        m_StoppingRegions = ItkUcharImgType::New();
        m_StoppingRegions->SetSpacing( m_InputImage->GetSpacing() );
        m_StoppingRegions->SetOrigin( m_InputImage->GetOrigin() );
        m_StoppingRegions->SetDirection( m_InputImage->GetDirection() );
        m_StoppingRegions->SetRegions( m_InputImage->GetLargestPossibleRegion() );
        m_StoppingRegions->Allocate();
        m_StoppingRegions->FillBuffer(0);
    }

    if (m_SeedImage.IsNull())
    {
        m_SeedImage = ItkUcharImgType::New();
        m_SeedImage->SetSpacing( m_InputImage->GetSpacing() );
        m_SeedImage->SetOrigin( m_InputImage->GetOrigin() );
        m_SeedImage->SetDirection( m_InputImage->GetDirection() );
        m_SeedImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
        m_SeedImage->Allocate();
        m_SeedImage->FillBuffer(1);
    }

    if (m_MaskImage.IsNull())
    {
        // initialize mask image
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( m_InputImage->GetSpacing() );
        m_MaskImage->SetOrigin( m_InputImage->GetOrigin() );
        m_MaskImage->SetDirection( m_InputImage->GetDirection() );
        m_MaskImage->SetRegions( m_InputImage->GetLargestPossibleRegion() );
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
    m_SamplingPointset = mitk::PointSet::New();
    m_AlternativePointset = mitk::PointSet::New();
    m_StartTime = std::chrono::system_clock::now();

    std::cout << "MLBSTrackingFilter: Angular threshold: " << m_AngularThreshold << std::endl;
    std::cout << "MLBSTrackingFilter: Stepsize: " << m_StepSize << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Seeds per voxel: " << m_SeedsPerVoxel << std::endl;
    std::cout << "MLBSTrackingFilter: Max. sampling distance: " << m_SamplingDistance << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Number of samples: " << m_NumberOfSamples << std::endl;
    std::cout << "MLBSTrackingFilter: Max. tract length: " << m_MaxTractLength << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Min. tract length: " << m_MinTractLength << " mm" << std::endl;
    std::cout << "MLBSTrackingFilter: Starting streamline tracking using " << this->GetNumberOfThreads() << " threads." << std::endl;
}

template<  int ShOrder, int NumImageFeatures >
void MLBSTrackingFilter<  ShOrder, NumImageFeatures >::CalculateNewPosition(itk::Point<double, 3>& pos, vnl_vector_fixed<double, 3>& dir)
{
    //    vnl_matrix_fixed< double, 3, 3 > rot = m_FeatureImage->GetDirection().GetTranspose();
    //    dir = rot*dir;

    dir *= m_StepSize;
    pos[0] += dir[0];
    pos[1] += dir[1];
    pos[2] += dir[2];
}

template<  int ShOrder, int NumImageFeatures >
bool MLBSTrackingFilter<  ShOrder, NumImageFeatures >
::IsValidPosition(itk::Point<double, 3> &pos)
{
    typename FeatureImageType::IndexType idx;
    m_InputImage->TransformPhysicalPointToIndex(pos, idx);
    if (!m_InputImage->GetLargestPossibleRegion().IsInside(idx) || m_MaskImage->GetPixel(idx)==0)
        return false;

    return true;
}

template<  int ShOrder, int NumImageFeatures >
double MLBSTrackingFilter<  ShOrder, NumImageFeatures >::GetRandDouble(double min, double max)
{
    return (double)(rand()%((int)(10000*(max-min))) + 10000*min)/10000;
}

template<  int ShOrder, int NumImageFeatures >
vnl_vector_fixed<double,3> MLBSTrackingFilter<  ShOrder, NumImageFeatures >::GetNewDirection(itk::Point<double, 3> &pos, vnl_vector_fixed<double, 3>& olddir)
{
    if (m_DemoMode)
    {
        m_SamplingPointset->Clear();
        m_AlternativePointset->Clear();
    }
    vnl_vector_fixed<double,3> direction; direction.fill(0);

    ItkUcharImgType::IndexType idx;
    m_StoppingRegions->TransformPhysicalPointToIndex(pos, idx);
    if (m_StoppingRegions->GetLargestPossibleRegion().IsInside(idx) && m_StoppingRegions->GetPixel(idx)>0)
        return direction;

    if (m_MaskImage.IsNotNull() && ((m_MaskImage->GetLargestPossibleRegion().IsInside(idx) && m_MaskImage->GetPixel(idx)<=0) || !m_MaskImage->GetLargestPossibleRegion().IsInside(idx)) )
        return direction;

    if (olddir.magnitude()>0)
        olddir.normalize();

    int candidates = 0; // number of directions with probability > 0
    double w = 0;       // weight of the direction predicted at each sampling point
    if (IsValidPosition(pos))
    {
        direction = m_ForestHandler.Classify(pos, candidates, olddir, m_AngularThreshold, w, m_MaskImage); // get direction proposal at current streamline position
        direction *= w;  // HERE WE ARE WEIGHTING AGAIN EVEN THOUGH THE OUTPUT DIRECTIONS ARE ALREADY WEIGHTED!!! THE EFFECT OF THIS HAS YET TO BE EVALUATED.
    }

    itk::OrientationDistributionFunction< double, 50 >  probeVecs;
    itk::Point<double, 3> sample_pos;
    int alternatives = 1;
    for (int i=0; i<m_NumberOfSamples; i++)
    {
        vnl_vector_fixed<double,3> d;

        if (m_RandomSampling)
        {
            d[0] = GetRandDouble();
            d[1] = GetRandDouble();
            d[2] = GetRandDouble();
            d.normalize();
            d *= GetRandDouble(0,m_SamplingDistance);
        }
        else
        {
            d = probeVecs.GetDirection(i)*m_SamplingDistance;
        }

        sample_pos[0] = pos[0] + d[0];
        sample_pos[1] = pos[1] + d[1];
        sample_pos[2] = pos[2] + d[2];
        if(m_DemoMode)
            m_SamplingPointset->InsertPoint(i, sample_pos);

        candidates = 0;
        vnl_vector_fixed<double,3> tempDir; tempDir.fill(0.0);
        if (IsValidPosition(sample_pos))
            tempDir = m_ForestHandler.Classify(sample_pos, candidates, olddir, m_AngularThreshold, w, m_MaskImage); // sample neighborhood
        if (candidates>0 && tempDir.magnitude()>0.001)
        {
            direction += tempDir*w;
        }
        else if (m_AvoidStop && candidates==0 && olddir.magnitude()>0) // out of white matter
        {
            double dot = dot_product(d, olddir);
            if (dot >= 0.0) // in front of plane defined by pos and olddir
                d = -d + 2*dot*olddir; // reflect
            else
                d = -d; // invert

            // look a bit further into the other direction
            sample_pos[0] = pos[0] + d[0];
            sample_pos[1] = pos[1] + d[1];
            sample_pos[2] = pos[2] + d[2];
            if(m_DemoMode)
                m_AlternativePointset->InsertPoint(alternatives, sample_pos);
            alternatives++;
            candidates = 0;
            vnl_vector_fixed<double,3> tempDir; tempDir.fill(0.0);
            if (IsValidPosition(sample_pos))
                tempDir = m_ForestHandler.Classify(sample_pos, candidates, olddir, m_AngularThreshold, w, m_MaskImage); // sample neighborhood

            if (candidates>0 && tempDir.magnitude()>0.001)  // are we back in the white matter?
            {
                direction += d;         // go into the direction of the white matter
                direction += tempDir*w;  // go into the direction of the white matter direction at this location
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

template<  int ShOrder, int NumImageFeatures >
double MLBSTrackingFilter<  ShOrder, NumImageFeatures >::FollowStreamline(itk::Point<double, 3> pos, vnl_vector_fixed<double,3> dir, FiberType* fib, double tractLength, bool front)
{
    vnl_vector_fixed<double,3> dirOld = dir;
    dirOld = dir;

    for (int step=0; step< m_MaxLength/2; step++)
    {

        // get new position
        CalculateNewPosition(pos, dir);

        // is new position inside of image and mask
        if (m_AbortTracking)   // if not end streamline
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
        if (m_DemoMode) // CHECK: warum sind die samplingpunkte der streamline in der visualisierung immer einen schritt voras?
        {
            m_Mutex.Lock();
            m_BuildFibersReady++;
            m_Tractogram.push_back(*fib);
            BuildFibers(true);
            m_Stop = true;
            m_Mutex.Unlock();

            while (m_Stop){
            }
        }
        dir = GetNewDirection(pos, dirOld);

        while (m_PauseTracking){}

        if (dir.magnitude()<0.0001)
            return tractLength;
    }
    return tractLength;
}

template<  int ShOrder, int NumImageFeatures >
int MLBSTrackingFilter<  ShOrder, NumImageFeatures >::CheckCurvature(FiberType* fib, bool front)
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

template<  int ShOrder, int NumImageFeatures >
void MLBSTrackingFilter<  ShOrder, NumImageFeatures >::ThreadedGenerateData(const InputImageRegionType &regionForThread, ThreadIdType threadId)
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
            vnl_vector_fixed<double,3> dir; dir.fill(0.0);
            if (IsValidPosition(worldPos))
                dir = m_ForestHandler.Classify(worldPos, candidates, dirOld, 0, prob, m_MaskImage);
            if (dir.magnitude()<0.0001)
                continue;

            // forward tracking
            tractLength = FollowStreamline(worldPos, dir, &fib, 0, false);
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
            tractLength = FollowStreamline(worldPos, -dir, &fib, tractLength, true);
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

template<  int ShOrder, int NumImageFeatures >
void MLBSTrackingFilter<  ShOrder, NumImageFeatures >::BuildFibers(bool check)
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
        for (FiberType::iterator it = fib.begin(); it!=fib.end(); ++it)
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

template<  int ShOrder, int NumImageFeatures >
void MLBSTrackingFilter<  ShOrder, NumImageFeatures >::AfterThreadedGenerateData()
{
    MITK_INFO << "Generating polydata ";
    BuildFibers(false);
    MITK_INFO << "done";

    m_EndTime = std::chrono::system_clock::now();
    std::chrono::hours   hh = std::chrono::duration_cast<std::chrono::hours>(m_EndTime - m_StartTime);
    std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(m_EndTime - m_StartTime);
    std::chrono::seconds ss = std::chrono::duration_cast<std::chrono::seconds>(m_EndTime - m_StartTime);
    mm %= 60;
    ss %= 60;
    MITK_INFO << "Tracking took " << hh.count() << "h, " << mm.count() << "m and " << ss.count() << "s";
}

}

#endif // __itkDiffusionQballPrincipleDirectionsImageFilter_txx
