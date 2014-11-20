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
#include <itkConstantPadImageFilter.h>
#include <itkCropImageFilter.h>
#include <mitkAstroStickModel.h>
#include <vtkTransform.h>
#include <iostream>
#include <fstream>
#include <itkImageDuplicator.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkTractDensityImageFilter.h>
#include <boost/lexical_cast.hpp>
#include <itkTractsToVectorImageFilter.h>

namespace itk
{

template< class PixelType >
TractsToDWIImageFilter< PixelType >::TractsToDWIImageFilter()
    : m_FiberBundle(NULL)
    , m_StatusText("")
    , m_UseConstantRandSeed(false)
    , m_RandGen(itk::Statistics::MersenneTwisterRandomVariateGenerator::New())
{
    m_RandGen->SetSeed();
}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::~TractsToDWIImageFilter()
{

}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::DoubleDwiType::Pointer TractsToDWIImageFilter< PixelType >::DoKspaceStuff( std::vector< DoubleDwiType::Pointer >& images )
{
    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    // create slice object
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
    sliceRegion.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
    Vector< double, 2 > sliceSpacing;
    sliceSpacing[0] = m_UpsampledSpacing[0];
    sliceSpacing[1] = m_UpsampledSpacing[1];

    // frequency map slice
    SliceType::Pointer fMapSlice = NULL;
    if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
    {
        fMapSlice = SliceType::New();
        ImageRegion<2> region;
        region.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
        region.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
        fMapSlice->SetLargestPossibleRegion( region );
        fMapSlice->SetBufferedRegion( region );
        fMapSlice->SetRequestedRegion( region );
        fMapSlice->Allocate();
        fMapSlice->FillBuffer(0.0);
    }

    DoubleDwiType::Pointer newImage = DoubleDwiType::New();
    newImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    newImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
    newImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    newImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_ImageRegion );
    newImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_ImageRegion );
    newImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_ImageRegion );
    newImage->SetVectorLength( images.at(0)->GetVectorLength() );
    newImage->Allocate();

    std::vector< unsigned int > spikeVolume;
    for (unsigned int i=0; i<m_Parameters.m_SignalGen.m_Spikes; i++)
        spikeVolume.push_back(m_RandGen->GetIntegerVariate()%images.at(0)->GetVectorLength());
    std::sort (spikeVolume.begin(), spikeVolume.end());
    std::reverse (spikeVolume.begin(), spikeVolume.end());

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    unsigned long lastTick = 0;

    boost::progress_display disp(2*images.at(0)->GetVectorLength()*images.at(0)->GetLargestPossibleRegion().GetSize(2));
    for (unsigned int g=0; g<images.at(0)->GetVectorLength(); g++)
    {
        std::vector< unsigned int > spikeSlice;
        while (!spikeVolume.empty() && spikeVolume.back()==g)
        {
            spikeSlice.push_back(m_RandGen->GetIntegerVariate()%images.at(0)->GetLargestPossibleRegion().GetSize(2));
            spikeVolume.pop_back();
        }
        std::sort (spikeSlice.begin(), spikeSlice.end());
        std::reverse (spikeSlice.begin(), spikeSlice.end());

        for (unsigned int z=0; z<images.at(0)->GetLargestPossibleRegion().GetSize(2); z++)
        {
            std::vector< SliceType::Pointer > compartmentSlices;
            std::vector< double > t2Vector;

            for (unsigned int i=0; i<images.size(); i++)
            {
                DiffusionSignalModel<double>* signalModel;
                if (i<numFiberCompartments)
                    signalModel = m_Parameters.m_FiberModelList.at(i);
                else
                    signalModel = m_Parameters.m_NonFiberModelList.at(i-numFiberCompartments);

                SliceType::Pointer slice = SliceType::New();
                slice->SetLargestPossibleRegion( sliceRegion );
                slice->SetBufferedRegion( sliceRegion );
                slice->SetRequestedRegion( sliceRegion );
                slice->SetSpacing(sliceSpacing);
                slice->Allocate();
                slice->FillBuffer(0.0);

                // extract slice from channel g
                for (unsigned int y=0; y<images.at(0)->GetLargestPossibleRegion().GetSize(1); y++)
                    for (unsigned int x=0; x<images.at(0)->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        SliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;
                        DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        slice->SetPixel(index2D, images.at(i)->GetPixel(index3D)[g]);

                        if (fMapSlice.IsNotNull() && i==0)
                            fMapSlice->SetPixel(index2D, m_Parameters.m_SignalGen.m_FrequencyMap->GetPixel(index3D));
                    }

                compartmentSlices.push_back(slice);
                t2Vector.push_back(signalModel->GetT2());
            }

            if (this->GetAbortGenerateData())
                return NULL;

            // create k-sapce (inverse fourier transform slices)
            itk::Size<2> outSize; outSize.SetElement(0, m_Parameters.m_SignalGen.m_ImageRegion.GetSize(0)); outSize.SetElement(1, m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1));
            itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
            idft->SetCompartmentImages(compartmentSlices);
            idft->SetT2(t2Vector);
            idft->SetUseConstantRandSeed(m_UseConstantRandSeed);
            idft->SetParameters(m_Parameters);
            idft->SetZ((double)z-(double)images.at(0)->GetLargestPossibleRegion().GetSize(2)/2.0);
            idft->SetDiffusionGradientDirection(m_Parameters.m_SignalGen.GetGradientDirection(g));
            idft->SetFrequencyMapSlice(fMapSlice);
            idft->SetOutSize(outSize);
            int numSpikes = 0;
            while (!spikeSlice.empty() && spikeSlice.back()==z)
            {
                numSpikes++;
                spikeSlice.pop_back();
            }
            idft->SetSpikesPerSlice(numSpikes);
            idft->Update();

            ComplexSliceType::Pointer fSlice;
            fSlice = idft->GetOutput();

            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned  long tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;

            // fourier transform slice
            SliceType::Pointer newSlice;
            itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
            dft->SetInput(fSlice);
            dft->Update();
            newSlice = dft->GetOutput();

            // put slice back into channel g
            for (unsigned int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
                for (unsigned int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
                {
                    DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
                    SliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;

                    DoubleDwiType::PixelType pix3D = newImage->GetPixel(index3D);
                    pix3D[g] = newSlice->GetPixel(index2D);
                    newImage->SetPixel(index3D, pix3D);
                }
            ++disp;
            newTick = 50*disp.count()/disp.expected_count();
            for (unsigned long tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;
        }
    }
    m_StatusText += "\n\n";
    return newImage;
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::GenerateData()
{
    m_TimeProbe.Start();
    m_StatusText = "Starting simulation\n";

    // check input data
    if (m_FiberBundle.IsNull())
        itkExceptionMacro("Input fiber bundle is NULL!");

    if (m_Parameters.m_FiberModelList.empty())
        itkExceptionMacro("No diffusion model for fiber compartments defined!");

    if (m_Parameters.m_NonFiberModelList.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined!");

    int baselineIndex = m_Parameters.m_SignalGen.GetFirstBaselineIndex();
    if (baselineIndex<0)
        itkExceptionMacro("No baseline index found!");

    if (!m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition)
        m_Parameters.m_SignalGen.m_DoAddGibbsRinging = false;

    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        m_RandGen->SetSeed(0);
    else
        m_RandGen->SetSeed();

    // initialize output dwi image
    ImageRegion<3> croppedRegion = m_Parameters.m_SignalGen.m_ImageRegion; croppedRegion.SetSize(1, croppedRegion.GetSize(1)*m_Parameters.m_SignalGen.m_CroppingFactor);
    itk::Point<double,3> shiftedOrigin = m_Parameters.m_SignalGen.m_ImageOrigin; shiftedOrigin[1] += (m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1)-croppedRegion.GetSize(1))*m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    outImage->SetOrigin( shiftedOrigin );
    outImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    outImage->SetLargestPossibleRegion( croppedRegion );
    outImage->SetBufferedRegion( croppedRegion );
    outImage->SetRequestedRegion( croppedRegion );
    outImage->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
    outImage->Allocate();
    typename OutputImageType::PixelType temp;
    temp.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);

    // ADJUST GEOMETRY FOR FURTHER PROCESSING
    // is input slize size a power of two?
    unsigned int x=m_Parameters.m_SignalGen.m_ImageRegion.GetSize(0); unsigned int y=m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1);
    ItkDoubleImgType::SizeType pad; pad[0]=x%2; pad[1]=y%2; pad[2]=0;
    m_Parameters.m_SignalGen.m_ImageRegion.SetSize(0, x+pad[0]);
    m_Parameters.m_SignalGen.m_ImageRegion.SetSize(1, y+pad[1]);
    if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull() && (pad[0]>0 || pad[1]>0))
    {
        itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer zeroPadder = itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
        zeroPadder->SetInput(m_Parameters.m_SignalGen.m_FrequencyMap);
        zeroPadder->SetConstant(0);
        zeroPadder->SetPadUpperBound(pad);
        zeroPadder->Update();
        m_Parameters.m_SignalGen.m_FrequencyMap = zeroPadder->GetOutput();
    }
    if (m_Parameters.m_SignalGen.m_MaskImage.IsNotNull() && (pad[0]>0 || pad[1]>0))
    {
        itk::ConstantPadImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer zeroPadder = itk::ConstantPadImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
        zeroPadder->SetInput(m_Parameters.m_SignalGen.m_MaskImage);
        zeroPadder->SetConstant(0);
        zeroPadder->SetPadUpperBound(pad);
        zeroPadder->Update();
        m_Parameters.m_SignalGen.m_MaskImage = zeroPadder->GetOutput();
    }

    // Apply in-plane upsampling for Gibbs ringing artifact
    double upsampling = 1;
    if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
        upsampling = 2;
    m_UpsampledSpacing = m_Parameters.m_SignalGen.m_ImageSpacing;
    m_UpsampledSpacing[0] /= upsampling;
    m_UpsampledSpacing[1] /= upsampling;
    m_UpsampledImageRegion = m_Parameters.m_SignalGen.m_ImageRegion;
    m_UpsampledImageRegion.SetSize(0, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[0]*upsampling);
    m_UpsampledImageRegion.SetSize(1, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[1]*upsampling);
    m_UpsampledOrigin = m_Parameters.m_SignalGen.m_ImageOrigin;
    m_UpsampledOrigin[0] -= m_Parameters.m_SignalGen.m_ImageSpacing[0]/2; m_UpsampledOrigin[0] += m_UpsampledSpacing[0]/2;
    m_UpsampledOrigin[1] -= m_Parameters.m_SignalGen.m_ImageSpacing[1]/2; m_UpsampledOrigin[1] += m_UpsampledSpacing[1]/2;
    m_UpsampledOrigin[2] -= m_Parameters.m_SignalGen.m_ImageSpacing[2]/2; m_UpsampledOrigin[2] += m_UpsampledSpacing[2]/2;

    // generate double images to store the individual compartment signals
    m_CompartmentImages.clear();
    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();

    for (int i=0; i<numFiberCompartments+numNonFiberCompartments; i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_UpsampledSpacing );
        doubleDwi->SetOrigin( m_UpsampledOrigin );
        doubleDwi->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        doubleDwi->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleDwi->SetBufferedRegion( m_UpsampledImageRegion );
        doubleDwi->SetRequestedRegion( m_UpsampledImageRegion );
        doubleDwi->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
        doubleDwi->Allocate();
        DoubleDwiType::PixelType pix;
        pix.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
        pix.Fill(0.0);
        doubleDwi->FillBuffer(pix);
        m_CompartmentImages.push_back(doubleDwi);
    }

    // initialize output volume fraction images
    m_VolumeFractions.clear();
    for (int i=0; i<numFiberCompartments+numNonFiberCompartments; i++)
    {
        ItkDoubleImgType::Pointer doubleImg = ItkDoubleImgType::New();
        doubleImg->SetSpacing( m_UpsampledSpacing );
        doubleImg->SetOrigin( m_UpsampledOrigin );
        doubleImg->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        doubleImg->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleImg->SetBufferedRegion( m_UpsampledImageRegion );
        doubleImg->SetRequestedRegion( m_UpsampledImageRegion );
        doubleImg->Allocate();
        doubleImg->FillBuffer(0);
        m_VolumeFractions.push_back(doubleImg);
    }

    // get volume fraction images
    ItkDoubleImgType::Pointer sumImage = ItkDoubleImgType::New();
    bool foundVolumeFractionImage = false;

    for (int i=0; i<numNonFiberCompartments; i++)  // look if a volume fraction image is set
    {
        if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage().IsNotNull())
        {
            foundVolumeFractionImage = true;

            itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer zeroPadder = itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
            zeroPadder->SetInput(m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage());
            zeroPadder->SetConstant(0);
            zeroPadder->SetPadUpperBound(pad);
            zeroPadder->Update();
            m_Parameters.m_NonFiberModelList[i]->SetVolumeFractionImage(zeroPadder->GetOutput());

            sumImage->SetSpacing( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetSpacing() );
            sumImage->SetOrigin( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetOrigin() );
            sumImage->SetDirection( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetDirection() );
            sumImage->SetLargestPossibleRegion( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion() );
            sumImage->SetBufferedRegion( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion() );
            sumImage->SetRequestedRegion( m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion() );
            sumImage->Allocate();
            sumImage->FillBuffer(0);
            break;
        }
    }
    if (!foundVolumeFractionImage)
    {
        sumImage->SetSpacing( m_UpsampledSpacing );
        sumImage->SetOrigin( m_UpsampledOrigin );
        sumImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        sumImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        sumImage->SetBufferedRegion( m_UpsampledImageRegion );
        sumImage->SetRequestedRegion( m_UpsampledImageRegion );
        sumImage->Allocate();
        sumImage->FillBuffer(0.0);
    }
    for (int i=0; i<numNonFiberCompartments; i++)
    {
        if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage().IsNull())
        {
            ItkDoubleImgType::Pointer doubleImg = ItkDoubleImgType::New();
            doubleImg->SetSpacing( sumImage->GetSpacing() );
            doubleImg->SetOrigin( sumImage->GetOrigin() );
            doubleImg->SetDirection( sumImage->GetDirection() );
            doubleImg->SetLargestPossibleRegion( sumImage->GetLargestPossibleRegion() );
            doubleImg->SetBufferedRegion( sumImage->GetLargestPossibleRegion() );
            doubleImg->SetRequestedRegion( sumImage->GetLargestPossibleRegion() );
            doubleImg->Allocate();
            doubleImg->FillBuffer(1.0/numNonFiberCompartments);
            m_Parameters.m_NonFiberModelList[i]->SetVolumeFractionImage(doubleImg);
        }
        ImageRegionIterator<ItkDoubleImgType> it(m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage(), m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            sumImage->SetPixel(it.GetIndex(), sumImage->GetPixel(it.GetIndex())+it.Get());
            ++it;
        }
    }
    for (int i=0; i<numNonFiberCompartments; i++)
    {
        ImageRegionIterator<ItkDoubleImgType> it(m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage(), m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            if (sumImage->GetPixel(it.GetIndex())>0)
                it.Set(it.Get()/sumImage->GetPixel(it.GetIndex()));
            ++it;
        }
    }

    // resample mask image and frequency map to fit upsampled geometry
    if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
    {
        if (m_Parameters.m_SignalGen.m_MaskImage.IsNotNull())
        {
            // rescale mask image (otherwise there are problems with the resampling)
            itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::Pointer rescaler = itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::New();
            rescaler->SetInput(0,m_Parameters.m_SignalGen.m_MaskImage);
            rescaler->SetOutputMaximum(100);
            rescaler->SetOutputMinimum(0);
            rescaler->Update();

            // resample mask image
            itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer resampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
            resampler->SetInput(rescaler->GetOutput());
            resampler->SetOutputParametersFromImage(m_Parameters.m_SignalGen.m_MaskImage);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_SignalGen.m_MaskImage = resampler->GetOutput();

            itk::ImageFileWriter<ItkUcharImgType>::Pointer w = itk::ImageFileWriter<ItkUcharImgType>::New();
            w->SetFileName("/local/mask_ups.nrrd");
            w->SetInput(m_Parameters.m_SignalGen.m_MaskImage);
            w->Update();
        }
        // resample frequency map
        if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
        {
            itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer resampler = itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
            resampler->SetInput(m_Parameters.m_SignalGen.m_FrequencyMap);
            resampler->SetOutputParametersFromImage(m_Parameters.m_SignalGen.m_FrequencyMap);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_SignalGen.m_FrequencyMap = resampler->GetOutput();
        }
    }

    // no input tissue mask is set -> create default
    m_MaskImageSet = true;
    if (m_Parameters.m_SignalGen.m_MaskImage.IsNull())
    {
        m_StatusText += "No tissue mask set\n";
        MITK_INFO << "No tissue mask set";
        m_Parameters.m_SignalGen.m_MaskImage = ItkUcharImgType::New();
        m_Parameters.m_SignalGen.m_MaskImage->SetSpacing( m_UpsampledSpacing );
        m_Parameters.m_SignalGen.m_MaskImage->SetOrigin( m_UpsampledOrigin );
        m_Parameters.m_SignalGen.m_MaskImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        m_Parameters.m_SignalGen.m_MaskImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->SetBufferedRegion( m_UpsampledImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->SetRequestedRegion( m_UpsampledImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->Allocate();
        m_Parameters.m_SignalGen.m_MaskImage->FillBuffer(1);
        m_MaskImageSet = false;
    }
    else
    {
        m_StatusText += "Using tissue mask\n";
        MITK_INFO << "Using tissue mask";
    }

    m_Parameters.m_SignalGen.m_ImageRegion = croppedRegion;
    x=m_Parameters.m_SignalGen.m_ImageRegion.GetSize(0); y=m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1);
    if ( x%2 == 1 )
        m_Parameters.m_SignalGen.m_ImageRegion.SetSize(0, x+1);
    if ( y%2 == 1 )
        m_Parameters.m_SignalGen.m_ImageRegion.SetSize(1, y+1);

    // resample fiber bundle for sufficient voxel coverage
    m_StatusText += "\n"+this->GetTime()+" > Resampling fibers ...\n";
    double segmentVolume = 0.0001;
    float minSpacing = 1;
    if(m_UpsampledSpacing[0]<m_UpsampledSpacing[1] && m_UpsampledSpacing[0]<m_UpsampledSpacing[2])
        minSpacing = m_UpsampledSpacing[0];
    else if (m_UpsampledSpacing[1] < m_UpsampledSpacing[2])
        minSpacing = m_UpsampledSpacing[1];
    else
        minSpacing = m_UpsampledSpacing[2];
    m_FiberBundleWorkingCopy = m_FiberBundle->GetDeepCopy();
    double volumeAccuracy = 10;
    m_FiberBundleWorkingCopy->ResampleSpline(minSpacing/volumeAccuracy);
    double mmRadius = m_Parameters.m_SignalGen.m_AxonRadius/1000;
    if (mmRadius>0)
        segmentVolume = M_PI*mmRadius*mmRadius*minSpacing/volumeAccuracy;

    double maxVolume = 0;
    m_VoxelVolume = m_UpsampledSpacing[0]*m_UpsampledSpacing[1]*m_UpsampledSpacing[2];

    if (m_Parameters.m_SignalGen.m_DoAddMotion)
    {
        std::string fileName = "fiberfox_motion_0.log";
        std::string filePath = mitk::IOUtil::GetTempPath();
        if (m_Parameters.m_Misc.m_OutputPath.size()>0)
            filePath = m_Parameters.m_Misc.m_OutputPath;

        int c = 1;

        while (itksys::SystemTools::FileExists((filePath+fileName).c_str()))
        {
            fileName = "fiberfox_motion_";
            fileName += boost::lexical_cast<std::string>(c);
            fileName += ".log";
            c++;
        }

        m_Logfile.open((filePath+fileName).c_str());
        m_Logfile << "0 rotation: 0,0,0; translation: 0,0,0\n";

        if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
        {
            m_StatusText += "Adding random motion artifacts:\n";
            m_StatusText += "Maximum rotation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Rotation) + "°\n";
            m_StatusText += "Maximum translation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Translation) + "mm\n";
        }
        else
        {
            m_StatusText += "Adding linear motion artifacts:\n";
            m_StatusText += "Maximum rotation: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Rotation) + "°\n";
            m_StatusText += "Maximum translation: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Translation) + "mm\n";
        }
        m_StatusText += "Motion logfile: " + (filePath+fileName) + "\n";
        MITK_INFO << "Adding motion artifacts";
        MITK_INFO << "Maximum rotation: " << m_Parameters.m_SignalGen.m_Rotation;
        MITK_INFO << "Maxmimum translation: " << m_Parameters.m_SignalGen.m_Translation;
    }
    maxVolume = 0;

    m_StatusText += "\n"+this->GetTime()+" > Generating " + boost::lexical_cast<std::string>(numFiberCompartments+numNonFiberCompartments) + "-compartment diffusion-weighted signal.\n";
    MITK_INFO << "Generating " << numFiberCompartments+numNonFiberCompartments << "-compartment diffusion-weighted signal.";
    int numFibers = m_FiberBundle->GetNumFibers();
    boost::progress_display disp(numFibers*m_Parameters.m_SignalGen.GetNumVolumes());


    // get transform for motion artifacts
    m_FiberBundleTransformed = m_FiberBundleWorkingCopy;
    m_Rotation = m_Parameters.m_SignalGen.m_Rotation/m_Parameters.m_SignalGen.GetNumVolumes();
    m_Translation = m_Parameters.m_SignalGen.m_Translation/m_Parameters.m_SignalGen.GetNumVolumes();

    // creat image to hold transformed mask (motion artifact)
    m_MaskImage = ItkUcharImgType::New();
    itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
    duplicator->SetInputImage(m_Parameters.m_SignalGen.m_MaskImage);
    duplicator->Update();
    m_MaskImage = duplicator->GetOutput();

    // second upsampling needed for motion artifacts
    ImageRegion<3>      upsampledImageRegion = m_UpsampledImageRegion;
    DoubleVectorType    upsampledSpacing = m_UpsampledSpacing;
    upsampledSpacing[0] /= 4;
    upsampledSpacing[1] /= 4;
    upsampledSpacing[2] /= 4;
    upsampledImageRegion.SetSize(0, m_UpsampledImageRegion.GetSize()[0]*4);
    upsampledImageRegion.SetSize(1, m_UpsampledImageRegion.GetSize()[1]*4);
    upsampledImageRegion.SetSize(2, m_UpsampledImageRegion.GetSize()[2]*4);
    itk::Point<double,3> upsampledOrigin = m_UpsampledOrigin;
    upsampledOrigin[0] -= m_UpsampledSpacing[0]/2; upsampledOrigin[0] += upsampledSpacing[0]/2;
    upsampledOrigin[1] -= m_UpsampledSpacing[1]/2; upsampledOrigin[1] += upsampledSpacing[1]/2;
    upsampledOrigin[2] -= m_UpsampledSpacing[2]/2; upsampledOrigin[2] += upsampledSpacing[2]/2;
    m_UpsampledMaskImage = ItkUcharImgType::New();
    itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer upsampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
    upsampler->SetInput(m_Parameters.m_SignalGen.m_MaskImage);
    upsampler->SetOutputParametersFromImage(m_Parameters.m_SignalGen.m_MaskImage);
    upsampler->SetSize(upsampledImageRegion.GetSize());
    upsampler->SetOutputSpacing(upsampledSpacing);
    upsampler->SetOutputOrigin(upsampledOrigin);
    itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::Pointer nn_interpolator
            = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
    upsampler->SetInterpolator(nn_interpolator);
    upsampler->Update();
    m_UpsampledMaskImage = upsampler->GetOutput();

    unsigned long lastTick = 0;
    int signalModelSeed = m_RandGen->GetIntegerVariate();
    switch (m_Parameters.m_SignalGen.m_DiffusionDirectionMode)
    {
    case(SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS):   // use fiber tangent directions to determine diffusion direction
    {
        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";

        for (unsigned int g=0; g<m_Parameters.m_SignalGen.GetNumVolumes(); g++)
        {
            // Set signal model random generator seeds to get same configuration in each voxel
            for (int i=0; i<m_Parameters.m_FiberModelList.size(); i++)
                m_Parameters.m_FiberModelList.at(i)->SetSeed(signalModelSeed);
            for (int i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
                m_Parameters.m_NonFiberModelList.at(i)->SetSeed(signalModelSeed);

            ItkDoubleImgType::Pointer intraAxonalVolumeImage = ItkDoubleImgType::New();
            intraAxonalVolumeImage->SetSpacing( m_UpsampledSpacing );
            intraAxonalVolumeImage->SetOrigin( m_UpsampledOrigin );
            intraAxonalVolumeImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
            intraAxonalVolumeImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
            intraAxonalVolumeImage->SetBufferedRegion( m_UpsampledImageRegion );
            intraAxonalVolumeImage->SetRequestedRegion( m_UpsampledImageRegion );
            intraAxonalVolumeImage->Allocate();
            intraAxonalVolumeImage->FillBuffer(0);

            vtkPolyData* fiberPolyData = m_FiberBundleTransformed->GetFiberPolyData();

            // generate fiber signal (if there are any fiber models present)
            if (!m_Parameters.m_FiberModelList.empty())
                for( int i=0; i<numFibers; i++ )
                {
                    vtkCell* cell = fiberPolyData->GetCell(i);
                    int numPoints = cell->GetNumberOfPoints();
                    vtkPoints* points = cell->GetPoints();

                    if (numPoints<2)
                        continue;

                    for( int j=0; j<numPoints; j++)
                    {
                        if (this->GetAbortGenerateData())
                        {
                            m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
                            return;
                        }

                        double* temp = points->GetPoint(j);
                        itk::Point<float, 3> vertex = GetItkPoint(temp);
                        itk::Vector<double> v = GetItkVector(temp);

                        itk::Vector<double, 3> dir(3);
                        if (j<numPoints-1)
                            dir = GetItkVector(points->GetPoint(j+1))-v;
                        else
                            dir = v-GetItkVector(points->GetPoint(j-1));

                        if (dir.GetSquaredNorm()<0.0001 || dir[0]!=dir[0] || dir[1]!=dir[1] || dir[2]!=dir[2])
                            continue;

                        itk::Index<3> idx;
                        itk::ContinuousIndex<float, 3> contIndex;
                        m_MaskImage->TransformPhysicalPointToIndex(vertex, idx);
                        m_MaskImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

                        if (!m_MaskImage->GetLargestPossibleRegion().IsInside(idx) || m_MaskImage->GetPixel(idx)<=0)
                            continue;

                        // generate signal for each fiber compartment
                        for (int k=0; k<numFiberCompartments; k++)
                        {
                            m_Parameters.m_FiberModelList[k]->SetFiberDirection(dir);
                            DoubleDwiType::PixelType pix = m_CompartmentImages.at(k)->GetPixel(idx);
                            pix[g] += segmentVolume*m_Parameters.m_FiberModelList[k]->SimulateMeasurement(g);

                            m_CompartmentImages.at(k)->SetPixel(idx, pix);
                        }

                        // update fiber volume image
                        double vol = intraAxonalVolumeImage->GetPixel(idx) + segmentVolume;
                        intraAxonalVolumeImage->SetPixel(idx, vol);
                        if (g==0 && vol>maxVolume)
                            maxVolume = vol;
                    }

                    // progress report
                    ++disp;
                    unsigned long newTick = 50*disp.count()/disp.expected_count();
                    for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                        m_StatusText += "*";
                    lastTick = newTick;
                }

            // generate non-fiber signal
            ImageRegionIterator<ItkUcharImgType> it3(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
            double fact = 1;
            if (m_Parameters.m_SignalGen.m_AxonRadius<0.0001 || maxVolume>m_VoxelVolume)
                fact = m_VoxelVolume/maxVolume;
            while(!it3.IsAtEnd())
            {
                if (it3.Get()>0)
                {
                    DoubleDwiType::IndexType index = it3.GetIndex();

                    // adjust intra-axonal signal to abtain an only-fiber voxel
                    if (fabs(fact-1.0)>0.0001)
                        for (int i=0; i<numFiberCompartments; i++)
                        {
                            DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                            pix[g] *= fact;
                            m_CompartmentImages.at(i)->SetPixel(index, pix);
                        }

                    // simulate other compartments
                    SimulateNonFiberSignal(index, intraAxonalVolumeImage->GetPixel(index)*fact, g);
                }
                ++it3;
            }

            // move fibers
            SimulateMotion(g);
        }
        break;
    }
    case (SignalGenerationParameters::MAIN_FIBER_DIRECTIONS): // use main fiber directions to determine voxel-wise diffusion directions
    {
        typedef itk::Image< itk::Vector< float, 3>, 3 >                                 ItkDirectionImage3DType;
        typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >  ItkDirectionImageContainerType;

        // calculate main fiber directions
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(m_FiberBundleTransformed);
        fOdfFilter->SetMaskImage(m_MaskImage);
        fOdfFilter->SetAngularThreshold(cos(m_Parameters.m_SignalGen.m_FiberSeparationThreshold*M_PI/180.0));
        fOdfFilter->SetNormalizeVectors(false);
        fOdfFilter->SetUseWorkingCopy(true);
        fOdfFilter->SetSizeThreshold(0);
        fOdfFilter->SetMaxNumDirections(3);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        // allocate image storing intra-axonal volume fraction information
        ItkDoubleImgType::Pointer intraAxonalVolumeImage = ItkDoubleImgType::New();
        intraAxonalVolumeImage->SetSpacing( m_UpsampledSpacing );
        intraAxonalVolumeImage->SetOrigin( m_UpsampledOrigin );
        intraAxonalVolumeImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        intraAxonalVolumeImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->SetBufferedRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->SetRequestedRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->Allocate();
        intraAxonalVolumeImage->FillBuffer(0);

        // determine intra-axonal volume fraction using the tract density
        itk::TractDensityImageFilter< ItkDoubleImgType >::Pointer tdiFilter = itk::TractDensityImageFilter< ItkDoubleImgType >::New();
        tdiFilter->SetFiberBundle(m_FiberBundleTransformed);
        tdiFilter->SetBinaryOutput(false);
        tdiFilter->SetOutputAbsoluteValues(false);
        tdiFilter->SetInputImage(intraAxonalVolumeImage);
        tdiFilter->SetUseImageGeometry(true);
        tdiFilter->Update();
        intraAxonalVolumeImage = tdiFilter->GetOutput();

        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        boost::progress_display disp(m_MaskImage->GetLargestPossibleRegion().GetNumberOfPixels()*m_Parameters.m_SignalGen.GetNumVolumes());

        for (unsigned int g=0; g<m_Parameters.m_SignalGen.GetNumVolumes(); g++)
        {
            // Set signal model random generator seeds to get same configuration in each voxel
            for (int i=0; i<m_Parameters.m_FiberModelList.size(); i++)
                m_Parameters.m_FiberModelList.at(i)->SetSeed(signalModelSeed);
            for (int i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
                m_Parameters.m_NonFiberModelList.at(i)->SetSeed(signalModelSeed);

            if (m_Parameters.m_SignalGen.m_DoAddMotion && g>0)  // if fibers have moved we need a new TDI and new directions
            {
                fOdfFilter->SetFiberBundle(m_FiberBundleTransformed);
                fOdfFilter->SetMaskImage(m_MaskImage);
                fOdfFilter->Update();
                directionImageContainer = fOdfFilter->GetDirectionImageContainer();

                tdiFilter->SetFiberBundle(m_FiberBundleTransformed);
                tdiFilter->Update();
                intraAxonalVolumeImage = tdiFilter->GetOutput();
            }

            ImageRegionIterator< ItkUcharImgType > it(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
            while(!it.IsAtEnd())
            {
                ++disp;
                unsigned long newTick = 50*disp.count()/disp.expected_count();
                for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                    m_StatusText += "*";
                lastTick = newTick;

                if (this->GetAbortGenerateData())
                {
                    m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
                    return;
                }

                if (it.Get()>0)
                {
                    // generate fiber signal
                    for (int c=0; c<m_Parameters.m_FiberModelList.size(); c++)
                    {
                        int count = 0;
                        DoubleDwiType::PixelType pix = m_CompartmentImages.at(c)->GetPixel(it.GetIndex());
                        for (unsigned int i=0; i<directionImageContainer->Size(); i++)
                        {
                            itk::Vector< double, 3> dir;
                            dir.CastFrom(directionImageContainer->GetElement(i)->GetPixel(it.GetIndex()));
                            double norm = dir.GetNorm();
                            if (norm>0.0001)
                            {
                                m_Parameters.m_FiberModelList.at(c)->SetFiberDirection(dir);
                                pix[g] += m_Parameters.m_FiberModelList.at(c)->SimulateMeasurement(g)*norm;
                                count++;
                            }
                        }
                        if (count>0)
                            pix[g] /= count;
                        pix[g] *= intraAxonalVolumeImage->GetPixel(it.GetIndex())*m_VoxelVolume;
                        m_CompartmentImages.at(c)->SetPixel(it.GetIndex(), pix);
                    }

                    // simulate other compartments
                    SimulateNonFiberSignal(it.GetIndex(), intraAxonalVolumeImage->GetPixel(it.GetIndex())*m_VoxelVolume, g);
                }
                ++it;
            }

            SimulateMotion(g);
        }

        itk::ImageFileWriter< ItkUcharImgType >::Pointer wr = itk::ImageFileWriter< ItkUcharImgType >::New();
        wr->SetInput(fOdfFilter->GetNumDirectionsImage());
        wr->SetFileName(mitk::IOUtil::GetTempPath()+"/NumDirections_MainFiberDirections.nrrd");
        wr->Update();
        break;
    }
    case (SignalGenerationParameters::RANDOM_DIRECTIONS):
    {
        ItkUcharImgType::Pointer numDirectionsImage = ItkUcharImgType::New();
        numDirectionsImage->SetSpacing( m_UpsampledSpacing );
        numDirectionsImage->SetOrigin( m_UpsampledOrigin );
        numDirectionsImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        numDirectionsImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        numDirectionsImage->SetBufferedRegion( m_UpsampledImageRegion );
        numDirectionsImage->SetRequestedRegion( m_UpsampledImageRegion );
        numDirectionsImage->Allocate();
        numDirectionsImage->FillBuffer(0);
        double sepAngle = cos(m_Parameters.m_SignalGen.m_FiberSeparationThreshold*M_PI/180.0);

        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        boost::progress_display disp(m_MaskImage->GetLargestPossibleRegion().GetNumberOfPixels());
        ImageRegionIterator<ItkUcharImgType> it(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned int tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;

            if (this->GetAbortGenerateData())
            {
                m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
                return;
            }

            if (it.Get()>0)
            {
                int numFibs = m_RandGen->GetIntegerVariate(2)+1;
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(0)->GetPixel(it.GetIndex());
                double volume = m_RandGen->GetVariateWithClosedRange(0.3);

                //                double sum = 0;
                std::vector< double > fractions;
                for (int i=0; i<numFibs; i++)
                {
                    //                    fractions.push_back(1);
                    fractions.push_back(0.5+m_RandGen->GetVariateWithClosedRange(0.5));
                    //                    sum += fractions.at(i);
                }
                //                for (int i=0; i<numFibs; i++)
                //                    fractions[i] /= sum;

                std::vector< itk::Vector<double, 3> > directions;
                for (int i=0; i<numFibs; i++)
                {
                    DoubleVectorType fib;
                    fib[0] = m_RandGen->GetVariateWithClosedRange(2)-1.0;
                    fib[1] = m_RandGen->GetVariateWithClosedRange(2)-1.0;
                    fib[2] = m_RandGen->GetVariateWithClosedRange(2)-1.0;
                    fib.Normalize();

                    double min = 0;
                    for (unsigned int d=0; d<directions.size(); d++)
                    {
                        double angle = fabs(fib*directions[d]);
                        if (angle>min)
                            min = angle;
                    }
                    if (min<sepAngle)
                    {
                        m_Parameters.m_FiberModelList.at(0)->SetFiberDirection(fib);
                        pix += m_Parameters.m_FiberModelList.at(0)->SimulateMeasurement()*fractions[i];
                        directions.push_back(fib);
                    }
                    else
                        i--;
                }
                pix *= (1-volume);
                m_CompartmentImages.at(0)->SetPixel(it.GetIndex(), pix);

                // CSF/GM
                {
                    pix += volume*m_Parameters.m_NonFiberModelList.at(0)->SimulateMeasurement();
                }

                numDirectionsImage->SetPixel(it.GetIndex(), numFibs);
            }
            ++it;
        }

        itk::ImageFileWriter< ItkUcharImgType >::Pointer wr = itk::ImageFileWriter< ItkUcharImgType >::New();
        wr->SetInput(numDirectionsImage);
        wr->SetFileName(mitk::IOUtil::GetTempPath()+"/NumDirections_RandomDirections.nrrd");
        wr->Update();
    }
    }

    if (m_Logfile.is_open())
    {
        m_Logfile << "DONE";
        m_Logfile.close();
    }
    m_StatusText += "\n\n";
    if (this->GetAbortGenerateData())
    {
        m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
        return;
    }

    DoubleDwiType::Pointer doubleOutImage;
    if ( m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition ) // do k-space stuff
    {
        m_StatusText += this->GetTime()+" > Adjusting complex signal\n";
        MITK_INFO << "Adjusting complex signal:";

        if (m_Parameters.m_SignalGen.m_DoSimulateRelaxation)
            m_StatusText += "Simulating signal relaxation\n";
        if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
            m_StatusText += "Simulating distortions\n";
        if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
            m_StatusText += "Simulating ringing artifacts\n";
        if (m_Parameters.m_SignalGen.m_EddyStrength>0)
            m_StatusText += "Simulating eddy currents\n";
        if (m_Parameters.m_SignalGen.m_Spikes>0)
            m_StatusText += "Simulating spikes\n";
        if (m_Parameters.m_SignalGen.m_CroppingFactor<1.0)
            m_StatusText += "Simulating aliasing artifacts\n";
        if (m_Parameters.m_SignalGen.m_KspaceLineOffset>0)
            m_StatusText += "Simulating ghosts\n";

        doubleOutImage = DoKspaceStuff(m_CompartmentImages);
        m_Parameters.m_SignalGen.m_SignalScale = 1; // already scaled in DoKspaceStuff
    }
    else    // don't do k-space stuff, just sum compartments
    {
        m_StatusText += this->GetTime()+" > Summing compartments\n";
        MITK_INFO << "Summing compartments";
        doubleOutImage = m_CompartmentImages.at(0);

        for (unsigned int i=1; i<m_CompartmentImages.size(); i++)
        {
            itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::Pointer adder = itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::New();
            adder->SetInput1(doubleOutImage);
            adder->SetInput2(m_CompartmentImages.at(i));
            adder->Update();
            doubleOutImage = adder->GetOutput();
        }
    }
    if (this->GetAbortGenerateData())
    {
        m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
        return;
    }

    m_StatusText += this->GetTime()+" > Finalizing image\n";
    MITK_INFO << "Finalizing image";
    if (m_Parameters.m_SignalGen.m_SignalScale>1)
        m_StatusText += " Scaling signal\n";
    if (m_Parameters.m_NoiseModel!=NULL)
        m_StatusText += " Adding noise\n";
    unsigned int window = 0;
    unsigned int min = itk::NumericTraits<unsigned int>::max();
    ImageRegionIterator<OutputImageType> it4 (outImage, outImage->GetLargestPossibleRegion());
    DoubleDwiType::PixelType signal; signal.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
    boost::progress_display disp2(outImage->GetLargestPossibleRegion().GetNumberOfPixels());

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    lastTick = 0;

    while(!it4.IsAtEnd())
    {
        if (this->GetAbortGenerateData())
        {
            m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
            return;
        }

        ++disp2;
        unsigned long newTick = 50*disp2.count()/disp2.expected_count();
        for (unsigned long tick = 0; tick<(newTick-lastTick); tick++)
            m_StatusText += "*";
        lastTick = newTick;

        typename OutputImageType::IndexType index = it4.GetIndex();
        signal = doubleOutImage->GetPixel(index)*m_Parameters.m_SignalGen.m_SignalScale;

        if (m_Parameters.m_NoiseModel!=NULL)
            m_Parameters.m_NoiseModel->AddNoise(signal);

        for (unsigned int i=0; i<signal.Size(); i++)
        {
            if (signal[i]>0)
                signal[i] = floor(signal[i]+0.5);
            else
                signal[i] = ceil(signal[i]-0.5);

            if ( (!m_Parameters.m_SignalGen.IsBaselineIndex(i) || signal.Size()==1) && signal[i]>window)
                window = signal[i];
            if ( (!m_Parameters.m_SignalGen.IsBaselineIndex(i) || signal.Size()==1) && signal[i]<min)
                min = signal[i];
        }
        it4.Set(signal);
        ++it4;
    }
    window -= min;
    unsigned int level = window/2 + min;
    m_LevelWindow.SetLevelWindow(level, window);
    this->SetNthOutput(0, outImage);

    m_StatusText += "\n\n";
    m_StatusText += "Finished simulation\n";
    m_StatusText += "Simulation time: "+GetTime();
    m_TimeProbe.Stop();
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::SimulateMotion(int g)
{
    if (m_Parameters.m_SignalGen.m_DoAddMotion && g<m_Parameters.m_SignalGen.GetNumVolumes()-1)
    {
        if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
        {
            m_FiberBundleTransformed = m_FiberBundleWorkingCopy->GetDeepCopy();
            m_Rotation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[0]*2)-m_Parameters.m_SignalGen.m_Rotation[0];
            m_Rotation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[1]*2)-m_Parameters.m_SignalGen.m_Rotation[1];
            m_Rotation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[2]*2)-m_Parameters.m_SignalGen.m_Rotation[2];
            m_Translation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[0]*2)-m_Parameters.m_SignalGen.m_Translation[0];
            m_Translation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[1]*2)-m_Parameters.m_SignalGen.m_Translation[1];
            m_Translation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[2]*2)-m_Parameters.m_SignalGen.m_Translation[2];
        }

        // rotate mask image
        if (m_MaskImageSet)
        {
            ImageRegionIterator<ItkUcharImgType> maskIt(m_UpsampledMaskImage, m_UpsampledMaskImage->GetLargestPossibleRegion());
            m_MaskImage->FillBuffer(0);

            while(!maskIt.IsAtEnd())
            {
                if (maskIt.Get()<=0)
                {
                    ++maskIt;
                    continue;
                }

                DoubleDwiType::IndexType index = maskIt.GetIndex();
                itk::Point<double, 3> point;
                m_UpsampledMaskImage->TransformIndexToPhysicalPoint(index, point);
                if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
                    point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), m_Rotation[0],m_Rotation[1],m_Rotation[2],m_Translation[0],m_Translation[1],m_Translation[2]);
                else
                    point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), m_Rotation[0]*(g+1),m_Rotation[1]*(g+1),m_Rotation[2]*(g+1),m_Translation[0]*(g+1),m_Translation[1]*(g+1),m_Translation[2]*(g+1));

                m_MaskImage->TransformPhysicalPointToIndex(point, index);
                if (m_MaskImage->GetLargestPossibleRegion().IsInside(index))
                    m_MaskImage->SetPixel(index,100);
                ++maskIt;
            }
        }

        // rotate fibers
        if (m_Logfile.is_open())
        {
            m_Logfile << g+1 << " rotation: " << m_Rotation[0] << "," << m_Rotation[1] << "," << m_Rotation[2] << ";";
            m_Logfile << " translation: " << m_Translation[0] << "," << m_Translation[1] << "," << m_Translation[2] << "\n";
        }
        m_FiberBundleTransformed->TransformFibers(m_Rotation[0],m_Rotation[1],m_Rotation[2],m_Translation[0],m_Translation[1],m_Translation[2]);
    }
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::SimulateNonFiberSignal(ItkUcharImgType::IndexType index, double intraAxonalVolume, int g)
{
    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();

    if (intraAxonalVolume>0.0001 && m_Parameters.m_SignalGen.m_DoDisablePartialVolume)  // only fiber in voxel
    {
        DoubleDwiType::PixelType pix = m_CompartmentImages.at(0)->GetPixel(index);
        if (g>=0)
            pix[g] *= m_VoxelVolume/intraAxonalVolume;
        else
            pix *= m_VoxelVolume/intraAxonalVolume;
        m_CompartmentImages.at(0)->SetPixel(index, pix);
        m_VolumeFractions.at(0)->SetPixel(index, 1);
        for (int i=1; i<numFiberCompartments; i++)
        {
            DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
            if (g>=0)
                pix[g] = 0.0;
            else
                pix.Fill(0.0);
            m_CompartmentImages.at(i)->SetPixel(index, pix);
        }
    }
    else
    {
        m_VolumeFractions.at(0)->SetPixel(index, intraAxonalVolume/m_VoxelVolume);

        itk::Point<double, 3> point;
        m_MaskImage->TransformIndexToPhysicalPoint(index, point);
        if (m_Parameters.m_SignalGen.m_DoAddMotion)
        {
            if (m_Parameters.m_SignalGen.m_DoRandomizeMotion && g>0)
                point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0],-m_Rotation[1],-m_Rotation[2],-m_Translation[0],-m_Translation[1],-m_Translation[2]);
            else if (g>=0)
                point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0]*g,-m_Rotation[1]*g,-m_Rotation[2]*g,-m_Translation[0]*g,-m_Translation[1]*g,-m_Translation[2]*g);
        }

        if (m_Parameters.m_SignalGen.m_DoDisablePartialVolume)
        {
            int maxVolumeIndex = 0;
            double maxWeight = 0;
            for (int i=0; i<numNonFiberCompartments; i++)
            {
                double weight = 0;
                if (numNonFiberCompartments>1)
                {
                    DoubleDwiType::IndexType newIndex;
                    m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->TransformPhysicalPointToIndex(point, newIndex);
                    if (!m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion().IsInside(newIndex))
                        continue;
                    weight = m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetPixel(newIndex);
                }

                if (weight>maxWeight)
                {
                    maxWeight = weight;
                    maxVolumeIndex = i;
                }
            }
            DoubleDwiType::Pointer doubleDwi = m_CompartmentImages.at(maxVolumeIndex+numFiberCompartments);
            DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);

            if (g>=0)
                pix[g] += m_Parameters.m_NonFiberModelList[maxVolumeIndex]->SimulateMeasurement(g);
            else
                pix += m_Parameters.m_NonFiberModelList[maxVolumeIndex]->SimulateMeasurement();
            doubleDwi->SetPixel(index, pix);
            m_VolumeFractions.at(maxVolumeIndex+numFiberCompartments)->SetPixel(index, 1);
        }
        else
        {
            double extraAxonalVolume = m_VoxelVolume-intraAxonalVolume;    // non-fiber volume
            double interAxonalVolume = 0;
            if (numFiberCompartments>1)
                interAxonalVolume = extraAxonalVolume * intraAxonalVolume/m_VoxelVolume;   // inter-axonal fraction of non fiber compartment scales linearly with f
            double other = extraAxonalVolume - interAxonalVolume;        // rest of compartment
            double singleinter = interAxonalVolume/(numFiberCompartments-1);

            // adjust non-fiber and intra-axonal signal
            for (int i=1; i<numFiberCompartments; i++)
            {
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                if (intraAxonalVolume>0)    // remove scaling by intra-axonal volume from inter-axonal compartment
                {
                    if (g>=0)
                        pix[g] /= intraAxonalVolume;
                    else
                        pix /= intraAxonalVolume;
                }
                if (g>=0)
                    pix[g] *= singleinter;
                else
                    pix *= singleinter;
                m_CompartmentImages.at(i)->SetPixel(index, pix);
                m_VolumeFractions.at(i)->SetPixel(index, singleinter/m_VoxelVolume);
            }

            for (int i=0; i<numNonFiberCompartments; i++)
            {
                double weight = 1;
                if (numNonFiberCompartments>1)
                {
                    DoubleDwiType::IndexType newIndex;
                    m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->TransformPhysicalPointToIndex(point, newIndex);
                    if (!m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetLargestPossibleRegion().IsInside(newIndex))
                        continue;
                    weight = m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()->GetPixel(newIndex);
                }

                DoubleDwiType::Pointer doubleDwi = m_CompartmentImages.at(i+numFiberCompartments);
                DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);

                if (g>=0)
                    pix[g] += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement(g)*other*weight;
                else
                    pix += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement()*other*weight;
                doubleDwi->SetPixel(index, pix);
                m_VolumeFractions.at(i+numFiberCompartments)->SetPixel(index, other/m_VoxelVolume*weight);
            }
        }
    }
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

template< class PixelType >
double TractsToDWIImageFilter< PixelType >::RoundToNearest(double num) {
    return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5);
}

template< class PixelType >
std::string TractsToDWIImageFilter< PixelType >::GetTime()
{
    m_TimeProbe.Stop();
    unsigned long total = RoundToNearest(m_TimeProbe.GetTotal());
    unsigned long hours = total/3600;
    unsigned long minutes = (total%3600)/60;
    unsigned long seconds = total%60;
    std::string out = "";
    out.append(boost::lexical_cast<std::string>(hours));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(minutes));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(seconds));
    m_TimeProbe.Start();
    return out;
}

}
