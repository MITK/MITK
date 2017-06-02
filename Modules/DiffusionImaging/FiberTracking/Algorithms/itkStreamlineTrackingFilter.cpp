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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <omp.h>
#include "itkStreamlineTrackingFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <itkImageFileWriter.h>
#include "itkPointShell.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace itk {


StreamlineTrackingFilter
::StreamlineTrackingFilter()
    : m_PauseTracking(false)
    , m_AbortTracking(false)
    , m_FiberPolyData(nullptr)
    , m_Points(nullptr)
    , m_Cells(nullptr)
    , m_AngularThreshold(-1)
    , m_StepSize(0)
    , m_MaxLength(10000)
    , m_MinTractLength(20.0)
    , m_MaxTractLength(400.0)
    , m_SeedsPerVoxel(1)
    , m_RandomSampling(false)
    , m_SamplingDistance(-1)
    , m_DeflectionMod(1.0)
    , m_OnlyForwardSamples(true)
    , m_UseStopVotes(true)
    , m_NumberOfSamples(30)
    , m_NumPreviousDirections(1)
    , m_StoppingRegions(nullptr)
    , m_SeedImage(nullptr)
    , m_MaskImage(nullptr)
    , m_AposterioriCurvCheck(false)
    , m_AvoidStop(true)
    , m_DemoMode(false)
    , m_SeedOnlyGm(false)
    , m_ControlGmEndings(false)
    , m_WmLabel(3) // mrtrix 5ttseg labels
    , m_GmLabel(1) // mrtrix 5ttseg labels
    , m_StepSizeVox(-1)
    , m_SamplingDistanceVox(-1)
    , m_AngularThresholdDeg(-1)
    , m_MaxNumTracts(-1)
    , m_Random(true)
    , m_Verbose(true)
{
    this->SetNumberOfRequiredInputs(0);
}


void StreamlineTrackingFilter::BeforeTracking()
{
    std::cout.setf(std::ios::boolalpha);

    m_TrackingHandler->InitForTracking();

    m_FiberPolyData = PolyDataType::New();
    m_Points = vtkSmartPointer< vtkPoints >::New();
    m_Cells = vtkSmartPointer< vtkCellArray >::New();

    itk::Vector< double, 3 > imageSpacing = m_TrackingHandler->GetSpacing();

    float minSpacing;
    if(imageSpacing[0]<imageSpacing[1] && imageSpacing[0]<imageSpacing[2])
        minSpacing = imageSpacing[0];
    else if (imageSpacing[1] < imageSpacing[2])
        minSpacing = imageSpacing[1];
    else
        minSpacing = imageSpacing[2];

    if (m_StepSizeVox<mitk::eps)
        m_StepSize = 0.5*minSpacing;
    else
        m_StepSize = m_StepSizeVox*minSpacing;

    if (m_AngularThresholdDeg<0)
    {
        if  (m_StepSize/minSpacing<=1.0)
            m_AngularThreshold = std::cos( 0.5 * M_PI * m_StepSize/minSpacing );
        else
            m_AngularThreshold = std::cos( 0.5 * M_PI );
    }
    else
        m_AngularThreshold = std::cos( m_AngularThresholdDeg*M_PI/180.0 );
    m_TrackingHandler->SetAngularThreshold(m_AngularThreshold);

    if (m_SamplingDistanceVox<mitk::eps)
        m_SamplingDistance = minSpacing*0.25;
    else
        m_SamplingDistance = m_SamplingDistanceVox * minSpacing;

    m_PolyDataContainer.clear();
    for (unsigned int i=0; i<this->GetNumberOfThreads(); i++)
    {
        PolyDataType poly = PolyDataType::New();
        m_PolyDataContainer.push_back(poly);
    }

    if (m_StoppingRegions.IsNull())
    {
        m_StoppingRegions = ItkUcharImgType::New();
        m_StoppingRegions->SetSpacing( imageSpacing );
        m_StoppingRegions->SetOrigin( m_TrackingHandler->GetOrigin() );
        m_StoppingRegions->SetDirection( m_TrackingHandler->GetDirection() );
        m_StoppingRegions->SetRegions( m_TrackingHandler->GetLargestPossibleRegion() );
        m_StoppingRegions->Allocate();
        m_StoppingRegions->FillBuffer(0);
    }
    else
        std::cout << "StreamlineTracking - Using stopping region image" << std::endl;

    if (m_SeedImage.IsNull())
    {
        m_SeedImage = ItkUcharImgType::New();
        m_SeedImage->SetSpacing( imageSpacing );
        m_SeedImage->SetOrigin( m_TrackingHandler->GetOrigin() );
        m_SeedImage->SetDirection( m_TrackingHandler->GetDirection() );
        m_SeedImage->SetRegions( m_TrackingHandler->GetLargestPossibleRegion() );
        m_SeedImage->Allocate();
        m_SeedImage->FillBuffer(1);
    }
    else
        std::cout << "StreamlineTracking - Using seed image" << std::endl;

    if (m_MaskImage.IsNull())
    {
        // initialize mask image
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( imageSpacing );
        m_MaskImage->SetOrigin( m_TrackingHandler->GetOrigin() );
        m_MaskImage->SetDirection( m_TrackingHandler->GetDirection() );
        m_MaskImage->SetRegions( m_TrackingHandler->GetLargestPossibleRegion() );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    else
        std::cout << "StreamlineTracking - Using mask image" << std::endl;

    if (m_TissueImage.IsNull())
    {
        if (m_SeedOnlyGm)
        {
            MITK_WARN << "StreamlineTracking - Cannot seed in gray matter. No tissue type image set.";
            m_SeedOnlyGm = false;
        }

        if (m_ControlGmEndings)
        {
            MITK_WARN << "StreamlineTracking - Cannot control gray matter endings. No tissue type image set.";
            m_ControlGmEndings = false;
        }
    }
    else
    {
        if (m_ControlGmEndings)
            m_SeedOnlyGm = true;
        if (m_ControlGmEndings || m_SeedOnlyGm)
            std::cout << "StreamlineTracking - Using tissue image" << std::endl;
        else
            MITK_WARN << "StreamlineTracking - Tissue image set but no gray matter seeding or fiber endpoint-control enabled" << std::endl;
    }

    m_BuildFibersReady = 0;
    m_BuildFibersFinished = false;
    m_Tractogram.clear();
    m_SamplingPointset = mitk::PointSet::New();
    m_AlternativePointset = mitk::PointSet::New();
    m_StopVotePointset = mitk::PointSet::New();
    m_StartTime = std::chrono::system_clock::now();

    if (m_SeedOnlyGm && m_ControlGmEndings)
        InitGrayMatterEndings();

    if (m_DemoMode)
        omp_set_num_threads(1);

    if (m_TrackingHandler->GetMode()==mitk::TrackingDataHandler::MODE::DETERMINISTIC)
        std::cout << "StreamlineTracking - Mode: deterministic" << std::endl;
    else if(m_TrackingHandler->GetMode()==mitk::TrackingDataHandler::MODE::PROBABILISTIC)
        std::cout << "StreamlineTracking - Mode: probabilistic" << std::endl;
    else
        std::cout << "StreamlineTracking - Mode: ???" << std::endl;

    std::cout << "StreamlineTracking - Angular threshold: " << m_AngularThreshold << " (" << 180*std::acos( m_AngularThreshold )/M_PI << "°)" << std::endl;
    std::cout << "StreamlineTracking - Stepsize: " << m_StepSize << "mm (" << m_StepSize/minSpacing << "*vox)" << std::endl;
    std::cout << "StreamlineTracking - Seeds per voxel: " << m_SeedsPerVoxel << std::endl;
    std::cout << "StreamlineTracking - Max. tract length: " << m_MaxTractLength << "mm" << std::endl;
    std::cout << "StreamlineTracking - Min. tract length: " << m_MinTractLength << "mm" << std::endl;
    std::cout << "StreamlineTracking - Max. num. tracts: " << m_MaxNumTracts << std::endl;

    std::cout << "StreamlineTracking - Num. neighborhood samples: " << m_NumberOfSamples << std::endl;
    std::cout << "StreamlineTracking - Max. sampling distance: " << m_SamplingDistance << "mm (" << m_SamplingDistance/minSpacing << "*vox)" << std::endl;
    std::cout << "StreamlineTracking - Deflection modifier: " << m_DeflectionMod << std::endl;

    std::cout << "StreamlineTracking - Use stop votes: " << m_UseStopVotes << std::endl;
    std::cout << "StreamlineTracking - Only frontal samples: " << m_OnlyForwardSamples << std::endl;

    if (m_DemoMode)
    {
        std::cout << "StreamlineTracking - Running in demo mode";
        std::cout << "StreamlineTracking - Starting streamline tracking using 1 thread" << std::endl;
    }
    else
        std::cout << "StreamlineTracking - Starting streamline tracking using " << omp_get_max_threads() << " threads" << std::endl;
}


void StreamlineTrackingFilter::InitGrayMatterEndings()
{
    m_TrackingHandler->SetAngularThreshold(0);
    m_GmStubs.clear();
    if (m_TissueImage.IsNotNull())
    {
        std::cout << "StreamlineTracking - initializing GM endings" << std::endl;
        ImageRegionConstIterator< ItkUcharImgType > it(m_TissueImage, m_TissueImage->GetLargestPossibleRegion() );
        it.GoToBegin();

        vnl_vector_fixed<float,3> d1; d1.fill(0.0);
        std::deque< vnl_vector_fixed<float,3> > olddirs;
        while (olddirs.size()<m_NumPreviousDirections)
            olddirs.push_back(d1);

        while( !it.IsAtEnd() )
        {
            if (it.Value()==m_GmLabel)
            {
                ItkUcharImgType::IndexType s_idx = it.GetIndex();
                itk::ContinuousIndex<float, 3> start;
                m_TissueImage->TransformIndexToPhysicalPoint(s_idx, start);
                itk::Point<float, 3> wm_p;
                float max = -1;
                FiberType fib;

                for (int x : {-1,0,1})
                    for (int y : {-1,0,1})
                        for (int z : {-1,0,1})
                        {
                            if (x==y && y==z)
                                continue;

                            ItkUcharImgType::IndexType e_idx;
                            e_idx[0] = s_idx[0] + x;
                            e_idx[1] = s_idx[1] + y;
                            e_idx[2] = s_idx[2] + z;

                            if ( !m_TissueImage->GetLargestPossibleRegion().IsInside(e_idx) || m_TissueImage->GetPixel(e_idx)!=m_WmLabel )
                                continue;

                            itk::ContinuousIndex<float, 3> end;
                            m_TissueImage->TransformIndexToPhysicalPoint(e_idx, end);

                            d1 = m_TrackingHandler->ProposeDirection(end, olddirs, s_idx);
                            if (d1.magnitude()<0.0001)
                                continue;
                            d1.normalize();

                            vnl_vector_fixed< float, 3 > d2;
                            d2[0] = end[0] - start[0];
                            d2[1] = end[1] - start[1];
                            d2[2] = end[2] - start[2];
                            d2.normalize();
                            float a = fabs(dot_product(d1,d2));
                            if (a>max)
                            {
                                max = a;
                                wm_p = end;
                            }
                        }

                if (max>=0)
                {
                    fib.push_back(start);
                    fib.push_back(wm_p);
                    m_GmStubs.push_back(fib);
                }
            }
            ++it;
        }
    }
    m_TrackingHandler->SetAngularThreshold(m_AngularThreshold);
}


void StreamlineTrackingFilter::CalculateNewPosition(itk::Point<float, 3>& pos, vnl_vector_fixed<float, 3>& dir)
{
    pos[0] += dir[0]*m_StepSize;
    pos[1] += dir[1]*m_StepSize;
    pos[2] += dir[2]*m_StepSize;
}


bool StreamlineTrackingFilter
::IsValidPosition(itk::Point<float, 3> &pos)
{
    ItkUcharImgType::IndexType idx;
    m_MaskImage->TransformPhysicalPointToIndex(pos, idx);
    if (!m_MaskImage->GetLargestPossibleRegion().IsInside(idx) || m_MaskImage->GetPixel(idx)==0)
        return false;

    return true;
}


float StreamlineTrackingFilter::GetRandDouble(float min, float max)
{
    return (float)(rand()%((int)(10000*(max-min))) + 10000*min)/10000;
}


std::vector< vnl_vector_fixed<float,3> > StreamlineTrackingFilter::CreateDirections(int NPoints)
{
    std::vector< vnl_vector_fixed<float,3> > pointshell;

    if (NPoints<2)
        return pointshell;

    std::vector< float > theta; theta.resize(NPoints);

    std::vector< float > phi; phi.resize(NPoints);

    float C = sqrt(4*M_PI);

    phi[0] = 0.0;
    phi[NPoints-1] = 0.0;

    for(int i=0; i<NPoints; i++)
    {
        theta[i] = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
        if( i>0 && i<NPoints-1)
        {
            phi[i] = (phi[i-1] + C / sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
            // % (2*DIST_POINTSHELL_PI);
        }
    }


    for(int i=0; i<NPoints; i++)
    {
        vnl_vector_fixed<float,3> d;
        d[0] = cos(theta[i]) * cos(phi[i]);
        d[1] = cos(theta[i]) * sin(phi[i]);
        d[2] = sin(theta[i]);
        pointshell.push_back(d);
    }

    return pointshell;
}


vnl_vector_fixed<float,3> StreamlineTrackingFilter::GetNewDirection(itk::Point<float, 3> &pos, std::deque<vnl_vector_fixed<float, 3> >& olddirs, itk::Index<3> &oldIndex)
{
    if (m_DemoMode)
    {
        m_SamplingPointset->Clear();
        m_AlternativePointset->Clear();
        m_StopVotePointset->Clear();
    }
    vnl_vector_fixed<float,3> direction; direction.fill(0);

    ItkUcharImgType::IndexType idx;
    m_MaskImage->TransformPhysicalPointToIndex(pos, idx);

    if (IsValidPosition(pos))
    {
        if (m_StoppingRegions->GetPixel(idx)>0)
            return direction;
        direction = m_TrackingHandler->ProposeDirection(pos, olddirs, oldIndex); // get direction proposal at current streamline position
    }
    else
        return direction;

    vnl_vector_fixed<float,3> olddir = olddirs.back();
    std::vector< vnl_vector_fixed<float,3> > probeVecs = CreateDirections(m_NumberOfSamples);
    itk::Point<float, 3> sample_pos;
    int alternatives = 1;
    int stop_votes = 0;
    int possible_stop_votes = 0;
    for (int i=0; i<probeVecs.size(); i++)
    {
        vnl_vector_fixed<float,3> d;
        bool is_stop_voter = false;
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
            d = probeVecs.at(i);
            float dot = dot_product(d, olddir);
            if (m_UseStopVotes && dot>0.7)
            {
                is_stop_voter = true;
                possible_stop_votes++;
            }
            else if (m_OnlyForwardSamples && dot<0)
                continue;
            d *= m_SamplingDistance;
        }

        sample_pos[0] = pos[0] + d[0];
        sample_pos[1] = pos[1] + d[1];
        sample_pos[2] = pos[2] + d[2];

        vnl_vector_fixed<float,3> tempDir; tempDir.fill(0.0);
        if (IsValidPosition(sample_pos))
            tempDir = m_TrackingHandler->ProposeDirection(sample_pos, olddirs, oldIndex); // sample neighborhood
        if (tempDir.magnitude()>mitk::eps)
        {
            direction += tempDir;

            if(m_DemoMode)
                m_SamplingPointset->InsertPoint(i, sample_pos);
        }
        else if (m_AvoidStop && olddir.magnitude()>0.5) // out of white matter
        {
            if (is_stop_voter)
                stop_votes++;
            if (m_DemoMode)
                m_StopVotePointset->InsertPoint(i, sample_pos);

            float dot = dot_product(d, olddir);
            if (dot >= 0.0) // in front of plane defined by pos and olddir
                d = -d + 2*dot*olddir; // reflect
            else
                d = -d; // invert

            // look a bit further into the other direction
            sample_pos[0] = pos[0] + d[0];
            sample_pos[1] = pos[1] + d[1];
            sample_pos[2] = pos[2] + d[2];
            alternatives++;
            vnl_vector_fixed<float,3> tempDir; tempDir.fill(0.0);
            if (IsValidPosition(sample_pos))
                tempDir = m_TrackingHandler->ProposeDirection(sample_pos, olddirs, oldIndex); // sample neighborhood

            if (tempDir.magnitude()>mitk::eps)  // are we back in the white matter?
            {
                direction += d * m_DeflectionMod;         // go into the direction of the white matter
                direction += tempDir;  // go into the direction of the white matter direction at this location

                if(m_DemoMode)
                    m_AlternativePointset->InsertPoint(alternatives, sample_pos);
            }
            else
            {
                if (m_DemoMode)
                    m_StopVotePointset->InsertPoint(i, sample_pos);
            }
        }
        else
        {
            if (m_DemoMode)
                m_StopVotePointset->InsertPoint(i, sample_pos);

            if (is_stop_voter)
                stop_votes++;
        }
    }

    if (direction.magnitude()>0.001 && (possible_stop_votes==0 || (float)stop_votes/possible_stop_votes<0.5) )
        direction.normalize();
    else
        direction.fill(0);

    return direction;
}


float StreamlineTrackingFilter::FollowStreamline(itk::Point<float, 3> pos, vnl_vector_fixed<float,3> dir, FiberType* fib, float tractLength, bool front)
{
    vnl_vector_fixed<float,3> zero_dir; zero_dir.fill(0.0);
    std::deque< vnl_vector_fixed<float,3> > last_dirs;
    for (unsigned int i=0; i<m_NumPreviousDirections-1; i++)
        last_dirs.push_back(zero_dir);

    for (int step=0; step< m_MaxLength/2; step++)
    {
        ItkUcharImgType::IndexType oldIndex;
        m_StoppingRegions->TransformPhysicalPointToIndex(pos, oldIndex);

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
                int curv = CheckCurvature(fib, front);
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

#pragma omp critical
        if (m_DemoMode) // CHECK: warum sind die samplingpunkte der streamline in der visualisierung immer einen schritt voras?
        {
            m_BuildFibersReady++;
            m_Tractogram.push_back(*fib);
            BuildFibers(true);
            m_Stop = true;

            while (m_Stop){
            }
        }

        dir.normalize();
        last_dirs.push_back(dir);
        if (last_dirs.size()>m_NumPreviousDirections)
            last_dirs.pop_front();
        dir = GetNewDirection(pos, last_dirs, oldIndex);

        while (m_PauseTracking){}

        if (dir.magnitude()<0.0001)
            return tractLength;
    }
    return tractLength;
}


int StreamlineTrackingFilter::CheckCurvature(FiberType* fib, bool front)
{
    float m_Distance = 5;
    if (fib->size()<3)
        return 0;

    float dist = 0;
    std::vector< vnl_vector_fixed< float, 3 > > vectors;
    vnl_vector_fixed< float, 3 > meanV; meanV.fill(0);
    float dev = 0;

    if (front)
    {
        int c=0;
        while(dist<m_Distance && c<fib->size()-1)
        {
            itk::Point<float> p1 = fib->at(c);
            itk::Point<float> p2 = fib->at(c+1);

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
            itk::Point<float> p1 = fib->at(c);
            itk::Point<float> p2 = fib->at(c-1);

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
        float angle = dot_product(meanV, vectors.at(c));
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


void StreamlineTrackingFilter::GenerateData()
{
    this->BeforeTracking();

    std::vector< itk::Point<float> > seedpoints;

    if (!m_ControlGmEndings)
    {
        typedef ImageRegionConstIterator< ItkUcharImgType >     MaskIteratorType;
        MaskIteratorType    sit(m_SeedImage, m_SeedImage->GetLargestPossibleRegion() );
        MaskIteratorType    mit(m_MaskImage, m_MaskImage->GetLargestPossibleRegion() );
        sit.GoToBegin();
        mit.GoToBegin();

        while( !sit.IsAtEnd() )
        {
            if (sit.Value()==0 || mit.Value()==0 || (m_SeedOnlyGm && m_TissueImage->GetPixel(sit.GetIndex())!=m_GmLabel))
            {
                ++sit;
                ++mit;
                continue;
            }

            for (int s=0; s<m_SeedsPerVoxel; s++)
            {
                ItkUcharImgType::IndexType index = sit.GetIndex();
                itk::ContinuousIndex<float, 3> start;

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

                itk::Point<float> worldPos;
                m_SeedImage->TransformContinuousIndexToPhysicalPoint( start, worldPos );
                seedpoints.push_back(worldPos);
            }
            ++sit;
            ++mit;
        }
    }
    else
    {
        for (auto s : m_GmStubs)
            seedpoints.push_back(s[1]);
    }

    if (m_Random)
    {
        std::srand(std::time(0));
        std::random_shuffle ( seedpoints.begin(), seedpoints.end() );
    }

    bool stop = false;
    unsigned int current_tracts = 0;
    int num_seeds = seedpoints.size();
    itk::Index<3> zeroIndex; zeroIndex.Fill(0);
    int progress = 0;
    int i = 0;
    int print_interval = num_seeds/100;
    if (print_interval<100)
        m_Verbose=false;

#pragma omp parallel
    while (i<num_seeds && !stop)
    {
        int temp_i = i;
#pragma omp critical
            i++;

        if (temp_i>=num_seeds || stop)
            continue;
        else if (m_Verbose && i%print_interval==0)
#pragma omp critical
        {
            progress += print_interval;
            std::cout << "                                                                                                     \r";
            if (m_MaxNumTracts>0)
                std::cout << "Tried: " << progress << "/" << num_seeds << " | Accepted: " << current_tracts << "/" << m_MaxNumTracts << '\r';
            else
                std::cout << "Tried: " << progress << "/" << num_seeds << " | Accepted: " << current_tracts << '\r';
            cout.flush();
        }

        itk::Point<float> worldPos = seedpoints.at(temp_i);
        FiberType fib;
        float tractLength = 0;
        unsigned int counter = 0;

        // get starting direction
        vnl_vector_fixed<float,3> dir; dir.fill(0.0);
        std::deque< vnl_vector_fixed<float,3> > olddirs;
        while (olddirs.size()<m_NumPreviousDirections)
            olddirs.push_back(dir); // start without old directions (only zero directions)

        vnl_vector_fixed< float, 3 > gm_start_dir;
        if (m_ControlGmEndings)
        {
            gm_start_dir[0] = m_GmStubs[temp_i][1][0] - m_GmStubs[temp_i][0][0];
            gm_start_dir[1] = m_GmStubs[temp_i][1][1] - m_GmStubs[temp_i][0][1];
            gm_start_dir[2] = m_GmStubs[temp_i][1][2] - m_GmStubs[temp_i][0][2];
            gm_start_dir.normalize();
            olddirs.pop_back();
            olddirs.push_back(gm_start_dir);
        }

        if (IsValidPosition(worldPos))
            dir = m_TrackingHandler->ProposeDirection(worldPos, olddirs, zeroIndex);

        if (dir.magnitude()>0.0001)
        {
            if (m_ControlGmEndings)
            {
                float a = dot_product(gm_start_dir, dir);
                if (a<0)
                    dir = -dir;
            }

            // forward tracking
            tractLength = FollowStreamline(worldPos, dir, &fib, 0, false);
            fib.push_front(worldPos);

            if (m_ControlGmEndings)
            {
                fib.push_front(m_GmStubs[temp_i][0]);
                CheckFiberForGmEnding(&fib);
            }
            else
            {
                // backward tracking (only if we don't explicitely start in the GM)
                tractLength = FollowStreamline(worldPos, -dir, &fib, tractLength, true);
                if (m_ControlGmEndings)
                {
                    CheckFiberForGmEnding(&fib);
                    std::reverse(fib.begin(),fib.end());
                    CheckFiberForGmEnding(&fib);
                }
            }
            counter = fib.size();

#pragma omp critical
            if (tractLength>=m_MinTractLength && counter>=2)
            {
                if (!stop)
                {
                    m_Tractogram.push_back(fib);
                    current_tracts++;
                }
                if (m_MaxNumTracts > 0 && current_tracts>=static_cast<unsigned int>(m_MaxNumTracts))
                {
                    if (!stop)
                    {
                        std::cout << "                                                                                                     \r";
                        MITK_INFO << "Reconstructed maximum number of tracts (" << current_tracts << "). Stopping tractography.";
                    }
                    stop = true;
                }
            }
        }
    }

    this->AfterTracking();
}



void StreamlineTrackingFilter::CheckFiberForGmEnding(FiberType* fib)
{
    if (m_TissueImage.IsNull())
        return;

    // first check if the current fibe rendpoint is located inside of the white matter
    // if not, remove last fiber point and repeat
    bool in_wm = false;
    while (!in_wm && fib->size()>2)
    {
        ItkUcharImgType::IndexType idx;
        m_TissueImage->TransformPhysicalPointToIndex(fib->back(), idx);
        if (m_TissueImage->GetPixel(idx)==m_WmLabel)
            in_wm = true;
        else
            fib->pop_back();
    }
    if (fib->size()<3 || !in_wm)
    {
        fib->clear();
        return;
    }

    // get fiber direction at end point
    vnl_vector_fixed< float, 3 > d1;
    d1[0] = fib->back()[0] - fib->at(fib->size()-2)[0];
    d1[1] = fib->back()[1] - fib->at(fib->size()-2)[1];
    d1[2] = fib->back()[2] - fib->at(fib->size()-2)[2];
    d1.normalize();

    // find closest gray matter voxel
    ItkUcharImgType::IndexType s_idx;
    m_TissueImage->TransformPhysicalPointToIndex(fib->back(), s_idx);
    itk::Point<float> gm_endp;
    float max = -1;

    for (int x : {-1,0,1})
        for (int y : {-1,0,1})
            for (int z : {-1,0,1})
            {
                if (x==y && y==z)
                    continue;

                ItkUcharImgType::IndexType e_idx;
                e_idx[0] = s_idx[0] + x;
                e_idx[1] = s_idx[1] + y;
                e_idx[2] = s_idx[2] + z;

                if ( !m_TissueImage->GetLargestPossibleRegion().IsInside(e_idx) || m_TissueImage->GetPixel(e_idx)!=m_GmLabel )
                    continue;

                itk::ContinuousIndex<float, 3> end;
                m_TissueImage->TransformIndexToPhysicalPoint(e_idx, end);
                vnl_vector_fixed< float, 3 > d2;
                d2[0] = end[0] - fib->back()[0];
                d2[1] = end[1] - fib->back()[1];
                d2[2] = end[2] - fib->back()[2];
                d2.normalize();
                float a = dot_product(d1,d2);
                if (a>max)
                {
                    max = a;
                    gm_endp = end;
                }
            }

    if (max>=0)
        fib->push_back(gm_endp);
    else  // no gray matter enpoint found -> delete fiber
        fib->clear();
}


void StreamlineTrackingFilter::BuildFibers(bool check)
{
    if (m_BuildFibersReady<omp_get_num_threads() && check)
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


void StreamlineTrackingFilter::AfterTracking()
{
    std::cout << "                                                                                                     \r";
    MITK_INFO << "Reconstructed " << m_Tractogram.size() << " fibers.";
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
