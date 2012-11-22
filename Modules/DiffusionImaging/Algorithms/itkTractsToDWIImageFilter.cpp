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
#include <mitkGibbsRingingArtifact.h>
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkBSplineInterpolateImageFunction.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
    : m_OuputKspaceImage(false)
    , m_CircleDummy(true)
{
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);
}

TractsToDWIImageFilter::~TractsToDWIImageFilter()
{

}

std::vector< TractsToDWIImageFilter::DoubleDwiType::Pointer > TractsToDWIImageFilter::AddKspaceArtifacts( std::vector< DoubleDwiType::Pointer >& images )
{
    // create slice object
    SliceType::Pointer slice = SliceType::New();
    ImageRegion<2> region;
    region.SetSize(0, images[0]->GetLargestPossibleRegion().GetSize(0));
    region.SetSize(1, images[0]->GetLargestPossibleRegion().GetSize(1));
    slice->SetLargestPossibleRegion( region );
    slice->SetBufferedRegion( region );
    slice->SetRequestedRegion( region );
    slice->Allocate();

    int undersampling;
    for (int i=0; i<m_KspaceArtifacts.size(); i++)
        if ( dynamic_cast<mitk::GibbsRingingArtifact<double>*>(m_KspaceArtifacts.at(i)) )
            undersampling = dynamic_cast<mitk::GibbsRingingArtifact<double>*>(m_KspaceArtifacts.at(i))->GetKspaceCropping();

    if (undersampling<=1)
        undersampling = 1;

    m_UpsImage = ItkFloatImgType::New();
    m_UpsImage->SetSpacing( m_Spacing/undersampling );
    m_UpsImage->SetOrigin( m_Origin );
    m_UpsImage->SetDirection( m_DirectionMatrix );
    ImageRegion<3> region2 = m_ImageRegion;
    region2.SetSize(0, m_ImageRegion.GetSize()[0]*undersampling);
    region2.SetSize(1, m_ImageRegion.GetSize()[1]*undersampling);
    m_UpsImage->SetLargestPossibleRegion( region2 );
    m_UpsImage->SetBufferedRegion( region2 );
    m_UpsImage->SetRequestedRegion( region2 );
    m_UpsImage->Allocate();
    itk::ResampleImageFilter<SliceType, SliceType>::SizeType upsSize;
    upsSize[0] = region2.GetSize()[0];
    upsSize[1] = region2.GetSize()[1];
    mitk::Vector2D upsSpacing;
    upsSpacing[0] = m_Spacing[0]/undersampling;
    upsSpacing[1] = m_Spacing[1]/undersampling;

    MITK_INFO << "New Size: " << upsSize;

    boost::progress_display disp(images.size()*images[0]->GetVectorLength()*images[0]->GetLargestPossibleRegion().GetSize(2));
    std::vector< DoubleDwiType::Pointer > outImages;
    for (int i=0; i<images.size(); i++)
    {
        DoubleDwiType::Pointer image = images.at(i);
        DoubleDwiType::Pointer newImage = DoubleDwiType::New();
        newImage->SetSpacing( m_Spacing );
        newImage->SetOrigin( m_Origin );
        newImage->SetDirection( m_DirectionMatrix );
        newImage->SetLargestPossibleRegion( m_ImageRegion );
        newImage->SetBufferedRegion( m_ImageRegion );
        newImage->SetRequestedRegion( m_ImageRegion );
        newImage->SetVectorLength( image->GetVectorLength() );
        newImage->Allocate();

        DiffusionSignalModel<double>* signalModel;
        if (i<m_FiberModels.size())
            signalModel = m_FiberModels.at(i);
        else
            signalModel = m_NonFiberModels.at(i-m_FiberModels.size());

        for (int g=0; g<image->GetVectorLength(); g++)
            for (int z=0; z<image->GetLargestPossibleRegion().GetSize(2); z++)
            {
                ++disp;
                // extract slice from channel g
                for (int y=0; y<image->GetLargestPossibleRegion().GetSize(1); y++)
                    for (int x=0; x<image->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;
                        DoubleDwiType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        SliceType::PixelType pix2D = image->GetPixel(index3D)[g];
                        slice->SetPixel(index2D, pix2D);
                    }
//                itk::BSplineInterpolateImageFunction<SliceType,SliceType::PixelType>::Pointer interpolator = itk::BSplineInterpolateImageFunction<SliceType,SliceType::PixelType>::New();
//                itk::NearestNeighborInterpolateImageFunction<SliceType,SliceType::PixelType>::Pointer interpolator = itk::NearestNeighborInterpolateImageFunction<SliceType,SliceType::PixelType>::New();
//                itk::ResampleImageFilter<SliceType, SliceType>::Pointer resampler = itk::ResampleImageFilter<SliceType, SliceType>::New();
//                resampler->SetInterpolator(interpolator);
//                resampler->SetSize(upsSize);
//                resampler->SetInput(slice);
//                resampler->SetOutputSpacing(upsSpacing);
//                resampler->Update();
//                SliceType::Pointer reslice = resampler->GetOutput();
                SliceType::Pointer reslice = SliceType::New();
                ImageRegion<2> upsRegion;
                upsRegion.SetSize(0, upsSize[0]);
                upsRegion.SetSize(1, upsSize[1]);
                reslice->SetLargestPossibleRegion( upsRegion );
                reslice->SetBufferedRegion( upsRegion );
                reslice->SetRequestedRegion( upsRegion );
                reslice->Allocate();

                itk::ImageRegionIterator<SliceType> it(slice, region);
                while(!it.IsAtEnd())
                {
                    SliceType::IndexType idx = it.GetIndex();
                    SliceType::PixelType pix = it.Get();

                    for (int ux=0; ux<undersampling; ux++)
                        for (int uy=0; uy<undersampling; uy++)
                        {
                            SliceType::IndexType upsIdx = idx;
                            upsIdx[0] *= undersampling;
                            upsIdx[1] *= undersampling;
                            upsIdx[0] += ux;
                            upsIdx[1] += uy;
                            reslice->SetPixel(upsIdx, pix);
                        }

                    ++it;
                }

                // fourier transform slice
                itk::FFTRealToComplexConjugateImageFilter< SliceType::PixelType, 2 >::Pointer fft = itk::FFTRealToComplexConjugateImageFilter< SliceType::PixelType, 2 >::New();
                fft->SetInput(reslice);
                fft->Update();
                ComplexSliceType::Pointer fSlice = fft->GetOutput();
                fSlice = RearrangeSlice(fSlice);

                if (g==0 && i==0)
                    for (int y=0; y<upsSize[1]; y++)
                    for (int x=0; x<upsSize[0]; x++)
                    {
                        DoubleDwiType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        ItkFloatImgType::PixelType pix3D = m_UpsImage->GetPixel(index3D);

                        SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;
//                        pix3D = sqrt(fSlice->GetPixel(index2D).real()*fSlice->GetPixel(index2D).real()+fSlice->GetPixel(index2D).imag()*fSlice->GetPixel(index2D).imag());
                        pix3D = reslice->GetPixel(index2D);
                        m_UpsImage->SetPixel(index3D, pix3D);
                    }

                // add artifacts
                for (int a=0; a<m_KspaceArtifacts.size(); a++)
                {
                    m_KspaceArtifacts.at(a)->SetRelaxationT2(signalModel->GetRelaxationT2());
                    fSlice = m_KspaceArtifacts.at(a)->AddArtifact(fSlice);
                }

                SliceType::Pointer newSlice;
                if (!m_OuputKspaceImage)
                {
                    fSlice = RearrangeSlice(fSlice);
                    // inverse fourier transform slice
                    itk::FFTComplexConjugateToRealImageFilter< SliceType::PixelType, 2 >::Pointer ifft = itk::FFTComplexConjugateToRealImageFilter< SliceType::PixelType, 2 >::New();
                    ifft->SetInput(fSlice);
                    ifft->Update();
                    newSlice = ifft->GetOutput();
                }

                // put slice back into channel g
                for (int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
                    for (int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        DoubleDwiType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        DoubleDwiType::PixelType pix3D = newImage->GetPixel(index3D);

                        SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;

                        if (m_OuputKspaceImage)
                            pix3D[g] = sqrt(fSlice->GetPixel(index2D).real()*fSlice->GetPixel(index2D).real()+fSlice->GetPixel(index2D).imag()*fSlice->GetPixel(index2D).imag());
                        else
                            pix3D[g] = newSlice->GetPixel(index2D);

                        newImage->SetPixel(index3D, pix3D);
                    }
            }
        outImages.push_back(newImage);
    }
    return outImages;
}

TractsToDWIImageFilter::ComplexSliceType::Pointer TractsToDWIImageFilter::RearrangeSlice(ComplexSliceType::Pointer slice)
{
    ImageRegion<2> region = slice->GetLargestPossibleRegion();
    ComplexSliceType::Pointer rearrangedSlice = ComplexSliceType::New();
    rearrangedSlice->SetLargestPossibleRegion( region );
    rearrangedSlice->SetBufferedRegion( region );
    rearrangedSlice->SetRequestedRegion( region );
    rearrangedSlice->Allocate();

    int xHalf = region.GetSize(0)/2;
    int yHalf = region.GetSize(1)/2;

    for (int y=0; y<region.GetSize(1); y++)
        for (int x=0; x<region.GetSize(0); x++)
        {
            SliceType::IndexType idx;
            idx[0]=x; idx[1]=y;
            vcl_complex< double > pix = slice->GetPixel(idx);

            if( idx[0] <  xHalf )
                idx[0] = idx[0] + xHalf;
            else
                idx[0] = idx[0] - xHalf;

            if( idx[1] <  yHalf )
                idx[1] = idx[1] + yHalf;
            else
                idx[1] = idx[1] - yHalf;

            rearrangedSlice->SetPixel(idx, pix);
        }

    return rearrangedSlice;
}

void TractsToDWIImageFilter::GenerateData()
{
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    if (m_FiberModels.empty())
        itkExceptionMacro("No diffusion model for fiber compartments defined!");

    if (m_NonFiberModels.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined!");

    int numFibers = m_FiberBundle->GetNumFibers();
    if (numFibers<=0)
        itkExceptionMacro("Input fiber bundle contains no fibers!");

    float minSpacing = 1;
    if(m_Spacing[0]<m_Spacing[1] && m_Spacing[0]<m_Spacing[2])
        minSpacing = m_Spacing[0];
    else if (m_Spacing[1] < m_Spacing[2])
        minSpacing = m_Spacing[1];
    else
        minSpacing = m_Spacing[2];

    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    fiberBundle->DoFiberSmoothing(ceil(10.0*2.0/minSpacing));

    // initialize output dwi image
    OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( m_Origin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( m_ImageRegion );
    outImage->SetBufferedRegion( m_ImageRegion );
    outImage->SetRequestedRegion( m_ImageRegion );
    outImage->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
    outImage->Allocate();
    OutputImageType::PixelType temp;
    temp.SetSize(m_FiberModels[0]->GetNumGradients());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);
    this->SetNthOutput(0, outImage);

    // generate working double images because we work with small values (will be scaled later)
    std::vector< DoubleDwiType::Pointer > compartments;
    for (int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_Spacing );
        doubleDwi->SetOrigin( m_Origin );
        doubleDwi->SetDirection( m_DirectionMatrix );
        doubleDwi->SetLargestPossibleRegion( m_ImageRegion );
        doubleDwi->SetBufferedRegion( m_ImageRegion );
        doubleDwi->SetRequestedRegion( m_ImageRegion );
        doubleDwi->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
        doubleDwi->Allocate();
        DoubleDwiType::PixelType pix;
        pix.SetSize(m_FiberModels[0]->GetNumGradients());
        pix.Fill(0.0);
        doubleDwi->FillBuffer(pix);
        compartments.push_back(doubleDwi);
    }

    if (m_TissueMask.IsNull())
    {
        m_TissueMask = ItkUcharImgType::New();
        m_TissueMask->SetSpacing( m_Spacing );
        m_TissueMask->SetOrigin( m_Origin );
        m_TissueMask->SetDirection( m_DirectionMatrix );
        m_TissueMask->SetLargestPossibleRegion( m_ImageRegion );
        m_TissueMask->SetBufferedRegion( m_ImageRegion );
        m_TissueMask->SetRequestedRegion( m_ImageRegion );
        m_TissueMask->Allocate();
        m_TissueMask->FillBuffer(1);
    }
    else
        MITK_INFO << "Using tissue mask";

    if (m_CircleDummy)
    {
        for (int i=0; i<compartments.size(); i++)
        {
            DoubleDwiType::PixelType pix;
            pix.SetSize(m_FiberModels[0]->GetNumGradients());
            pix.Fill(2);

            DoubleDwiType::Pointer doubleDwi = compartments.at(i);
            ImageRegion<3> region = doubleDwi->GetLargestPossibleRegion();
            ImageRegionIterator<DoubleDwiType> it(doubleDwi, region);
            while(!it.IsAtEnd())
            {
                DoubleDwiType::IndexType index = it.GetIndex();
                double t = region.GetSize(0)/2;
                double d1 = index[0]-t+0.5;
                t = region.GetSize(1)/2;
                double d2 = index[1]-t+0.5;
                if (sqrt(d1*d1+d2*d2)<20)
                    it.Set(pix);
                ++it;
            }
        }
    }
    else
    {

        vtkSmartPointer<vtkPolyData> fiberPolyData = fiberBundle->GetFiberPolyData();
        vtkSmartPointer<vtkCellArray> vLines = fiberPolyData->GetLines();
        vLines->InitTraversal();

        MITK_INFO << "Generating signal of " << m_FiberModels.size() << " fiber compartments";
        double maxFiberDensity = 0;
        boost::progress_display disp(numFibers);
        for( int i=0; i<numFibers; i++ )
        {
            ++disp;
            vtkIdType   numPoints(0);
            vtkIdType*  points(NULL);
            vLines->GetNextCell ( numPoints, points );
            if (numPoints<2)
                continue;

            for( int j=0; j<numPoints; j++)
            {
                double* temp = fiberPolyData->GetPoint(points[j]);
                itk::Point<float, 3> vertex = GetItkPoint(temp);
                itk::Vector<double> v = GetItkVector(temp);

                itk::Vector<double, 3> dir(3);
                if (j<numPoints-1)
                    dir = GetItkVector(fiberPolyData->GetPoint(points[j+1]))-v;
                else
                    dir = v-GetItkVector(fiberPolyData->GetPoint(points[j-1]));

                itk::Index<3> index;
                compartments.at(0)->TransformPhysicalPointToIndex(vertex, index);

                if (!compartments.at(0)->GetLargestPossibleRegion().IsInside(index))
                    continue;

                for (int k=0; k<m_FiberModels.size(); k++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(k);
                    m_FiberModels[k]->SetFiberDirection(dir);
                    doubleDwi->SetPixel(index, doubleDwi->GetPixel(index) + m_FiberModels[k]->SimulateMeasurement());

                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                    if (pix[0]>maxFiberDensity)
                        maxFiberDensity = pix[0];
                }
            }
        }

        MITK_INFO << "Generating signal of " << m_NonFiberModels.size() << " non-fiber compartments";
        boost::progress_display disp2(m_NonFiberModels.size()*compartments.at(0)->GetLargestPossibleRegion().GetNumberOfPixels());
        for (int i=0; i<m_NonFiberModels.size(); i++)
        {
            DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
            ImageRegionIterator<DoubleDwiType> it(doubleDwi, doubleDwi->GetLargestPossibleRegion());
            while(!it.IsAtEnd())
            {
                ++disp2;
                DoubleDwiType::IndexType index = it.GetIndex();
                //            if (m_TissueMask->GetPixel(index)>0)
                doubleDwi->SetPixel(index, doubleDwi->GetPixel(index) + m_NonFiberModels[i]->SimulateMeasurement());
                ++it;
            }
        }

        MITK_INFO << "Adjusting compartment signal intensities according to volume fraction";
        ImageRegionIterator<DoubleDwiType> it3(compartments.at(0), compartments.at(0)->GetLargestPossibleRegion());
        boost::progress_display disp3(compartments.at(0)->GetLargestPossibleRegion().GetNumberOfPixels());
        while(!it3.IsAtEnd())
        {
            ++disp3;
            DoubleDwiType::IndexType index = it3.GetIndex();

            //        if (m_TissueMask->GetPixel(index)>0)
            {
                // compartment weights are calculated according to fiber density
                double w = compartments.at(0)->GetPixel(index)[0]/maxFiberDensity;

                // adjust fiber signal
                for (int i=0; i<m_FiberModels.size(); i++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(i);
                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                    if (pix[0]>0)
                        pix /= pix[0];
                    pix *= w/m_FiberModels.size();
                    doubleDwi->SetPixel(index, pix);
                }

                // adjust non-fiber signal
                for (int i=0; i<m_NonFiberModels.size(); i++)
                {
                    DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
                    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                    if (pix[0]>0)
                        pix /= pix[0];
                    pix *= (1-w)/m_NonFiberModels.size();
                    doubleDwi->SetPixel(index, pix);
                }
            }
            ++it3;
        }

    }

    if (!m_KspaceArtifacts.empty() || m_OuputKspaceImage)
    {
        if (!m_KspaceArtifacts.empty())
            MITK_INFO << "Generating k-space artifacts";
        else
            MITK_INFO << "Generating k-space image";
        compartments = AddKspaceArtifacts(compartments);
    }

    MITK_INFO << "Summing compartments and adding noise";
    ImageRegionIterator<DWIImageType> it4 (outImage, m_ImageRegion);
    DoubleDwiType::PixelType signal; signal.SetSize(m_FiberModels[0]->GetNumGradients());
    boost::progress_display disp4(m_ImageRegion.GetNumberOfPixels());
    while(!it4.IsAtEnd())
    {
        ++disp4;
        DWIImageType::IndexType index = it4.GetIndex();
        signal.Fill(0.0);

        // adjust fiber signal
        for (int i=0; i<m_FiberModels.size(); i++)
        {
            double s = 1;
            if (!m_OuputKspaceImage)
                s = m_FiberModels.at(i)->GetSignalScale();
            signal += compartments.at(i)->GetPixel(index)*s;
        }

        // adjust non-fiber signal
        for (int i=0; i<m_NonFiberModels.size(); i++)
        {
            double s = 1;
            if (!m_OuputKspaceImage)
                s = m_NonFiberModels.at(i)->GetSignalScale();
            signal += compartments.at(i)->GetPixel(index)*s;
        }

        if (!m_OuputKspaceImage)
            m_NoiseModel->AddNoise(signal);
        it4.Set(signal);
        ++it4;
    }
}

itk::Point<float, 3> TractsToDWIImageFilter::GetItkPoint(double point[3])
{
    itk::Point<float, 3> itkPoint;
    itkPoint[0] = point[0];
    itkPoint[1] = point[1];
    itkPoint[2] = point[2];
    return itkPoint;
}

itk::Vector<double, 3> TractsToDWIImageFilter::GetItkVector(double point[3])
{
    itk::Vector<double, 3> itkVector;
    itkVector[0] = point[0];
    itkVector[1] = point[1];
    itkVector[2] = point[2];
    return itkVector;
}

vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(double point[3])
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = point[0];
    vnlVector[1] = point[1];
    vnlVector[2] = point[2];
    return vnlVector;
}


vnl_vector_fixed<double, 3> TractsToDWIImageFilter::GetVnlVector(Vector<float,3>& vector)
{
    vnl_vector_fixed<double, 3> vnlVector;
    vnlVector[0] = vector[0];
    vnlVector[1] = vector[1];
    vnlVector[2] = vector[2];
    return vnlVector;
}

}
