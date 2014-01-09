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
#include <boost/lexical_cast.hpp>

namespace itk
{

template< class PixelType >
TractsToDWIImageFilter< PixelType >::TractsToDWIImageFilter()
    : m_CircleDummy(false)
    , m_VolumeAccuracy(10)
    , m_AddGibbsRinging(false)
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
    , m_Wrap(1.0)
    , m_NoiseModel(NULL)
    , m_SpikeAmplitude(1)
    , m_AddMotionArtifact(false)
{
    m_Spacing.Fill(2.5); m_Origin.Fill(0.0);
    m_DirectionMatrix.SetIdentity();
    m_ImageRegion.SetSize(0, 10);
    m_ImageRegion.SetSize(1, 10);
    m_ImageRegion.SetSize(2, 10);

    m_MaxTranslation.Fill(0.0);
    m_MaxRotation.Fill(0.0);

    m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
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
    if (m_FrequencyMap.IsNotNull())
    {
        fMapSlice = SliceType::New();
        ImageRegion<2> region;
        region.SetSize(0, m_UpsampledImageRegion.GetSize()[0]);
        region.SetSize(1, m_UpsampledImageRegion.GetSize()[1]);
        fMapSlice->SetLargestPossibleRegion( region );
        fMapSlice->SetBufferedRegion( region );
        fMapSlice->SetRequestedRegion( region );
        fMapSlice->Allocate();
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

    std::vector< unsigned int > spikeVolume;
    for (int i=0; i<m_Spikes; i++)
        spikeVolume.push_back(rand()%images.at(0)->GetVectorLength());
    std::sort (spikeVolume.begin(), spikeVolume.end());
    std::reverse (spikeVolume.begin(), spikeVolume.end());

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    unsigned long lastTick = 0;

    boost::progress_display disp(2*images.at(0)->GetVectorLength()*images.at(0)->GetLargestPossibleRegion().GetSize(2));
    for (unsigned int g=0; g<images.at(0)->GetVectorLength(); g++)
    {
        std::vector< int > spikeSlice;
        while (!spikeVolume.empty() && spikeVolume.back()==g)
        {
            spikeSlice.push_back(rand()%images.at(0)->GetLargestPossibleRegion().GetSize(2));
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
                for (unsigned int y=0; y<images.at(0)->GetLargestPossibleRegion().GetSize(1); y++)
                    for (unsigned int x=0; x<images.at(0)->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        SliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;
                        DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;

                        slice->SetPixel(index2D, images.at(i)->GetPixel(index3D)[g]);

                        if (fMapSlice.IsNotNull() && i==0)
                            fMapSlice->SetPixel(index2D, m_FrequencyMap->GetPixel(index3D));
                    }

                compartmentSlices.push_back(slice);
                t2Vector.push_back(signalModel->GetT2());
            }

            if (this->GetAbortGenerateData())
                return NULL;

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
            idft->SetFrequencyMap(fMapSlice);
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

            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (int tick = 0; tick<(newTick-lastTick); tick++)
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
            for (int tick = 0; tick<(newTick-lastTick); tick++)
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
    m_StartTime = clock();
    m_StatusText = "Starting simulation\n";

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

    // initialize output dwi image
    ImageRegion<3> croppedRegion = m_ImageRegion; croppedRegion.SetSize(1, croppedRegion.GetSize(1)*m_Wrap);
    itk::Point<double,3> shiftedOrigin = m_Origin; shiftedOrigin[1] += (m_ImageRegion.GetSize(1)-croppedRegion.GetSize(1))*m_Spacing[1]/2;

    typename OutputImageType::Pointer outImage = OutputImageType::New();
    outImage->SetSpacing( m_Spacing );
    outImage->SetOrigin( shiftedOrigin );
    outImage->SetDirection( m_DirectionMatrix );
    outImage->SetLargestPossibleRegion( croppedRegion );
    outImage->SetBufferedRegion( croppedRegion );
    outImage->SetRequestedRegion( croppedRegion );
    outImage->SetVectorLength( m_FiberModels[0]->GetNumGradients() );
    outImage->Allocate();
    typename OutputImageType::PixelType temp;
    temp.SetSize(m_FiberModels[0]->GetNumGradients());
    temp.Fill(0.0);
    outImage->FillBuffer(temp);

    // ADJUST GEOMETRY FOR FURTHER PROCESSING
    // is input slize size a power of two?
    unsigned int x=m_ImageRegion.GetSize(0); unsigned int y=m_ImageRegion.GetSize(1);
    if ( x%2 == 1 )
        m_ImageRegion.SetSize(0, x+1);
    if ( y%2 == 1 )
        m_ImageRegion.SetSize(1, y+1);

    // apply in-plane upsampling
    double upsampling = 1;
    if (m_AddGibbsRinging)
    {
        m_StatusText += "Gibbs ringing enabled\n";
        MITK_INFO << "Adding ringing artifacts.";
        upsampling = 2;
    }
    m_UpsampledSpacing = m_Spacing;
    m_UpsampledSpacing[0] /= upsampling;
    m_UpsampledSpacing[1] /= upsampling;
    m_UpsampledImageRegion = m_ImageRegion;
    m_UpsampledImageRegion.SetSize(0, m_ImageRegion.GetSize()[0]*upsampling);
    m_UpsampledImageRegion.SetSize(1, m_ImageRegion.GetSize()[1]*upsampling);
    m_UpsampledOrigin = m_Origin;
    m_UpsampledOrigin[0] -= m_Spacing[0]/2; m_UpsampledOrigin[0] += m_UpsampledSpacing[0]/2;
    m_UpsampledOrigin[1] -= m_Spacing[1]/2; m_UpsampledOrigin[1] += m_UpsampledSpacing[1]/2;
    m_UpsampledOrigin[2] -= m_Spacing[2]/2; m_UpsampledOrigin[2] += m_UpsampledSpacing[2]/2;

    // generate double images to store the individual compartment signals
    std::vector< DoubleDwiType::Pointer > compartments;
    for (unsigned int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_UpsampledSpacing );
        doubleDwi->SetOrigin( m_UpsampledOrigin );
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

    // initialize volume fraction images
    m_VolumeFractions.clear();
    for (unsigned int i=0; i<m_FiberModels.size()+m_NonFiberModels.size(); i++)
    {
        ItkDoubleImgType::Pointer doubleImg = ItkDoubleImgType::New();
        doubleImg->SetSpacing( m_UpsampledSpacing );
        doubleImg->SetOrigin( m_UpsampledOrigin );
        doubleImg->SetDirection( m_DirectionMatrix );
        doubleImg->SetLargestPossibleRegion( m_UpsampledImageRegion );
        doubleImg->SetBufferedRegion( m_UpsampledImageRegion );
        doubleImg->SetRequestedRegion( m_UpsampledImageRegion );
        doubleImg->Allocate();
        doubleImg->FillBuffer(0);
        m_VolumeFractions.push_back(doubleImg);
    }

    // resample mask image and frequency map to fit upsampled geometry
    if (m_AddGibbsRinging)
    {
        if (m_TissueMask.IsNotNull())
        {
            // rescale mask image (otherwise there are problems with the resampling)
            itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::Pointer rescaler = itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::New();
            rescaler->SetInput(0,m_TissueMask);
            rescaler->SetOutputMaximum(100);
            rescaler->SetOutputMinimum(0);
            rescaler->Update();

            // resample mask image
            itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::Pointer resampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
            resampler->SetInput(rescaler->GetOutput());
            resampler->SetOutputParametersFromImage(m_TissueMask);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            resampler->Update();
            m_TissueMask = resampler->GetOutput();
        }

        // resample frequency map
        if (m_FrequencyMap.IsNotNull())
        {
            itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer resampler = itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
            resampler->SetInput(m_FrequencyMap);
            resampler->SetOutputParametersFromImage(m_FrequencyMap);
            resampler->SetSize(m_UpsampledImageRegion.GetSize());
            resampler->SetOutputSpacing(m_UpsampledSpacing);
            resampler->SetOutputOrigin(m_UpsampledOrigin);
            resampler->Update();
            m_FrequencyMap = resampler->GetOutput();
        }
    }

    // no input tissue mask is set -> create default
    bool maskImageSet = true;
    if (m_TissueMask.IsNull())
    {
        m_StatusText += "No tissue mask set\n";
        MITK_INFO << "No tissue mask set";
        m_TissueMask = ItkUcharImgType::New();
        m_TissueMask->SetSpacing( m_UpsampledSpacing );
        m_TissueMask->SetOrigin( m_UpsampledOrigin );
        m_TissueMask->SetDirection( m_DirectionMatrix );
        m_TissueMask->SetLargestPossibleRegion( m_UpsampledImageRegion );
        m_TissueMask->SetBufferedRegion( m_UpsampledImageRegion );
        m_TissueMask->SetRequestedRegion( m_UpsampledImageRegion );
        m_TissueMask->Allocate();
        m_TissueMask->FillBuffer(1);
        maskImageSet = false;
    }
    else
    {
        m_StatusText += "Using tissue mask\n";
        MITK_INFO << "Using tissue mask";
    }

    m_ImageRegion = croppedRegion;
    x=m_ImageRegion.GetSize(0); y=m_ImageRegion.GetSize(1);
    if ( x%2 == 1 )
        m_ImageRegion.SetSize(0, x+1);
    if ( y%2 == 1 )
        m_ImageRegion.SetSize(1, y+1);

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
    fiberBundle->ResampleFibers(minSpacing/m_VolumeAccuracy);
    double mmRadius = m_FiberRadius/1000;
    if (mmRadius>0)
        segmentVolume = M_PI*mmRadius*mmRadius*minSpacing/m_VolumeAccuracy;

    double interpFact = 2*atan(-0.5*m_InterpolationShrink);
    double maxVolume = 0;
    double voxelVolume = m_UpsampledSpacing[0]*m_UpsampledSpacing[1]*m_UpsampledSpacing[2];

    if (m_AddMotionArtifact)
    {
        if (m_RandomMotion)
        {
            m_StatusText += "Adding random motion artifacts:\n";
            m_StatusText += "Maximum rotation: +/-" + boost::lexical_cast<std::string>(m_MaxRotation) + "°\n";
            m_StatusText += "Maximum translation: +/-" + boost::lexical_cast<std::string>(m_MaxTranslation) + "mm\n";
        }
        else
        {
            m_StatusText += "Adding linear motion artifacts:\n";
            m_StatusText += "Maximum rotation: " + boost::lexical_cast<std::string>(m_MaxRotation) + "°\n";
            m_StatusText += "Maximum translation: " + boost::lexical_cast<std::string>(m_MaxTranslation) + "mm\n";
        }
        MITK_INFO << "Adding motion artifacts";
        MITK_INFO << "Maximum rotation: " << m_MaxRotation;
        MITK_INFO << "Maxmimum translation: " << m_MaxTranslation;
    }
    maxVolume = 0;

    m_StatusText += "\n"+this->GetTime()+" > Generating signal of " + boost::lexical_cast<std::string>(m_FiberModels.size()) + " fiber compartments\n";
    MITK_INFO << "Generating signal of " << m_FiberModels.size() << " fiber compartments";
    boost::progress_display disp(numFibers*m_FiberModels.at(0)->GetNumGradients());

    ofstream logFile;
    logFile.open("fiberfox_motion.log");
    logFile << "0 rotation: 0,0,0; translation: 0,0,0\n";

    // get transform for motion artifacts
    FiberBundleType fiberBundleTransformed = fiberBundle;
    VectorType rotation = m_MaxRotation/m_FiberModels.at(0)->GetNumGradients();
    VectorType translation = m_MaxTranslation/m_FiberModels.at(0)->GetNumGradients();

    // creat image to hold transformed mask (motion artifact)
    ItkUcharImgType::Pointer tempTissueMask = ItkUcharImgType::New();
    itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
    duplicator->SetInputImage(m_TissueMask);
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
    upsampler->SetInput(m_TissueMask);
    upsampler->SetOutputParametersFromImage(m_TissueMask);
    upsampler->SetSize(upsampledImageRegion.GetSize());
    upsampler->SetOutputSpacing(upsampledSpacing);
    upsampler->SetOutputOrigin(upsampledOrigin);
    itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType>::Pointer nn_interpolator
            = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType>::New();
    upsampler->SetInterpolator(nn_interpolator);
    upsampler->Update();
    upsampledTissueMask = upsampler->GetOutput();

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    unsigned int lastTick = 0;

    for (int g=0; g<m_FiberModels.at(0)->GetNumGradients(); g++)
    {
        vtkPolyData* fiberPolyData = fiberBundleTransformed->GetFiberPolyData();

        ItkDoubleImgType::Pointer intraAxonalVolume = ItkDoubleImgType::New();
        intraAxonalVolume->SetSpacing( m_UpsampledSpacing );
        intraAxonalVolume->SetOrigin( m_UpsampledOrigin );
        intraAxonalVolume->SetDirection( m_DirectionMatrix );
        intraAxonalVolume->SetLargestPossibleRegion( m_UpsampledImageRegion );
        intraAxonalVolume->SetBufferedRegion( m_UpsampledImageRegion );
        intraAxonalVolume->SetRequestedRegion( m_UpsampledImageRegion );
        intraAxonalVolume->Allocate();
        intraAxonalVolume->FillBuffer(0);

        // generate fiber signal
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

                if (!m_UseInterpolation)    // use nearest neighbour interpolation
                {
                    if (!tempTissueMask->GetLargestPossibleRegion().IsInside(idx) || tempTissueMask->GetPixel(idx)<=0)
                        continue;

                    // generate signal for each fiber compartment
                    for (unsigned int k=0; k<m_FiberModels.size(); k++)
                    {
                        DoubleDwiType::Pointer doubleDwi = compartments.at(k);
                        m_FiberModels[k]->SetFiberDirection(dir);
                        DoubleDwiType::PixelType pix = doubleDwi->GetPixel(idx);
                        pix[g] += segmentVolume*m_FiberModels[k]->SimulateMeasurement(g);

                        if (pix[g]!=pix[g])
                        {
                            std::cout << "pix[g] " << pix[g] << std::endl;
                            std::cout << "dir " << dir << std::endl;
                            std::cout << "segmentVolume " << segmentVolume << std::endl;
                            std::cout << "m_FiberModels[k]->SimulateMeasurement(g) " << m_FiberModels[k]->SimulateMeasurement(g) << std::endl;
                        }

                        doubleDwi->SetPixel(idx, pix );

                        double vol = intraAxonalVolume->GetPixel(idx) + segmentVolume;
                        intraAxonalVolume->SetPixel(idx, vol );

                        if (g==0 && vol>maxVolume)
                            maxVolume = vol;
                    }
                    continue;
                }

                double frac_x = contIndex[0] - idx[0]; double frac_y = contIndex[1] - idx[1]; double frac_z = contIndex[2] - idx[2];
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
                            if (!tempTissueMask->GetLargestPossibleRegion().IsInside(newIdx) || tempTissueMask->GetPixel(newIdx)<=0)
                                continue;

                            // generate signal for each fiber compartment
                            for (unsigned int k=0; k<m_FiberModels.size(); k++)
                            {
                                DoubleDwiType::Pointer doubleDwi = compartments.at(k);
                                m_FiberModels[k]->SetFiberDirection(dir);
                                DoubleDwiType::PixelType pix = doubleDwi->GetPixel(newIdx);
                                pix[g] += segmentVolume*frac*m_FiberModels[k]->SimulateMeasurement(g);
                                doubleDwi->SetPixel(newIdx, pix );

                                double vol = intraAxonalVolume->GetPixel(idx) + segmentVolume;
                                intraAxonalVolume->SetPixel(idx, vol );

                                if (g==0 && vol>maxVolume)
                                    maxVolume = vol;
                            }
                        }
                    }
                }
            }
            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (int tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;
        }

        // generate non-fiber signal
        ImageRegionIterator<ItkUcharImgType> it3(tempTissueMask, tempTissueMask->GetLargestPossibleRegion());
        double fact = 1;
        if (m_FiberRadius<0.0001)
            fact = voxelVolume/maxVolume;
        while(!it3.IsAtEnd())
        {
            if (it3.Get()>0)
            {
                DoubleDwiType::IndexType index = it3.GetIndex();

                // get fiber volume fraction
                DoubleDwiType::Pointer fiberDwi = compartments.at(0);
                DoubleDwiType::PixelType fiberPix = fiberDwi->GetPixel(index); // intra axonal compartment
                if (fact>1) // auto scale intra-axonal if no fiber radius is specified
                {
                    fiberPix[g] *= fact;
                    fiberDwi->SetPixel(index, fiberPix);
                }
                double f = intraAxonalVolume->GetPixel(index)*fact;

                if (f>voxelVolume || (f>0.0 && m_EnforcePureFiberVoxels) )  // more fiber than space in voxel?
                {
                    fiberPix[g] *= voxelVolume/f;
                    fiberDwi->SetPixel(index, fiberPix);
                    m_VolumeFractions.at(0)->SetPixel(index, 1);
                }
                else
                {
                    m_VolumeFractions.at(0)->SetPixel(index, f/voxelVolume);

                    double nonf = voxelVolume-f;    // non-fiber volume
                    double inter = 0;
                    if (m_FiberModels.size()>1)
                        inter = nonf * f/voxelVolume;   // inter-axonal fraction of non fiber compartment scales linearly with f
                    double other = nonf - inter;        // rest of compartment
                    double singleinter = inter/(m_FiberModels.size()-1);

                    // adjust non-fiber and intra-axonal signal
                    for (unsigned int i=1; i<m_FiberModels.size(); i++)
                    {
                        DoubleDwiType::Pointer doubleDwi = compartments.at(i);
                        DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                        if (f>0)
                            pix[g] /= f;
                        pix[g] *= singleinter;
                        doubleDwi->SetPixel(index, pix);
                        m_VolumeFractions.at(i)->SetPixel(index, singleinter/voxelVolume);
                    }
                    for (unsigned int i=0; i<m_NonFiberModels.size(); i++)
                    {
                        DoubleDwiType::Pointer doubleDwi = compartments.at(i+m_FiberModels.size());
                        DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
                        //                        if (dynamic_cast< mitk::AstroStickModel<double>* >(m_NonFiberModels.at(i)))
                        //                        {
                        //                            mitk::AstroStickModel<double>* model = dynamic_cast< mitk::AstroStickModel<double>* >(m_NonFiberModels.at(i));
                        //                            model->SetSeed(8111984);
                        //                        }
                        pix[g] += m_NonFiberModels[i]->SimulateMeasurement(g)*other*m_NonFiberModels[i]->GetWeight();
                        doubleDwi->SetPixel(index, pix);
                        m_VolumeFractions.at(i+m_FiberModels.size())->SetPixel(index, other/voxelVolume*m_NonFiberModels[i]->GetWeight());
                    }
                }
            }
            ++it3;
        }

        // move fibers
        if (m_AddMotionArtifact)
        {
            if (m_RandomMotion)
            {
                fiberBundleTransformed = fiberBundle->GetDeepCopy();
                rotation[0] = m_RandGen->GetVariateWithClosedRange(m_MaxRotation[0]*2)-m_MaxRotation[0];
                rotation[1] = m_RandGen->GetVariateWithClosedRange(m_MaxRotation[1]*2)-m_MaxRotation[1];
                rotation[2] = m_RandGen->GetVariateWithClosedRange(m_MaxRotation[2]*2)-m_MaxRotation[2];
                translation[0] = m_RandGen->GetVariateWithClosedRange(m_MaxTranslation[0]*2)-m_MaxTranslation[0];
                translation[1] = m_RandGen->GetVariateWithClosedRange(m_MaxTranslation[1]*2)-m_MaxTranslation[1];
                translation[2] = m_RandGen->GetVariateWithClosedRange(m_MaxTranslation[2]*2)-m_MaxTranslation[2];
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
                    if (m_RandomMotion)
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
            logFile << g+1 << " rotation:" << rotation[0] << "," << rotation[1] << "," << rotation[2] << ";";
            logFile << " translation:" << translation[0] << "," << translation[1] << "," << translation[2] << "\n";
            fiberBundleTransformed->TransformFibers(rotation[0],rotation[1],rotation[2],translation[0],translation[1],translation[2]);
        }
    }
    logFile.close();
    m_StatusText += "\n\n";
    if (this->GetAbortGenerateData())
    {
        m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
        return;
    }

    // do k-space stuff
    DoubleDwiType::Pointer doubleOutImage;
    if (m_Spikes>0 || m_FrequencyMap.IsNotNull() || m_kOffset>0 || m_SimulateRelaxation || m_SimulateEddyCurrents || m_AddGibbsRinging || m_Wrap<1.0)
    {
        m_StatusText += this->GetTime()+" > Adjusting complex signal\n";
        MITK_INFO << "Adjusting complex signal";
        doubleOutImage = DoKspaceStuff(compartments);
        m_SignalScale = 1;
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
    unsigned int window = 0;
    unsigned int min = itk::NumericTraits<unsigned int>::max();
    ImageRegionIterator<OutputImageType> it4 (outImage, outImage->GetLargestPossibleRegion());
    DoubleDwiType::PixelType signal; signal.SetSize(m_FiberModels[0]->GetNumGradients());
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
        for (int tick = 0; tick<(newTick-lastTick); tick++)
            m_StatusText += "*";
        lastTick = newTick;

        typename OutputImageType::IndexType index = it4.GetIndex();
        signal = doubleOutImage->GetPixel(index)*m_SignalScale;

        if (m_NoiseModel!=NULL)
        {
            DoubleDwiType::PixelType accu = signal; accu.Fill(0.0);
            for (unsigned int i=0; i<m_NumberOfRepetitions; i++)
            {
                DoubleDwiType::PixelType temp = signal;
                m_NoiseModel->AddNoise(temp);
                accu += temp;
            }
            signal = accu/m_NumberOfRepetitions;
        }

        for (unsigned int i=0; i<signal.Size(); i++)
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

    m_StatusText += "\n\n";
    m_StatusText += "Finished simulation\n";
    m_StatusText += "Simulation time: "+GetTime();
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
std::string TractsToDWIImageFilter< PixelType >::GetTime()
{
    unsigned long total = (double)(clock() - m_StartTime)/CLOCKS_PER_SEC;
    unsigned long hours = total/3600;
    unsigned long minutes = (total%3600)/60;
    unsigned long seconds = total%60;
    std::string out = "";
    out.append(boost::lexical_cast<std::string>(hours));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(minutes));
    out.append(":");
    out.append(boost::lexical_cast<std::string>(seconds));
    return out;
}

}
