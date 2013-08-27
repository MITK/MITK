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

#include "itkTractsToDWIImageFilter.h"
#include <boost/progress.hpp>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkCastImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkResampleDwiImageFilter.h>
#include <itkKspaceImageFilter.h>
#include <itkDftImageFilter.h>
#include <itkAddImageFilter.h>

namespace itk
{

template< class PixelType >
TractsToDWIImageFilter< PixelType >::TractsToDWIImageFilter()
    : m_CircleDummy(false)
    , m_VolumeAccuracy(10)
    , m_Upsampling(1)
    , m_NumberOfRepetitions(1)
    , m_EnforcePureFiberVoxels(false)
    , m_InterpolationShrink(1000)
    , m_FiberRadius(0)
    , m_SignalScale(25)
    , m_kOffset(0)
    , m_tLine(1)
    , m_UseInterpolation(false)
    , m_SimulateRelaxation(true)
    , m_tInhom(50)
    , m_TE(100)
    , m_FrequencyMap(NULL)
    , m_EddyGradientStrength(0.001)
    , m_SimulateEddyCurrents(false)
    , m_Spikes(0)
    , m_SpikeAmplitude(1)
{
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);
}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::~TractsToDWIImageFilter()
{

}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::DoubleDwiType::Pointer TractsToDWIImageFilter< PixelType >::DoKspaceStuff( std::vector< DoubleDwiType::Pointer >& images )
{
    // create slice object
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
    sliceRegion.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
    Vector< double, 2 > sliceSpacing;
    sliceSpacing[0] = m_UpsampledSpacing[0];
    sliceSpacing[1] = m_UpsampledSpacing[1];

    // frequency map slice
    SliceType::Pointer fMap = NULL;
    if (m_FrequencyMap.IsNotNull())
    {
        fMap = SliceType::New();
        ImageRegion<2> region;
        region.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
        region.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
        fMap->SetLargestPossibleRegion( region );
        fMap->SetBufferedRegion( region );
        fMap->SetRequestedRegion( region );
        fMap->Allocate();
    }

    DoubleDwiType::Pointer newImage = DoubleDwiType::New();
    newImage->SetSpacing( m_Spacing );
    newImage->SetOrigin( m_Origin );
    newImage->SetDirection( m_DirectionMatrix );
    newImage->SetLargestPossibleRegion( m_ImageRegion );
    newImage->SetBufferedRegion( m_ImageRegion );
    newImage->SetRequestedRegion( m_ImageRegion );
    newImage->SetVectorLength( images.at(0)->GetVectorLength() );
    newImage->Allocate();

    MatrixType transform = m_DirectionMatrix;
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
        {
            if (j<2)
                transform[i][j] *= m_UpsampledSpacing[j];
            else
                transform[i][j] *= m_Spacing[j];
        }

    std::vector< int > spikeVolume;
    for (int i=0; i<m_Spikes; i++)
        spikeVolume.push_back(rand()%images.at(0)->GetVectorLength());
    std::sort (spikeVolume.begin(), spikeVolume.end());
    std::reverse (spikeVolume.begin(), spikeVolume.end());

    boost::progress_display disp(images.at(0)->GetVectorLength()*images.at(0)->GetLargestPossibleRegion().GetSize(2));
    for (int g=0; g<images.at(0)->GetVectorLength(); g++)
    {
        std::vector< int > spikeSlice;
        while (!spikeVolume.empty() && spikeVolume.back()==g)
        {
            spikeSlice.push_back(rand()%images.at(0)->GetLargestPossibleRegion().GetSize(2));
            spikeVolume.pop_back();
        }
        std::sort (spikeSlice.begin(), spikeSlice.end());
        std::reverse (spikeSlice.begin(), spikeSlice.end());

        for (int z=0; z<images.at(0)->GetLargestPossibleRegion().GetSize(2); z++)
        {
            std::vector< SliceType::Pointer > compartmentSlices;
            std::vector< double > t2Vector;

            for (int i=0; i<images.size(); i++)
            {
                DiffusionSignalModel<double>* signalModel;
                if (i<m_FiberModels.size())
                    signalModel = m_FiberModels.at(i);
                else
                    signalModel = m_NonFiberModels.at(i-m_FiberModels.size());

                SliceType::Pointer slice = SliceType::New();
                slice->SetLargestPossibleRegion( sliceRegion );
                slice->SetBufferedRegion( sliceRegion );
                slice->SetRequestedRegion( sliceRegion );
                slice->SetSpacing(sliceSpacing);
                slice->Allocate();
                slice->FillBuffer(0.0);

                // extract slice from channel g
                for (int y=0; y<images.at(0)->GetLargestPossibleRegion().GetSize(1); y++)
                    for (int x=0; x<images.at(0)->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        SliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;
                        DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        slice->SetPixel(index2D, images.at(i)->GetPixel(index3D)[g]);

                        if (fMap.IsNotNull() && i==0)
                            fMap->SetPixel(index2D, m_FrequencyMap->GetPixel(index3D));
                    }

                compartmentSlices.push_back(slice);
                t2Vector.push_back(signalModel->GetT2());
            }

            // create k-sapce (inverse fourier transform slices)
            itk::Size<2> outSize; outSize.SetElement(0, m_ImageRegion.GetSize(0)); outSize.SetElement(1, m_ImageRegion.GetSize(1));
            itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
            idft->SetCompartmentImages(compartmentSlices);
            idft->SetT2(t2Vector);
            idft->SetkOffset(m_kOffset);
            idft->SettLine(m_tLine);
            idft->SetTE(m_TE);
            idft->SetTinhom(m_tInhom);
            idft->SetSimulateRelaxation(m_SimulateRelaxation);
            idft->SetSimulateEddyCurrents(m_SimulateEddyCurrents);
            idft->SetEddyGradientMagnitude(m_EddyGradientStrength);
            idft->SetZ((double)z-(double)images.at(0)->GetLargestPossibleRegion().GetSize(2)/2.0);
            idft->SetDirectionMatrix(transform);
            idft->SetDiffusionGradientDirection(m_FiberModels.at(0)->GetGradientDirection(g));
            idft->SetFrequencyMap(fMap);
            idft->SetSignalScale(m_SignalScale);
            idft->SetOutSize(outSize);
            int numSpikes = 0;
            while (!spikeSlice.empty() && spikeSlice.back()==z)
            {
                numSpikes++;
                spikeSlice.pop_back();
            }
            idft->SetSpikes(numSpikes);
            idft->SetSpikeAmplitude(m_SpikeAmplitude);
            idft->Update();

            ComplexSliceType::Pointer fSlice;
            fSlice = idft->GetOutput();

            for (int i=0; i<m_KspaceArtifacts.size(); i++)
                fSlice = m_KspaceArtifacts.at(i)->AddArtifact(fSlice);

            // fourier transform slice
            SliceType::Pointer newSlice;
            itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
            dft->SetInput(fSlice);
            dft->Update();
            newSlice = dft->GetOutput();

            // put slice back into channel g
            for (int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
                for (int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
                {
                    DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
                    SliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;

                    DoubleDwiType::PixelType pix3D = newImage->GetPixel(index3D);
                    pix3D[g] = newSlice->GetPixel(index2D);
                    newImage->SetPixel(index3D, pix3D);
                }

            ++disp;
        }
    }
    return newImage;
}

//template< class PixelType >
//TractsToDWIImageFilter< PixelType >::ComplexSliceType::Pointer TractsToDWIImageFilter< PixelType >::RearrangeSlice(ComplexSliceType::Pointer slice)
//{
//    ImageRegion<2> region = slice->GetLargestPossibleRegion();
//    ComplexSliceType::Pointer rearrangedSlice = ComplexSliceType::New();
//    rearrangedSlice->SetLargestPossibleRegion( region );
//    rearrangedSlice->SetBufferedRegion( region );
//    rearrangedSlice->SetRequestedRegion( region );
//    rearrangedSlice->Allocate();

//    int xHalf = region.GetSize(0)/2;
//    int yHalf = region.GetSize(1)/2;

//    for (int y=0; y<region.GetSize(1); y++)
//        for (int x=0; x<region.GetSize(0); x++)
//        {
//            SliceType::IndexType idx;
//            idx[0]=x; idx[1]=y;
//            vcl_complex< double > pix = slice->GetPixel(idx);

//            if( idx[0] <  xHalf )
//                idx[0] = idx[0] + xHalf;
//            else
//                idx[0] = idx[0] - xHalf;

//            if( idx[1] <  yHalf )
//                idx[1] = idx[1] + yHalf;
//            else
//                idx[1] = idx[1] - yHalf;

//            rearrangedSlice->SetPixel(idx, pix);
//        }

//    return rearrangedSlice;
//}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::GenerateData()
{
    // check input data
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    int numFibers = m_FiberBundle->GetNumFibers();
    if (numFibers<=0)
        itkExceptionMacro("Input fiber bundle contains no fibers!");

    if (m_FiberModels.empty())
        itkExceptionMacro("No diffusion model for fiber compartments defined!");

    if (m_EnforcePureFiberVoxels)
        while (m_FiberModels.size()>1)
            m_FiberModels.pop_back();

    if (m_NonFiberModels.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined!");

    int baselineIndex = m_FiberModels[0]->GetFirstBaselineIndex();
    if (baselineIndex<0)
        itkExceptionMacro("No baseline index found!");

    // check k-space undersampling
    if (m_Upsampling<1)
        m_Upsampling = 1;

    if (m_TissueMask.IsNotNull())
    {
        // use input tissue mask
        m_Spacing = m_TissueMask->GetSpacing();
        m_Origin = m_TissueMask->GetOrigin();
        m_DirectionMatrix = m_TissueMask->GetDirection();
        m_ImageRegion = m_TissueMask->GetLargestPossibleRegion();

        if (m_Upsampling>1.00001)
        {
            MITK_INFO << "Adding ringing artifacts. Image upsampling factor: " << m_Upsampling;
            ImageRegion<3> region = m_ImageRegion;
            region.SetSize(0, m_ImageRegion.GetSize(0)*m_Upsampling);
            region.SetSize(1, m_ImageRegion.GetSize(1)*m_Upsampling);
            itk::Vector<double> spacing = m_Spacing;
            spacing[0] /= m_Upsampling;
            spacing[1] /= m_Upsampling;
            itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::Pointer rescaler = itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::New();
            rescaler->SetInput(0,m_TissueMask);
            rescaler->SetOutputMaximum(100);
            rescaler->SetOutputMinimum(0);
            rescaler->Update();

            itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer resampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
            resampler->SetInput(rescaler->GetOutput());
            resampler->SetOutputParametersFromImage(m_TissueMask);
            resampler->SetSize(region.GetSize());
            resampler->SetOutputSpacing(spacing);
            resampler->Update();
            m_TissueMask = resampler->GetOutput();
        }
        MITK_INFO << "Using tissue mask";
    }

    // initialize output dwi image
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( m_Origin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
    outImage->Allocate();
    typename OutputImageType::PixelType temp;
    temp.SetSize(m_FiberModels[0]->GetNumGradients());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);

    // is input slize size a power of two?
    int x=m_ImageRegion.GetSize(0); int y=m_ImageRegion.GetSize(1);
    if ( x%2 == 1 )
        x += 1;
    if ( y%2 == 1 )
        y += 1;

    // if not, adjust size and dimension (needed for FFT); zero-padding
    if (x!=m_ImageRegion.GetSize(0))
        m_ImageRegion.SetSize(0, x);
    if (y!=m_ImageRegion.GetSize(1))
        m_ImageRegion.SetSize(1, y);

    // apply undersampling to image parameters
    m_UpsampledSpacing = m_Spacing;
    m_UpsampledImageRegion = m_ImageRegion;
    m_UpsampledSpacing[0] /= m_Upsampling;
    m_UpsampledSpacing[1] /= m_Upsampling;
    m_UpsampledImageRegion.SetSize(0, m_ImageRegion.GetSize()[0]*m_Upsampling);
    m_UpsampledImageRegion.SetSize(1, m_ImageRegion.GetSize()[1]*m_Upsampling);

    // everything from here on is using the upsampled image parameters!!!
    if (m_TissueMask.IsNull())
    {
        m_TissueMask = ItkUcharImgType::New();
        m_TissueMask->SetSpacing( m_UpsampledSpacing );
        m_TissueMask->SetOrigin( m_Origin );
        m_TissueMask->SetDirection( m_DirectionMatrix );
        m_TissueMask->SetLargestPossibleRegion( m_UpsampledImageRegion );
        m_TissueMask->SetBufferedRegion( m_UpsampledImageRegion );
        m_TissueMask->SetRequestedRegion( m_UpsampledImageRegion );
        m_TissueMask->Allocate();
        m_TissueMask->FillBuffer(1);
    }

    // resample frequency map
    if (m_FrequencyMap.IsNotNull())
    {
        itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer resampler = itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
        resampler->SetInput(m_FrequencyMap);
        resampler->SetOutputParametersFromImage(m_FrequencyMap);
        resampler->SetSize(m_UpsampledImageRegion.GetSize());
        resampler->SetOutputSpacing(m_UpsampledSpacing);
        resampler->Update();
        m_FrequencyMap = resampler->GetOutput();
    }

    // initialize volume fraction images
    m_VolumeFractions.clear();
    for (int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        ItkDoubleImgType::Pointer tempimg = ItkDoubleImgType::New();
        tempimg->SetSpacing( m_UpsampledSpacing );
        tempimg->SetOrigin( m_Origin );
        tempimg->SetDirection( m_DirectionMatrix );
        tempimg->SetLargestPossibleRegion( m_UpsampledImageRegion );
        tempimg->SetBufferedRegion( m_UpsampledImageRegion );
        tempimg->SetRequestedRegion( m_UpsampledImageRegion );
        tempimg->Allocate();
        tempimg->FillBuffer(0);
        m_VolumeFractions.push_back(tempimg);
    }

    // resample fiber bundle for sufficient voxel coverage
    double segmentVolume = 0.0001;
    float minSpacing = 1;
    if(m_UpsampledSpacing[0]<m_UpsampledSpacing[1] && m_UpsampledSpacing[0]<m_UpsampledSpacing[2])
        minSpacing = m_UpsampledSpacing[0];
    else if (m_UpsampledSpacing[1] < m_UpsampledSpacing[2])
        minSpacing = m_UpsampledSpacing[1];
    else
        minSpacing = m_UpsampledSpacing[2];
    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    fiberBundle->ResampleFibers(minSpacing/m_VolumeAccuracy);
    double mmRadius = m_FiberRadius/1000;
    if (mmRadius>0)
        segmentVolume = M_PI*mmRadius*mmRadius*minSpacing/m_VolumeAccuracy;

    // generate double images to work with because we don't want to lose precision
    // we use a separate image for each compartment model
    std::vector< DoubleDwiType::Pointer > compartments;
    for (int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_UpsampledSpacing );
        doubleDwi->SetOrigin( m_Origin );
        doubleDwi->SetDirection( m_DirectionMatrix );
        doubleDwi->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleDwi->SetBufferedRegion( m_UpsampledImageRegion );
        doubleDwi->SetRequestedRegion( m_UpsampledImageRegion );
        doubleDwi->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
        doubleDwi->Allocate();
        DoubleDwiType::PixelType pix;
        pix.SetSize(m_FiberModels[0]->GetNumGradients());
        pix.Fill(0.0);
        doubleDwi->FillBuffer(pix);
        compartments.push_back(doubleDwi);
    }

    double interpFact = 2*atan(-0.5*m_InterpolationShrink);
    double maxVolume = 0;

    MITK_INFO << "Generating signal of " << m_FiberModels.size() << " fiber compartments";
    vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
    boost::progress_display disp(numFibers);
    for( int i=0; i<numFibers; i++ )
    {
        vtkCell* cell = fiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        if (numPoints<2)
            continue;

        for( int j=0; j<numPoints; j++)
        {
            double* temp = points->GetPoint(j);
            itk::Point<float, 3> vertex = GetItkPoint(temp);
            itk::Vector<double> v = GetItkVector(temp);

            itk::Vector<double, 3> dir(3);
            if (j<numPoints-1)
                dir = GetItkVector(points->GetPoint(j+1))-v;
            else
                dir = v-GetItkVector(points->GetPoint(j-1));

            itk::Index<3> idx;
            itk::ContinuousIndex<float, 3> contIndex;
            m_TissueMask->TransformPhysicalPointToIndex(vertex, idx);
            m_TissueMask->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

            if (!m_UseInterpolation)    // use nearest neighbour interpolation
            {
                if (!m_TissueMask->GetLargestPossibleRegion().IsInside(idx) || m_TissueMask->GetPixel(idx)<=0)
                    continue;

                // generate signal for each fiber compartment
                for (int k=0; k<m_FiberModels.size(); k++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(k);
                    m_FiberModels[k]->SetFiberDirection(dir);
                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(idx);
                    pix += segmentVolume*m_FiberModels[k]->SimulateMeasurement();
                    doubleDwi->SetPixel(idx, pix );
                    if (pix[baselineIndex]>maxVolume)
                        maxVolume = pix[baselineIndex];
                }

                continue;
            }

            double frac_x = contIndex[0] - idx[0];
            double frac_y = contIndex[1] - idx[1];
            double frac_z = contIndex[2] - idx[2];
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

            frac_x = atan((0.5-frac_x)*m_InterpolationShrink)/interpFact + 0.5;
            frac_y = atan((0.5-frac_y)*m_InterpolationShrink)/interpFact + 0.5;
            frac_z = atan((0.5-frac_z)*m_InterpolationShrink)/interpFact + 0.5;

            // use trilinear interpolation
            itk::Index<3> newIdx;
            for (int x=0; x<2; x++)
            {
                frac_x = 1-frac_x;
                for (int y=0; y<2; y++)
                {
                    frac_y = 1-frac_y;
                    for (int z=0; z<2; z++)
                    {
                        frac_z = 1-frac_z;

                        newIdx[0] = idx[0]+x;
                        newIdx[1] = idx[1]+y;
                        newIdx[2] = idx[2]+z;

                        double frac = frac_x*frac_y*frac_z;

                        // is position valid?
                        if (!m_TissueMask->GetLargestPossibleRegion().IsInside(newIdx) || m_TissueMask->GetPixel(newIdx)<=0)
                            continue;

                        // generate signal for each fiber compartment
                        for (int k=0; k<m_FiberModels.size(); k++)
                        {
                            DoubleDwiType::Pointer doubleDwi = compartments.at(k);
                            m_FiberModels[k]->SetFiberDirection(dir);
                            DoubleDwiType::PixelType pix = doubleDwi->GetPixel(newIdx);
                            pix += segmentVolume*frac*m_FiberModels[k]->SimulateMeasurement();
                            doubleDwi->SetPixel(newIdx, pix );
                            if (pix[baselineIndex]>maxVolume)
                                maxVolume = pix[baselineIndex];
                        }
                    }
                }
            }
        }
        ++disp;
    }

    MITK_INFO << "Generating signal of " << m_NonFiberModels.size() << " non-fiber compartments";
    ImageRegionIterator<ItkUcharImgType> it3(m_TissueMask, m_TissueMask->GetLargestPossibleRegion());
    boost::progress_display disp3(m_TissueMask->GetLargestPossibleRegion().GetNumberOfPixels());
    double voxelVolume = m_UpsampledSpacing[0]*m_UpsampledSpacing[1]*m_UpsampledSpacing[2];

    double fact = 1;
    if (m_FiberRadius<0.0001)
        fact = voxelVolume/maxVolume;

    while(!it3.IsAtEnd())
    {
        DoubleDwiType::IndexType index = it3.GetIndex();

        if (it3.Get()>0)
        {
            // get fiber volume fraction
            DoubleDwiType::Pointer fiberDwi = compartments.at(0);
            DoubleDwiType::PixelType fiberPix = fiberDwi->GetPixel(index); // intra axonal compartment
            if (fact>1) // auto scale intra-axonal if no fiber radius is specified
            {
                fiberPix *= fact;
                fiberDwi->SetPixel(index, fiberPix);
            }
            double f = fiberPix[baselineIndex];

            if (f>voxelVolume || f>0 && m_EnforcePureFiberVoxels)  // more fiber than space in voxel?
            {
                fiberDwi->SetPixel(index, fiberPix*voxelVolume/f);
                m_VolumeFractions.at(0)->SetPixel(index, 1);

//                for (int i=1; i<m_FiberModels.size(); i++)
//                {
//                    DoubleDwiType::PixelType pix; pix.Fill(0.0);
//                    compartments.at(i)->SetPixel(index, pix);
//                    m_VolumeFractions.at(i)->SetPixel(index, 1);
//                }
            }
            else
            {
                m_VolumeFractions.at(0)->SetPixel(index, f/voxelVolume);

                double nonf = voxelVolume-f;    // non-fiber volume
                double inter = 0;
                if (m_FiberModels.size()>1)
                    inter = nonf * f/voxelVolume;   // intra-axonal fraction of non fiber compartment scales linearly with f
                double other = nonf - inter;        // rest of compartment
                double singleinter = inter/(m_FiberModels.size()-1);

                // adjust non-fiber and intra-axonal signal
                for (int i=1; i<m_FiberModels.size(); i++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(i);
                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                    if (pix[baselineIndex]>0)
                        pix /= pix[baselineIndex];
                    pix *= singleinter;
                    doubleDwi->SetPixel(index, pix);
                    m_VolumeFractions.at(i)->SetPixel(index, singleinter/voxelVolume);
                }
                for (int i=0; i<m_NonFiberModels.size(); i++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index) + m_NonFiberModels[i]->SimulateMeasurement()*other*m_NonFiberModels[i]->GetWeight();
                    doubleDwi->SetPixel(index, pix);
                    m_VolumeFractions.at(i+m_FiberModels.size())->SetPixel(index, other/voxelVolume*m_NonFiberModels[i]->GetWeight());
                }
            }
        }
        ++it3;
        ++disp3;
    }

    // do k-space stuff
    DoubleDwiType::Pointer doubleOutImage;
    if (m_Spikes>0 || m_FrequencyMap.IsNotNull() || !m_KspaceArtifacts.empty() || m_kOffset>0 || m_SimulateRelaxation || m_SimulateEddyCurrents || m_Upsampling>1.00001)
    {
        MITK_INFO << "Adjusting complex signal";
        doubleOutImage = DoKspaceStuff(compartments);
        m_SignalScale = 1;
    }
    else
    {
        MITK_INFO << "Summing compartments";
        doubleOutImage = compartments.at(0);

        for (int i=1; i<compartments.size(); i++)
        {
            itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::Pointer adder = itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::New();
            adder->SetInput1(doubleOutImage);
            adder->SetInput2(compartments.at(i));
            adder->Update();
            doubleOutImage = adder->GetOutput();
        }
    }

    MITK_INFO << "Finalizing image";
    unsigned int window = 0;
    unsigned int min = itk::NumericTraits<unsigned int>::max();
    ImageRegionIterator<OutputImageType> it4 (outImage, outImage->GetLargestPossibleRegion());
    DoubleDwiType::PixelType signal; signal.SetSize(m_FiberModels[0]->GetNumGradients());
    boost::progress_display disp4(outImage->GetLargestPossibleRegion().GetNumberOfPixels());
    while(!it4.IsAtEnd())
    {
        ++disp4;
        typename OutputImageType::IndexType index = it4.GetIndex();
        signal = doubleOutImage->GetPixel(index)*m_SignalScale;

        if (m_NoiseModel->GetNoiseVariance() > 0)
        {
            DoubleDwiType::PixelType accu = signal; accu.Fill(0.0);
            for (int i=0; i<m_NumberOfRepetitions; i++)
            {
                DoubleDwiType::PixelType temp = signal;
                m_NoiseModel->AddNoise(temp);
                accu += temp;
            }
            signal = accu/m_NumberOfRepetitions;
        }

        for (int i=0; i<signal.Size(); i++)
        {
            if (signal[i]>0)
                signal[i] = floor(signal[i]+0.5);
            else
                signal[i] = ceil(signal[i]-0.5);

            if (!m_FiberModels.at(0)->IsBaselineIndex(i) && signal[i]>window)
                window = signal[i];
            if (!m_FiberModels.at(0)->IsBaselineIndex(i) && signal[i]<min)
                min = signal[i];
        }
        it4.Set(signal);
        ++it4;
    }
    window -= min;
    unsigned int level = window/2 + min;
    m_LevelWindow.SetLevelWindow(level, window);
    this->SetNthOutput(0, outImage);
}

template< class PixelType >
itk::Point<float, 3> TractsToDWIImageFilter< PixelType >::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

template< class PixelType >
itk::Vector<double, 3> TractsToDWIImageFilter< PixelType >::GetItkVector(double point[3])
{
    itk::Vector<double, 3> itkVector;
    itkVector[0] = point[0];
    itkVector[1] = point[1];
    itkVector[2] = point[2];
    return itkVector;
}

template< class PixelType >
vnl_vector_fixed<double, 3> TractsToDWIImageFilter< PixelType >::GetVnlVector(double point[3])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}

template< class PixelType >
vnl_vector_fixed<double, 3> TractsToDWIImageFilter< PixelType >::GetVnlVector(Vector<float,3>& vector)
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = vector[0];
    vnlVector[1] = vector[1];
    vnlVector[2] = vector[2];
    return vnlVector;
}

}
