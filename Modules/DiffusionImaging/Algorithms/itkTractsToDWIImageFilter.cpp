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
#include <vnl/algo/vnl_fft_2d.h>
#include <vcl_complex.h>

namespace itk
{
TractsToDWIImageFilter::TractsToDWIImageFilter()
    : m_Undersampling(11)
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

        for (int g=0; g<image->GetVectorLength(); g++)
            for (int z=0; z<image->GetLargestPossibleRegion().GetSize(2); z++)
            {
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
                // fourier transform slice
                itk::FFTRealToComplexConjugateImageFilter< SliceType::PixelType, 2 >::Pointer fft = itk::FFTRealToComplexConjugateImageFilter< SliceType::PixelType, 2 >::New();
                fft->SetInput(slice);
                fft->Update();
                ComplexSliceType::Pointer fSlice = fft->GetOutput();

                // crop transformed slice
                fSlice = CropSlice(fSlice, m_ImageRegion.GetSize()[0]*(m_Undersampling-1)/2, m_ImageRegion.GetSize()[1]*(m_Undersampling-1)/2);

                // inverse fourier transform slice
                itk::FFTComplexConjugateToRealImageFilter< SliceType::PixelType, 2 >::Pointer ifft = itk::FFTComplexConjugateToRealImageFilter< SliceType::PixelType, 2 >::New();
                ifft->SetInput(fSlice);
                ifft->Update();
                SliceType::Pointer newSlice = ifft->GetOutput();

                // put slice back into channel g
                for (int y=0; y<m_ImageRegion.GetSize(1); y++)
                    for (int x=0; x<m_ImageRegion.GetSize(0); x++)
                    {
                        DoubleDwiType::IndexType index3D;
                        index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        DoubleDwiType::PixelType pix3D = newImage->GetPixel(index3D);

                        SliceType::IndexType index2D;
                        index2D[0]=x; index2D[1]=y;
                        pix3D[g] = newSlice->GetPixel(index2D);

                        newImage->SetPixel(index3D, pix3D);
                    }
            }
        outImages.push_back(newImage);
    }
    return outImages;
}

TractsToDWIImageFilter::ComplexSliceType::Pointer TractsToDWIImageFilter::CropSlice(ComplexSliceType::Pointer image, int x, int y)
{
    ImageRegion<2> region = image->GetLargestPossibleRegion();
    ImageRegion<2> newRegion;
    newRegion.SetSize(0, region.GetSize(0)-2*x);
    newRegion.SetSize(1, region.GetSize(1)-2*y);

    ComplexSliceType::Pointer newImage = ComplexSliceType::New();
    newImage->SetLargestPossibleRegion( newRegion );
    newImage->SetBufferedRegion( newRegion );
    newImage->SetRequestedRegion( newRegion );
    newImage->Allocate();
    newImage->FillBuffer(0);

    ImageRegionIterator<ComplexSliceType> it(image, region);
    while(!it.IsAtEnd())
    {
        ComplexSliceType::IndexType index = it.GetIndex();
        index[0] -= x; index[1] -= y;
        if (newRegion.IsInside(index))
            newImage->SetPixel(index, image->GetPixel(it.GetIndex()));

        ++it;
    }
    return newImage;
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

    if (m_Undersampling%2!=1)
        m_Undersampling += 1;

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

    // generate working double image because we work with small values (will be scaled later)
    std::vector< DoubleDwiType::Pointer > compartments;
    for (int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        ImageRegion<3> upsampledRegion;
        upsampledRegion.SetSize(0, m_ImageRegion.GetSize(0)*m_Undersampling);
        upsampledRegion.SetSize(1, m_ImageRegion.GetSize(1)*m_Undersampling);
        upsampledRegion.SetSize(2, m_ImageRegion.GetSize(2));

        mitk::Vector3D spacing;
        spacing[0] = m_Spacing[0]/m_Undersampling;
        spacing[1] = m_Spacing[1]/m_Undersampling;
        spacing[2] = m_Spacing[2];

        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( spacing );
        doubleDwi->SetOrigin( m_Origin );
        doubleDwi->SetDirection( m_DirectionMatrix );
        doubleDwi->SetLargestPossibleRegion( upsampledRegion );
        doubleDwi->SetBufferedRegion( upsampledRegion );
        doubleDwi->SetRequestedRegion( upsampledRegion );
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
            outImage->TransformPhysicalPointToIndex(vertex, index);

            if (!m_ImageRegion.IsInside(index))
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
    boost::progress_display disp2(m_ImageRegion.GetNumberOfPixels());
    for (int i=0; i<m_NonFiberModels.size(); i++)
    {
        ++disp2;
        DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
        ImageRegionIterator<DoubleDwiType> it(doubleDwi, m_ImageRegion);
        while(!it.IsAtEnd())
        {
            DoubleDwiType::IndexType index = it.GetIndex();
            doubleDwi->SetPixel(index, doubleDwi->GetPixel(index) + m_NonFiberModels[i]->SimulateMeasurement());
            ++it;
        }
    }

    MITK_INFO << "Adding k-space artifacts";
    compartments = AddKspaceArtifacts(compartments);

    MITK_INFO << "Summing signals and adding noise";
    ImageRegionIterator<DWIImageType> it (outImage, m_ImageRegion);
    DoubleDwiType::PixelType signal;
    signal.SetSize(m_FiberModels[0]->GetNumGradients());

    boost::progress_display disp3(m_ImageRegion.GetNumberOfPixels());
    while(!it.IsAtEnd())
    {
        ++disp3;
        DWIImageType::IndexType index = it.GetIndex();
        signal.Fill(0.0);

        if (m_TissueMask->GetPixel(index)>0)
        {
            // compartment weights are calculated according to fiber density
            double w = compartments.at(0)->GetPixel(index)[0]/maxFiberDensity;

            // add fiber signal
            for (int i=0; i<m_FiberModels.size(); i++)
            {
                DoubleDwiType::Pointer doubleDwi = compartments.at(i);
                DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                if (pix[0]>0)
                    pix /= pix[0];
                pix *= m_FiberModels.at(i)->GetSignalScale()*w/m_FiberModels.size();
                signal += pix;
            }

            // add on-fiber signal
            for (int i=0; i<m_NonFiberModels.size(); i++)
            {
                DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
                DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                if (pix[0]>0)
                    pix /= pix[0];
                pix *= m_NonFiberModels.at(i)->GetSignalScale()*(1-w)/m_NonFiberModels.size();
                signal += pix;
            }
        }

        m_NoiseModel->AddNoise(signal); // add noise
        it.Set(signal);
        ++it;
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
