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
#include <boost/lexical_cast.hpp>

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
    // create slice object
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
    sliceRegion.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
    Vector< double, 2 > sliceSpacing;
    sliceSpacing[0] = m_UpsampledSpacing[0];
    sliceSpacing[1] = m_UpsampledSpacing[1];

    // frequency map slice
    SliceType::Pointer fMapSlice = NULL;
    if (m_Parameters.m_FrequencyMap.IsNotNull())
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
    newImage->SetSpacing( m_Parameters.m_ImageSpacing );
    newImage->SetOrigin( m_Parameters.m_ImageOrigin );
    newImage->SetDirection( m_Parameters.m_ImageDirection );
    newImage->SetLargestPossibleRegion( m_Parameters.m_ImageRegion );
    newImage->SetBufferedRegion( m_Parameters.m_ImageRegion );
    newImage->SetRequestedRegion( m_Parameters.m_ImageRegion );
    newImage->SetVectorLength( images.at(0)->GetVectorLength() );
    newImage->Allocate();

    std::vector< unsigned int > spikeVolume;
    for (unsigned int i=0; i<m_Parameters.m_Spikes; i++)
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
                if (i<m_Parameters.m_FiberModelList.size())
                    signalModel = m_Parameters.m_FiberModelList.at(i);
                else
                    signalModel = m_Parameters.m_NonFiberModelList.at(i-m_Parameters.m_FiberModelList.size());

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
                            fMapSlice->SetPixel(index2D, m_Parameters.m_FrequencyMap->GetPixel(index3D));
                    }

                compartmentSlices.push_back(slice);
                t2Vector.push_back(signalModel->GetT2());
            }

            if (this->GetAbortGenerateData())
                return NULL;

            // create k-sapce (inverse fourier transform slices)
            itk::Size<2> outSize; outSize.SetElement(0, m_Parameters.m_ImageRegion.GetSize(0)); outSize.SetElement(1, m_Parameters.m_ImageRegion.GetSize(1));
            itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
            idft->SetCompartmentImages(compartmentSlices);
            idft->SetT2(t2Vector);
            idft->SetUseConstantRandSeed(m_UseConstantRandSeed);
            idft->SetParameters(m_Parameters);
            idft->SetZ((double)z-(double)images.at(0)->GetLargestPossibleRegion().GetSize(2)/2.0);
            idft->SetDiffusionGradientDirection(m_Parameters.GetGradientDirection(g));
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

    if (m_Parameters.m_DoDisablePartialVolume)
        while (m_Parameters.m_FiberModelList.size()>1)
            m_Parameters.m_FiberModelList.pop_back();

    if (m_Parameters.m_NonFiberModelList.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined!");

    int baselineIndex = m_Parameters.GetFirstBaselineIndex();
    if (baselineIndex<0)
        itkExceptionMacro("No baseline index found!");

    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        m_RandGen->SetSeed(0);
    else
        m_RandGen->SetSeed();

    // initialize output dwi image
    ImageRegion<3> croppedRegion = m_Parameters.m_ImageRegion; croppedRegion.SetSize(1, croppedRegion.GetSize(1)*m_Parameters.m_CroppingFactor);
    itk::Point<double,3> shiftedOrigin = m_Parameters.m_ImageOrigin; shiftedOrigin[1] += (m_Parameters.m_ImageRegion.GetSize(1)-croppedRegion.GetSize(1))*m_Parameters.m_ImageSpacing[1]/2;
    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Parameters.m_ImageSpacing );
    outImage->SetOrigin( shiftedOrigin );
    outImage->SetDirection( m_Parameters.m_ImageDirection );
    outImage->SetLargestPossibleRegion( croppedRegion );
    outImage->SetBufferedRegion( croppedRegion );
    outImage->SetRequestedRegion( croppedRegion );
    outImage->SetVectorLength( m_Parameters.GetNumVolumes() );
    outImage->Allocate();
    typename OutputImageType::PixelType temp;
    temp.SetSize(m_Parameters.GetNumVolumes());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);

    // ADJUST GEOMETRY FOR FURTHER PROCESSING
    // is input slize size a power of two?
    unsigned int x=m_Parameters.m_ImageRegion.GetSize(0); unsigned int y=m_Parameters.m_ImageRegion.GetSize(1);
    ItkDoubleImgType::SizeType pad; pad[0]=x%2; pad[1]=y%2; pad[2]=0;
    m_Parameters.m_ImageRegion.SetSize(0, x+pad[0]);
    m_Parameters.m_ImageRegion.SetSize(1, y+pad[1]);
    if (m_Parameters.m_FrequencyMap.IsNotNull() && (pad[0]>0 || pad[1]>0))
    {
        itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer zeroPadder = itk::ConstantPadImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
        zeroPadder->SetInput(m_Parameters.m_FrequencyMap);
        zeroPadder->SetConstant(0);
        zeroPadder->SetPadUpperBound(pad);
        zeroPadder->Update();
        m_Parameters.m_FrequencyMap = zeroPadder->GetOutput();
    }
    if (m_Parameters.m_MaskImage.IsNotNull() && (pad[0]>0 || pad[1]>0))
    {
        itk::ConstantPadImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer zeroPadder = itk::ConstantPadImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
        zeroPadder->SetInput(m_Parameters.m_MaskImage);
        zeroPadder->SetConstant(0);
        zeroPadder->SetPadUpperBound(pad);
        zeroPadder->Update();
        m_Parameters.m_MaskImage = zeroPadder->GetOutput();
    }

    // Apply in-plane upsampling for Gibbs ringing artifact
    double upsampling = 1;
    if (m_Parameters.m_DoAddGibbsRinging)
        upsampling = 2;
    m_UpsampledSpacing = m_Parameters.m_ImageSpacing;
    m_UpsampledSpacing[0] /= upsampling;
    m_UpsampledSpacing[1] /= upsampling;
    m_UpsampledImageRegion = m_Parameters.m_ImageRegion;
    m_UpsampledImageRegion.SetSize(0, m_Parameters.m_ImageRegion.GetSize()[0]*upsampling);
    m_UpsampledImageRegion.SetSize(1, m_Parameters.m_ImageRegion.GetSize()[1]*upsampling);
    m_UpsampledOrigin = m_Parameters.m_ImageOrigin;
    m_UpsampledOrigin[0] -= m_Parameters.m_ImageSpacing[0]/2; m_UpsampledOrigin[0] += m_UpsampledSpacing[0]/2;
    m_UpsampledOrigin[1] -= m_Parameters.m_ImageSpacing[1]/2; m_UpsampledOrigin[1] += m_UpsampledSpacing[1]/2;
    m_UpsampledOrigin[2] -= m_Parameters.m_ImageSpacing[2]/2; m_UpsampledOrigin[2] += m_UpsampledSpacing[2]/2;

    // generate double images to store the individual compartment signals
    std::vector< DoubleDwiType::Pointer > compartments;
    for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_UpsampledSpacing );
        doubleDwi->SetOrigin( m_UpsampledOrigin );
        doubleDwi->SetDirection( m_Parameters.m_ImageDirection );
        doubleDwi->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleDwi->SetBufferedRegion( m_UpsampledImageRegion );
        doubleDwi->SetRequestedRegion( m_UpsampledImageRegion );
        doubleDwi->SetVectorLength( m_Parameters.GetNumVolumes() );
        doubleDwi->Allocate();
        DoubleDwiType::PixelType pix;
        pix.SetSize(m_Parameters.GetNumVolumes());
        pix.Fill(0.0);
        doubleDwi->FillBuffer(pix);
        compartments.push_back(doubleDwi);
    }

    // initialize volume fraction images
    m_VolumeFractions.clear();
    for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
    {
        ItkDoubleImgType::Pointer doubleImg = ItkDoubleImgType::New();
        doubleImg->SetSpacing( m_UpsampledSpacing );
        doubleImg->SetOrigin( m_UpsampledOrigin );
        doubleImg->SetDirection( m_Parameters.m_ImageDirection );
        doubleImg->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleImg->SetBufferedRegion( m_UpsampledImageRegion );
        doubleImg->SetRequestedRegion( m_UpsampledImageRegion );
        doubleImg->Allocate();
        doubleImg->FillBuffer(0);
        m_VolumeFractions.push_back(doubleImg);
    }

    // resample mask image and frequency map to fit upsampled geometry
    if (m_Parameters.m_DoAddGibbsRinging)
    {
        if (m_Parameters.m_MaskImage.IsNotNull())
        {
            // rescale mask image (otherwise there are problems with the resampling)
            itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::Pointer rescaler = itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::New();
            rescaler->SetInput(0,m_Parameters.m_MaskImage);
            rescaler->SetOutputMaximum(100);
            rescaler->SetOutputMinimum(0);
            rescaler->Update();

            // resample mask image
            itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer resampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
            resampler->SetInput(rescaler->GetOutput());
            resampler->SetOutputParametersFromImage(m_Parameters.m_MaskImage);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_MaskImage = resampler->GetOutput();

            itk::ImageFileWriter<ItkUcharImgType>::Pointer w = itk::ImageFileWriter<ItkUcharImgType>::New();
            w->SetFileName("/local/mask_ups.nrrd");
            w->SetInput(m_Parameters.m_MaskImage);
            w->Update();
        }
        // resample frequency map
        if (m_Parameters.m_FrequencyMap.IsNotNull())
        {
            itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer resampler = itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
            resampler->SetInput(m_Parameters.m_FrequencyMap);
            resampler->SetOutputParametersFromImage(m_Parameters.m_FrequencyMap);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_FrequencyMap = resampler->GetOutput();
        }
    }

    // no input tissue mask is set -> create default
    bool maskImageSet = true;
    if (m_Parameters.m_MaskImage.IsNull())
    {
        m_StatusText += "No tissue mask set\n";
        MITK_INFO << "No tissue mask set";
        m_Parameters.m_MaskImage = ItkUcharImgType::New();
        m_Parameters.m_MaskImage->SetSpacing( m_UpsampledSpacing );
        m_Parameters.m_MaskImage->SetOrigin( m_UpsampledOrigin );
        m_Parameters.m_MaskImage->SetDirection( m_Parameters.m_ImageDirection );
        m_Parameters.m_MaskImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        m_Parameters.m_MaskImage->SetBufferedRegion( m_UpsampledImageRegion );
        m_Parameters.m_MaskImage->SetRequestedRegion( m_UpsampledImageRegion );
        m_Parameters.m_MaskImage->Allocate();
        m_Parameters.m_MaskImage->FillBuffer(1);
        maskImageSet = false;
    }
    else
    {
        m_StatusText += "Using tissue mask\n";
        MITK_INFO << "Using tissue mask";
    }

    m_Parameters.m_ImageRegion = croppedRegion;
    x=m_Parameters.m_ImageRegion.GetSize(0); y=m_Parameters.m_ImageRegion.GetSize(1);
    if ( x%2 == 1 )
        m_Parameters.m_ImageRegion.SetSize(0, x+1);
    if ( y%2 == 1 )
        m_Parameters.m_ImageRegion.SetSize(1, y+1);

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
    FiberBundleType fiberBundle = m_FiberBundle->GetDeepCopy();
    double volumeAccuracy = 10;
    fiberBundle->ResampleFibers(minSpacing/volumeAccuracy);
    double mmRadius = m_Parameters.m_AxonRadius/1000;
    if (mmRadius>0)
        segmentVolume = M_PI*mmRadius*mmRadius*minSpacing/volumeAccuracy;

    double maxVolume = 0;
    double voxelVolume = m_UpsampledSpacing[0]*m_UpsampledSpacing[1]*m_UpsampledSpacing[2];

    ofstream logFile;
    if (m_Parameters.m_DoAddMotion)
    {
        std::string fileName = "fiberfox_motion_0.log";
        int c = 1;
        while (itksys::SystemTools::FileExists(mitk::IOUtil::GetTempPath().append(fileName).c_str()))
        {
            fileName = "fiberfox_motion_";
            fileName += boost::lexical_cast<std::string>(c);
            fileName += ".log";
            c++;
        }

        logFile.open(mitk::IOUtil::GetTempPath().append(fileName).c_str());
        logFile << "0 rotation: 0,0,0; translation: 0,0,0\n";

        if (m_Parameters.m_DoRandomizeMotion)
        {
            m_StatusText += "Adding random motion artifacts:\n";
            m_StatusText += "Maximum rotation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_Rotation) + "°\n";
            m_StatusText += "Maximum translation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_Translation) + "mm\n";
        }
        else
        {
            m_StatusText += "Adding linear motion artifacts:\n";
            m_StatusText += "Maximum rotation: " + boost::lexical_cast<std::string>(m_Parameters.m_Rotation) + "°\n";
            m_StatusText += "Maximum translation: " + boost::lexical_cast<std::string>(m_Parameters.m_Translation) + "mm\n";
        }
        m_StatusText += "Motion logfile: " + mitk::IOUtil::GetTempPath().append(fileName) + "\n";
        MITK_INFO << "Adding motion artifacts";
        MITK_INFO << "Maximum rotation: " << m_Parameters.m_Rotation;
        MITK_INFO << "Maxmimum translation: " << m_Parameters.m_Translation;
    }
    maxVolume = 0;

    m_StatusText += "\n"+this->GetTime()+" > Generating signal of " + boost::lexical_cast<std::string>(m_Parameters.m_FiberModelList.size()) + " fiber compartments\n";
    MITK_INFO << "Generating signal of " << m_Parameters.m_FiberModelList.size() << " fiber compartments";
    int numFibers = m_FiberBundle->GetNumFibers();
    boost::progress_display disp(numFibers*m_Parameters.GetNumVolumes());


    // get transform for motion artifacts
    FiberBundleType fiberBundleTransformed = fiberBundle;
    VectorType rotation = m_Parameters.m_Rotation/m_Parameters.GetNumVolumes();
    VectorType translation = m_Parameters.m_Translation/m_Parameters.GetNumVolumes();

    // creat image to hold transformed mask (motion artifact)
    ItkUcharImgType::Pointer tempTissueMask = ItkUcharImgType::New();
    itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
    duplicator->SetInputImage(m_Parameters.m_MaskImage);
    duplicator->Update();
    tempTissueMask = duplicator->GetOutput();

    // second upsampling needed for motion artifacts
    ImageRegion<3>                      upsampledImageRegion = m_UpsampledImageRegion;
    itk::Vector<double,3>               upsampledSpacing = m_UpsampledSpacing;
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
    ItkUcharImgType::Pointer upsampledTissueMask = ItkUcharImgType::New();
    itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer upsampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
    upsampler->SetInput(m_Parameters.m_MaskImage);
    upsampler->SetOutputParametersFromImage(m_Parameters.m_MaskImage);
    upsampler->SetSize(upsampledImageRegion.GetSize());
    upsampler->SetOutputSpacing(upsampledSpacing);
    upsampler->SetOutputOrigin(upsampledOrigin);
    itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::Pointer nn_interpolator
            = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
    upsampler->SetInterpolator(nn_interpolator);
    upsampler->Update();
    upsampledTissueMask = upsampler->GetOutput();

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    unsigned long lastTick = 0;

    for (unsigned int g=0; g<m_Parameters.GetNumVolumes(); g++)
    {
        vtkPolyData* fiberPolyData = fiberBundleTransformed->GetFiberPolyData();

        ItkDoubleImgType::Pointer intraAxonalVolumeImage = ItkDoubleImgType::New();
        intraAxonalVolumeImage->SetSpacing( m_UpsampledSpacing );
        intraAxonalVolumeImage->SetOrigin( m_UpsampledOrigin );
        intraAxonalVolumeImage->SetDirection( m_Parameters.m_ImageDirection );
        intraAxonalVolumeImage->SetLargestPossibleRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->SetBufferedRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->SetRequestedRegion( m_UpsampledImageRegion );
        intraAxonalVolumeImage->Allocate();
        intraAxonalVolumeImage->FillBuffer(0);

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
                    tempTissueMask->TransformPhysicalPointToIndex(vertex, idx);
                    tempTissueMask->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

                    if (!tempTissueMask->GetLargestPossibleRegion().IsInside(idx) || tempTissueMask->GetPixel(idx)<=0)
                        continue;

                    // generate signal for each fiber compartment
                    for (unsigned int k=0; k<m_Parameters.m_FiberModelList.size(); k++)
                    {
                        m_Parameters.m_FiberModelList[k]->SetFiberDirection(dir);
                        DoubleDwiType::PixelType pix = compartments.at(k)->GetPixel(idx);
                        pix[g] += segmentVolume*m_Parameters.m_FiberModelList[k]->SimulateMeasurement(g);
                        compartments.at(k)->SetPixel(idx, pix);
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
        ImageRegionIterator<ItkUcharImgType> it3(tempTissueMask, tempTissueMask->GetLargestPossibleRegion());
        double fact = 1;
        if (m_Parameters.m_AxonRadius<0.0001 || maxVolume>voxelVolume)
            fact = voxelVolume/maxVolume;
        while(!it3.IsAtEnd())
        {
            if (it3.Get()>0)
            {
                DoubleDwiType::IndexType index = it3.GetIndex();

                // get fiber volume fraction
                double intraAxonalVolume = intraAxonalVolumeImage->GetPixel(index)*fact;

                for (unsigned int i=0; i<m_Parameters.m_FiberModelList.size(); i++)
                {
                    DoubleDwiType::PixelType pix = compartments.at(i)->GetPixel(index);
                    pix[g] *= fact;
                    compartments.at(i)->SetPixel(index, pix);
                }

                if (intraAxonalVolume>0.0001 && m_Parameters.m_DoDisablePartialVolume)  // only fiber in voxel
                {
                    DoubleDwiType::PixelType pix = compartments.at(0)->GetPixel(index);
                    pix[g] *= voxelVolume/intraAxonalVolume;
                    compartments.at(0)->SetPixel(index, pix);
                    m_VolumeFractions.at(0)->SetPixel(index, 1);
                    for (unsigned int i=1; i<m_Parameters.m_FiberModelList.size(); i++)
                    {
                        DoubleDwiType::PixelType pix = compartments.at(i)->GetPixel(index);
                        pix[g] = 0;
                        compartments.at(i)->SetPixel(index, pix);
                    }
                }
                else
                {
                    m_VolumeFractions.at(0)->SetPixel(index, intraAxonalVolume/voxelVolume);

                    double extraAxonalVolume = voxelVolume-intraAxonalVolume;    // non-fiber volume
                    double interAxonalVolume = 0;
                    if (m_Parameters.m_FiberModelList.size()>1)
                        interAxonalVolume = extraAxonalVolume * intraAxonalVolume/voxelVolume;   // inter-axonal fraction of non fiber compartment scales linearly with f
                    double other = extraAxonalVolume - interAxonalVolume;        // rest of compartment
                    double singleinter = interAxonalVolume/(m_Parameters.m_FiberModelList.size()-1);

                    // adjust non-fiber and intra-axonal signal
                    for (unsigned int i=1; i<m_Parameters.m_FiberModelList.size(); i++)
                    {
                        DoubleDwiType::PixelType pix = compartments.at(i)->GetPixel(index);
                        if (intraAxonalVolume>0)    // remove scaling by intra-axonal volume from inter-axonal compartment
                            pix[g] /= intraAxonalVolume;
                        pix[g] *= singleinter;
                        compartments.at(i)->SetPixel(index, pix);
                        m_VolumeFractions.at(i)->SetPixel(index, singleinter/voxelVolume);
                    }
                    for (unsigned int i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
                    {
                        DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_Parameters.m_FiberModelList.size());
                        DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                        pix[g] += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement(g)*other*m_Parameters.m_NonFiberModelList[i]->GetWeight();
                        doubleDwi->SetPixel(index, pix);
                        m_VolumeFractions.at(i+m_Parameters.m_FiberModelList.size())->SetPixel(index, other/voxelVolume*m_Parameters.m_NonFiberModelList[i]->GetWeight());
                    }
                }
            }
            ++it3;
        }

        // move fibers
        if (m_Parameters.m_DoAddMotion && g<m_Parameters.GetNumVolumes()-1)
        {
            if (m_Parameters.m_DoRandomizeMotion)
            {
                fiberBundleTransformed = fiberBundle->GetDeepCopy();
                rotation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Rotation[0]*2)-m_Parameters.m_Rotation[0];
                rotation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Rotation[1]*2)-m_Parameters.m_Rotation[1];
                rotation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Rotation[2]*2)-m_Parameters.m_Rotation[2];
                translation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Translation[0]*2)-m_Parameters.m_Translation[0];
                translation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Translation[1]*2)-m_Parameters.m_Translation[1];
                translation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_Translation[2]*2)-m_Parameters.m_Translation[2];
            }

            // rotate mask image
            if (maskImageSet)
            {
                ImageRegionIterator<ItkUcharImgType> maskIt(upsampledTissueMask, upsampledTissueMask->GetLargestPossibleRegion());
                tempTissueMask->FillBuffer(0);

                while(!maskIt.IsAtEnd())
                {
                    if (maskIt.Get()<=0)
                    {
                        ++maskIt;
                        continue;
                    }

                    DoubleDwiType::IndexType index = maskIt.GetIndex();
                    itk::Point<double, 3> point;
                    upsampledTissueMask->TransformIndexToPhysicalPoint(index, point);
                    if (m_Parameters.m_DoRandomizeMotion)
                        point = fiberBundle->TransformPoint(point.GetVnlVector(), rotation[0],rotation[1],rotation[2],translation[0],translation[1],translation[2]);
                    else
                        point = fiberBundle->TransformPoint(point.GetVnlVector(), rotation[0]*(g+1),rotation[1]*(g+1),rotation[2]*(g+1),translation[0]*(g+1),translation[1]*(g+1),translation[2]*(g+1));

                    tempTissueMask->TransformPhysicalPointToIndex(point, index);
                    if (tempTissueMask->GetLargestPossibleRegion().IsInside(index))
                        tempTissueMask->SetPixel(index,100);
                    ++maskIt;
                }
            }

            // rotate fibers
            if (logFile.is_open())
            {
                logFile << g+1 << " rotation: " << rotation[0] << "," << rotation[1] << "," << rotation[2] << ";";
                logFile << " translation: " << translation[0] << "," << translation[1] << "," << translation[2] << "\n";
            }
            fiberBundleTransformed->TransformFibers(rotation[0],rotation[1],rotation[2],translation[0],translation[1],translation[2]);
        }
    }
    if (logFile.is_open())
    {
        logFile << "DONE";
        logFile.close();
    }
    m_StatusText += "\n\n";
    if (this->GetAbortGenerateData())
    {
        m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
        return;
    }

    // do k-space stuff
    DoubleDwiType::Pointer doubleOutImage;
    if (m_Parameters.m_Spikes>0 || m_Parameters.m_FrequencyMap.IsNotNull() || m_Parameters.m_KspaceLineOffset>0 || m_Parameters.m_DoSimulateRelaxation || m_Parameters.m_EddyStrength>0 || m_Parameters.m_DoAddGibbsRinging || m_Parameters.m_CroppingFactor<1.0)
    {
        m_StatusText += this->GetTime()+" > Adjusting complex signal\n";
        MITK_INFO << "Adjusting complex signal:";

        if (m_Parameters.m_DoSimulateRelaxation)
            m_StatusText += "Simulating signal relaxation\n";
        if (m_Parameters.m_FrequencyMap.IsNotNull())
            m_StatusText += "Simulating distortions\n";
        if (m_Parameters.m_DoAddGibbsRinging)
            m_StatusText += "Simulating ringing artifacts\n";
        if (m_Parameters.m_EddyStrength>0)
            m_StatusText += "Simulating eddy currents\n";
        if (m_Parameters.m_Spikes>0)
            m_StatusText += "Simulating spikes\n";
        if (m_Parameters.m_CroppingFactor<1.0)
            m_StatusText += "Simulating aliasing artifacts\n";
        if (m_Parameters.m_KspaceLineOffset>0)
            m_StatusText += "Simulating ghosts\n";

        doubleOutImage = DoKspaceStuff(compartments);
        m_Parameters.m_SignalScale = 1;
    }
    else
    {
        m_StatusText += this->GetTime()+" > Summing compartments\n";
        MITK_INFO << "Summing compartments";
        doubleOutImage = compartments.at(0);

        for (unsigned int i=1; i<compartments.size(); i++)
        {
            itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::Pointer adder = itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::New();
            adder->SetInput1(doubleOutImage);
            adder->SetInput2(compartments.at(i));
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
    if (m_Parameters.m_SignalScale>1)
        m_StatusText += " Scaling signal\n";
    if (m_Parameters.m_NoiseModel!=NULL)
        m_StatusText += " Adding noise\n";
    unsigned int window = 0;
    unsigned int min = itk::NumericTraits<unsigned int>::max();
    ImageRegionIterator<OutputImageType> it4 (outImage, outImage->GetLargestPossibleRegion());
    DoubleDwiType::PixelType signal; signal.SetSize(m_Parameters.GetNumVolumes());
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
        signal = doubleOutImage->GetPixel(index)*m_Parameters.m_SignalScale;

        if (m_Parameters.m_NoiseModel!=NULL)
        {
            DoubleDwiType::PixelType accu = signal; accu.Fill(0.0);
            for (unsigned int i=0; i<m_Parameters.m_Repetitions; i++)
            {
                DoubleDwiType::PixelType temp = signal;
                m_Parameters.m_NoiseModel->AddNoise(temp);
                accu += temp;
            }
            signal = accu/m_Parameters.m_Repetitions;
        }

        for (unsigned int i=0; i<signal.Size(); i++)
        {
            if (signal[i]>0)
                signal[i] = floor(signal[i]+0.5);
            else
                signal[i] = ceil(signal[i]-0.5);

            if (!m_Parameters.IsBaselineIndex(i) && signal[i]>window)
                window = signal[i];
            if (!m_Parameters.IsBaselineIndex(i) && signal[i]<min)
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
