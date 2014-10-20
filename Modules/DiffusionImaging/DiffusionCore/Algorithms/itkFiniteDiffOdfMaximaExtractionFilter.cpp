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
#ifndef __itkFiniteDiffOdfMaximaExtractionFilter_cpp
#define __itkFiniteDiffOdfMaximaExtractionFilter_cpp

#include "itkFiniteDiffOdfMaximaExtractionFilter.h"
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIterator.h>
#include <vnl/vnl_vector.h>
#include <itkOrientationDistributionFunction.h>
#include <itkContinuousIndex.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>

#include <boost/math/special_functions.hpp>
#include <boost/progress.hpp>


using namespace boost::math;

namespace itk {


static bool CompareVectors(const vnl_vector_fixed< double, 3 >& v1, const vnl_vector_fixed< double, 3 >& v2)
{
    return (v1.magnitude()>v2.magnitude());
}

template< class PixelType, int ShOrder, int NrOdfDirections >
FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::FiniteDiffOdfMaximaExtractionFilter()
    : m_NormalizationMethod(MAX_VEC_NORM)
    , m_MaxNumPeaks(2)
    , m_PeakThreshold(0.4)
    , m_AbsolutePeakThreshold(0)
    , m_ClusteringThreshold(0.9)
    , m_AngularThreshold(0.7)
    , m_NumCoeffs((ShOrder*ShOrder + ShOrder + 2)/2 + ShOrder)
    , m_Toolkit(FSL)
{
    this->SetNumberOfRequiredInputs(1);
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::FindCandidatePeaks(OdfType& odf, double thr, std::vector< DirectionType >& container)
{
    double gfa = odf.GetGeneralizedFractionalAnisotropy();
    //Find the peaks using a finite difference method
    bool flag = true;
    vnl_vector_fixed< bool, NrOdfDirections > used; used.fill(false);
    //Find the peaks
    for (int i=0; i<NrOdfDirections; i++)
    {
        if (used[i])
            continue;

        double val = odf.GetElement(i);
        if (val>thr && val*gfa>m_AbsolutePeakThreshold)  // limit to one hemisphere ???
        {
            flag = true;
            std::vector< int > neighbours = odf.GetNeighbors(i);
            for (unsigned int j=0; j<neighbours.size(); j++)
                if (val<=odf.GetElement(neighbours.at(j)))
                {
                    flag = false;
                    break;
                }
            if (flag)   // point is a peak
            {
                container.push_back(odf.GetDirection(i).normalize());
                used[i] = true;
                for (unsigned int j=0; j<neighbours.size(); j++)
                    used[neighbours.at(j)] = true;
            }
        }
    }
}

template< class PixelType, int ShOrder, int NrOdfDirections >
std::vector< vnl_vector_fixed< double, 3 > >  FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>::MeanShiftClustering(std::vector< vnl_vector_fixed< double, 3 > >& inDirs)
{
    std::vector< DirectionType > outDirs;
    if (inDirs.empty())
        return inDirs;

    DirectionType oldMean, currentMean, workingMean;
    std::vector< int > touched;

    // initialize
    touched.resize(inDirs.size(), 0);
    bool free = true;
    currentMean = inDirs[0];  // initialize first seed
    while (free)
    {
        oldMean.fill(0.0);

        // start mean-shift clustering
        float angle = 0.0;
        int counter = 0;
        while ((currentMean-oldMean).magnitude()>0.0001)
        {
            counter = 0;
            oldMean = currentMean;
            workingMean = oldMean;
            workingMean.normalize();
            currentMean.fill(0.0);
            for (unsigned int i=0; i<inDirs.size(); i++)
            {
                angle = dot_product(workingMean, inDirs[i]);
                if (angle>=m_ClusteringThreshold)
                {
                    currentMean += inDirs[i];
                    touched[i] = 1;
                    counter++;
                }
                else if (-angle>=m_ClusteringThreshold)
                {
                    currentMean -= inDirs[i];
                    touched[i] = 1;
                    counter++;
                }
            }
        }

        // found stable mean
        if (counter>0)
        {
            float mag = currentMean.magnitude();
            if (mag>0)
            {
                currentMean /= mag;
                outDirs.push_back(currentMean);
            }
        }

        // find next unused seed
        free = false;
        for (unsigned int i=0; i<touched.size(); i++)
            if (touched[i]==0)
            {
                currentMean = inDirs[i];
                free = true;
            }
    }

    if (inDirs.size()==outDirs.size())
    {
        return outDirs;
    }
    else
        return MeanShiftClustering(outDirs);
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::BeforeThreadedGenerateData()
{
    typename CoefficientImageType::Pointer ShCoeffImage = static_cast< CoefficientImageType* >( this->ProcessObject::GetInput(0) );
    itk::Vector<double,3> spacing = ShCoeffImage->GetSpacing();
    double minSpacing = spacing[0];
    if (spacing[1]<minSpacing)
        minSpacing = spacing[1];
    if (spacing[2]<minSpacing)
        minSpacing = spacing[2];

    mitk::Point3D origin = ShCoeffImage->GetOrigin();
    itk::Matrix<double, 3, 3> direction = ShCoeffImage->GetDirection();
    ImageRegion<3> imageRegion = ShCoeffImage->GetLargestPossibleRegion();

    if (m_MaskImage.IsNotNull())
    {
        origin = m_MaskImage->GetOrigin();
        direction = m_MaskImage->GetDirection();
        imageRegion = m_MaskImage->GetLargestPossibleRegion();
    }

    m_DirectionImageContainer = ItkDirectionImageContainer::New();
    for (unsigned int i=0; i<m_MaxNumPeaks; i++)
    {
        itk::Vector< float, 3 > nullVec; nullVec.Fill(0.0);
        ItkDirectionImage::Pointer img = ItkDirectionImage::New();
        img->SetSpacing( spacing );
        img->SetOrigin( origin );
        img->SetDirection( direction );
        img->SetRegions( imageRegion );
        img->Allocate();
        img->FillBuffer(nullVec);
        m_DirectionImageContainer->InsertElement(m_DirectionImageContainer->Size(), img);
    }
    if (m_MaskImage.IsNull())
    {
        m_MaskImage = ItkUcharImgType::New();
        m_MaskImage->SetSpacing( spacing );
        m_MaskImage->SetOrigin( origin );
        m_MaskImage->SetDirection( direction );
        m_MaskImage->SetRegions( imageRegion );
        m_MaskImage->Allocate();
        m_MaskImage->FillBuffer(1);
    }
    m_NumDirectionsImage = ItkUcharImgType::New();
    m_NumDirectionsImage->SetSpacing( spacing );
    m_NumDirectionsImage->SetOrigin( origin );
    m_NumDirectionsImage->SetDirection( direction );
    m_NumDirectionsImage->SetRegions( imageRegion );
    m_NumDirectionsImage->Allocate();
    m_NumDirectionsImage->FillBuffer(0);

    this->SetNumberOfIndexedOutputs(m_MaxNumPeaks);

    // calculate SH basis
    OdfType odf;
    vnl_matrix_fixed<double, 3, NrOdfDirections>* directions = odf.GetDirections();
    vnl_matrix< double > sphCoords;
    std::vector< DirectionType > dirs;
    for (int i=0; i<NrOdfDirections; i++)
        dirs.push_back(directions->get_column(i));
    Cart2Sph(dirs, sphCoords);                          // convert candidate peaks to spherical angles
    m_ShBasis = CalcShBasis(sphCoords);                // evaluate spherical harmonics at each peak

    MITK_INFO << "Starting finite differences maximum extraction";
    MITK_INFO << "ODF sampling points: " << NrOdfDirections;
    MITK_INFO << "SH order: " << ShOrder;
    MITK_INFO << "Maximum peaks: " << m_MaxNumPeaks;
    MITK_INFO << "Relative threshold: " << m_PeakThreshold;
    MITK_INFO << "Absolute threshold: " << m_AbsolutePeakThreshold;
    MITK_INFO << "Clustering threshold: " << m_ClusteringThreshold;
    MITK_INFO << "Angular threshold: " << m_AngularThreshold;
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::AfterThreadedGenerateData()
{
    MITK_INFO << "Generating vector field";
    vtkSmartPointer<vtkCellArray> m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPoints>    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();

    typename CoefficientImageType::Pointer ShCoeffImage = static_cast< CoefficientImageType* >( this->ProcessObject::GetInput(0) );
    ImageRegionConstIterator< CoefficientImageType > cit(ShCoeffImage, ShCoeffImage->GetLargestPossibleRegion() );

    mitk::Vector3D spacing = ShCoeffImage->GetSpacing();
    double minSpacing = spacing[0];
    if (spacing[1]<minSpacing)
        minSpacing = spacing[1];
    if (spacing[2]<minSpacing)
        minSpacing = spacing[2];

    int maxProgress = ShCoeffImage->GetLargestPossibleRegion().GetSize()[0]*ShCoeffImage->GetLargestPossibleRegion().GetSize()[1]*ShCoeffImage->GetLargestPossibleRegion().GetSize()[2];
    boost::progress_display disp(maxProgress);

    while( !cit.IsAtEnd() )
    {
        ++disp;

        typename CoefficientImageType::IndexType index = cit.GetIndex();
        if (m_MaskImage->GetPixel(index)==0)
        {
            ++cit;
            continue;
        }

        for (unsigned int i=0; i<m_DirectionImageContainer->Size(); i++)
        {
            ItkDirectionImage::Pointer img = m_DirectionImageContainer->GetElement(i);
            itk::Vector< float, 3 > pixel = img->GetPixel(index);
            DirectionType dir; dir[0] = pixel[0]; dir[1] = pixel[1]; dir[2] = pixel[2];

            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
            itk::ContinuousIndex<double, 3> center;
            center[0] = index[0];
            center[1] = index[1];
            center[2] = index[2];
            itk::Point<double> worldCenter;
            m_MaskImage->TransformContinuousIndexToPhysicalPoint( center, worldCenter );

            itk::Point<double> worldStart;
            worldStart[0] = worldCenter[0]-dir[0]/2 * minSpacing;
            worldStart[1] = worldCenter[1]-dir[1]/2 * minSpacing;
            worldStart[2] = worldCenter[2]-dir[2]/2 * minSpacing;
            vtkIdType id = m_VtkPoints->InsertNextPoint(worldStart.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            itk::Point<double> worldEnd;
            worldEnd[0] = worldCenter[0]+dir[0]/2 * minSpacing;
            worldEnd[1] = worldCenter[1]+dir[1]/2 * minSpacing;
            worldEnd[2] = worldCenter[2]+dir[2]/2 * minSpacing;
            id = m_VtkPoints->InsertNextPoint(worldEnd.GetDataPointer());
            container->GetPointIds()->InsertNextId(id);
            m_VtkCellArray->InsertNextCell(container);
        }
        ++cit;
    }

    vtkSmartPointer<vtkPolyData> directionsPolyData = vtkSmartPointer<vtkPolyData>::New();
    directionsPolyData->SetPoints(m_VtkPoints);
    directionsPolyData->SetLines(m_VtkCellArray);
    m_OutputFiberBundle = mitk::FiberBundleX::New(directionsPolyData);

    for (unsigned int i=0; i<m_DirectionImageContainer->Size(); i++)
    {
        ItkDirectionImage::Pointer img = m_DirectionImageContainer->GetElement(i);
        this->SetNthOutput(i, img);
    }
}

template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread, ThreadIdType threadID )
{
    typename CoefficientImageType::Pointer ShCoeffImage = static_cast< CoefficientImageType* >( this->ProcessObject::GetInput(0) );

    ImageRegionConstIterator< CoefficientImageType > cit(ShCoeffImage, outputRegionForThread );

    OdfType odf;
    while( !cit.IsAtEnd() )
    {
        typename CoefficientImageType::IndexType index = cit.GetIndex();
        if (m_MaskImage->GetPixel(index)==0)
        {
            ++cit;
            continue;
        }

        CoefficientPixelType c = cit.Get();

        // calculate ODF
        double max = 0;
        odf.Fill(0.0);
        for (int i=0; i<NrOdfDirections; i++)
        {
            for (int j=0; j<m_NumCoeffs; j++)
                odf[i] += c[j]*m_ShBasis(i,j);
            if (odf[i]>max)
                max = odf[i];
        }
        if (max<0.0001)
        {
            ++cit;
            continue;
        }

        std::vector< DirectionType > candidates, peaks, temp;
        peaks.clear();
        max *= m_PeakThreshold;                         // relative threshold
        FindCandidatePeaks(odf, max, candidates);       // find all local maxima
        candidates = MeanShiftClustering(candidates);   // cluster maxima

        vnl_matrix< double > shBasis, sphCoords;
        Cart2Sph(candidates, sphCoords);                // convert candidate peaks to spherical angles
        shBasis = CalcShBasis(sphCoords);            // evaluate spherical harmonics at each peak
        max = 0.0;
        for (unsigned int i=0; i<candidates.size(); i++)         // scale peaks according to ODF value
        {
            double val = 0;
            for (int j=0; j<m_NumCoeffs; j++)
                val += c[j]*shBasis(i,j);
            if (val>max)
                max = val;
            peaks.push_back(candidates[i]*val);
        }
        std::sort( peaks.begin(), peaks.end(), CompareVectors );  // sort peaks

        // kick out directions to close to a larger direction (too far away to cluster but too close to keep)
        unsigned int m = peaks.size();
        if ( m>m_DirectionImageContainer->Size() )
            m = m_DirectionImageContainer->Size();
        for (unsigned int i=0; i<m; i++)
        {
            DirectionType v1 = peaks.at(i);
            double val = v1.magnitude();
            if (val<max*m_PeakThreshold || val<m_AbsolutePeakThreshold)
                break;

            bool flag = true;
            for (unsigned int j=0; j<peaks.size(); j++)
                if (i!=j)
                {
                    DirectionType v2 = peaks.at(j);
                    double val2 = v2.magnitude();
                    double angle = fabs(dot_product(v1,v2)/(val*val2));
                    if (angle>m_AngularThreshold && val<val2)
                    {
                        flag = false;
                        break;
                    }
                }

            if (flag)
                temp.push_back(v1);
        }
        peaks = temp;

        // fill output image
        unsigned int num = peaks.size();
        if ( num>m_DirectionImageContainer->Size() )
            num = m_DirectionImageContainer->Size();
        for (unsigned int i=0; i<num; i++)
        {
            vnl_vector<double> dir = peaks.at(i);

            ItkDirectionImage::Pointer img = m_DirectionImageContainer->GetElement(i);

            switch (m_NormalizationMethod)
            {
            case NO_NORM:
                break;
            case SINGLE_VEC_NORM:
                dir.normalize();
                break;
            case MAX_VEC_NORM:
                dir /= max;
                break;
            }

            dir = m_MaskImage->GetDirection()*dir;
            itk::Vector< float, 3 > pixel;
            pixel.SetElement(0, -dir[0]);
            pixel.SetElement(1, dir[1]);
            pixel.SetElement(2, dir[2]);
            img->SetPixel(index, pixel);
        }
        m_NumDirectionsImage->SetPixel(index, num);
        ++cit;
    }
    MITK_INFO << "Thread " << threadID << " finished extraction";
}

// convert cartesian to spherical coordinates
template< class PixelType, int ShOrder, int NrOdfDirections >
void FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::Cart2Sph(const std::vector< DirectionType >& dir, vnl_matrix<double>& sphCoords)
{
    sphCoords.set_size(dir.size(), 2);

    for (unsigned int i=0; i<dir.size(); i++)
    {
        double mag = dir[i].magnitude();

        if( mag<0.0001 )
        {
            sphCoords(i,0) = M_PI/2; // theta
            sphCoords(i,1) = M_PI/2; // phi
        }
        else
        {
            sphCoords(i,0) = acos(dir[i](2)/mag); // theta
            sphCoords(i,1) = atan2(dir[i](1), dir[i](0)); // phi
        }
    }
}

// generate spherical harmonic values of the desired order for each input direction
template< class PixelType, int ShOrder, int NrOdfDirections >
vnl_matrix<double> FiniteDiffOdfMaximaExtractionFilter< PixelType, ShOrder, NrOdfDirections>
::CalcShBasis(vnl_matrix<double>& sphCoords)
{
    int M = sphCoords.rows();
    int j, m; double mag, plm;
    vnl_matrix<double> shBasis;
    shBasis.set_size(M, m_NumCoeffs);

    for (int p=0; p<M; p++)
    {
        j=0;
        for (int l=0; l<=ShOrder; l=l+2)
            for (m=-l; m<=l; m++)
            {
                switch (m_Toolkit)
                {
                case FSL:
                plm = legendre_p<double>(l,abs(m),cos(sphCoords(p,0)));
                mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;

                if (m<0)
                    shBasis(p,j) = sqrt(2.0)*mag*cos(fabs((double)m)*sphCoords(p,1));
                else if (m==0)
                    shBasis(p,j) = mag;
                else
                    shBasis(p,j) = pow(-1.0, m)*sqrt(2.0)*mag*sin(m*sphCoords(p,1));
                    break;
                case MRTRIX:

                plm = legendre_p<double>(l,abs(m),-cos(sphCoords(p,0)));
                mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;
                if (m>0)
                    shBasis(p,j) = mag*cos(m*sphCoords(p,1));
                else if (m==0)
                    shBasis(p,j) = mag;
                else
                    shBasis(p,j) = mag*sin(-m*sphCoords(p,1));
                    break;
                }

                j++;
            }
    }
    return shBasis;
}

}

#endif // __itkFiniteDiffOdfMaximaExtractionFilter_cpp
