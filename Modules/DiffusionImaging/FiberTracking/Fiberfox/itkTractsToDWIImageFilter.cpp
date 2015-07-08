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
#include <itkInvertIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkExtractImageFilter.h>

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
TractsToDWIImageFilter< PixelType >::DoubleDwiType::Pointer TractsToDWIImageFilter< PixelType >::SimulateKspaceAcquisition( std::vector< DoubleDwiType::Pointer >& images )
{
    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    // create slice object
    ImageRegion<2> sliceRegion;
    sliceRegion.SetSize(0, m_WorkingImageRegion.GetSize()[0]);
    sliceRegion.SetSize(1, m_WorkingImageRegion.GetSize()[1]);
    Vector< double, 2 > sliceSpacing;
    sliceSpacing[0] = m_WorkingSpacing[0];
    sliceSpacing[1] = m_WorkingSpacing[1];

    DoubleDwiType::PixelType nullPix; nullPix.SetSize(images.at(0)->GetVectorLength()); nullPix.Fill(0.0);
    DoubleDwiType::Pointer magnitudeDwiImage = DoubleDwiType::New();
    magnitudeDwiImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    magnitudeDwiImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
    magnitudeDwiImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    magnitudeDwiImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    magnitudeDwiImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    magnitudeDwiImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    magnitudeDwiImage->SetVectorLength( images.at(0)->GetVectorLength() );
    magnitudeDwiImage->Allocate();
    magnitudeDwiImage->FillBuffer(nullPix);

    m_PhaseImage = DoubleDwiType::New();
    m_PhaseImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    m_PhaseImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
    m_PhaseImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    m_PhaseImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_PhaseImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_PhaseImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_PhaseImage->SetVectorLength( images.at(0)->GetVectorLength() );
    m_PhaseImage->Allocate();
    m_PhaseImage->FillBuffer(nullPix);

    m_KspaceImage = DoubleDwiType::New();
    m_KspaceImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    m_KspaceImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
    m_KspaceImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    m_KspaceImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_KspaceImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_KspaceImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_KspaceImage->SetVectorLength( m_Parameters.m_SignalGen.m_NumberOfCoils );
    m_KspaceImage->Allocate();
    m_KspaceImage->FillBuffer(nullPix);

    std::vector< unsigned int > spikeVolume;
    for (unsigned int i=0; i<m_Parameters.m_SignalGen.m_Spikes; i++)
        spikeVolume.push_back(m_RandGen->GetIntegerVariate()%(images.at(0)->GetVectorLength()));
    std::sort (spikeVolume.begin(), spikeVolume.end());
    std::reverse (spikeVolume.begin(), spikeVolume.end());

    // calculate coil positions
    double a = m_Parameters.m_SignalGen.m_ImageRegion.GetSize(0)*m_Parameters.m_SignalGen.m_ImageSpacing[0];
    double b = m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1)*m_Parameters.m_SignalGen.m_ImageSpacing[1];
    double c = m_Parameters.m_SignalGen.m_ImageRegion.GetSize(2)*m_Parameters.m_SignalGen.m_ImageSpacing[2];
    double diagonal = sqrt(a*a+b*b)/1000;   // image diagonal in m

    m_CoilPointset = mitk::PointSet::New();
    std::vector< itk::Vector<double, 3> > coilPositions;
    itk::Vector<double, 3> pos; pos.Fill(0.0); pos[1] = -diagonal/2;
    itk::Vector<double, 3> center;
    center[0] = a/2-m_Parameters.m_SignalGen.m_ImageSpacing[0]/2;
    center[1] = b/2-m_Parameters.m_SignalGen.m_ImageSpacing[2]/2;
    center[2] = c/2-m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;
    for (int c=0; c<m_Parameters.m_SignalGen.m_NumberOfCoils; c++)
    {
        coilPositions.push_back(pos);
        m_CoilPointset->InsertPoint(c, pos*1000 + m_Parameters.m_SignalGen.m_ImageOrigin.GetVectorFromOrigin() + center );

        double rz = 360.0/m_Parameters.m_SignalGen.m_NumberOfCoils * M_PI/180;
        vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
        rotZ[0][0] = cos(rz);
        rotZ[1][1] = rotZ[0][0];
        rotZ[0][1] = -sin(rz);
        rotZ[1][0] = -rotZ[0][1];

        pos.SetVnlVector(rotZ*pos.GetVnlVector());
    }

    m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
    m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
    unsigned long lastTick = 0;

    boost::progress_display disp(images.at(0)->GetVectorLength()*images.at(0)->GetLargestPossibleRegion().GetSize(2));
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
            std::vector< double > t1Vector;

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
                    }

                compartmentSlices.push_back(slice);
                t2Vector.push_back(signalModel->GetT2());
                t1Vector.push_back(signalModel->GetT1());
            }

            int numSpikes = 0;
            while (!spikeSlice.empty() && spikeSlice.back()==z)
            {
                numSpikes++;
                spikeSlice.pop_back();
            }
            int spikeCoil = m_RandGen->GetIntegerVariate()%m_Parameters.m_SignalGen.m_NumberOfCoils;

            if (this->GetAbortGenerateData())
                return NULL;

#pragma omp parallel for
            for (int c=0; c<m_Parameters.m_SignalGen.m_NumberOfCoils; c++)
            {
                // create k-sapce (inverse fourier transform slices)
                itk::KspaceImageFilter< SliceType::PixelType >::Pointer idft = itk::KspaceImageFilter< SliceType::PixelType >::New();
                idft->SetCompartmentImages(compartmentSlices);
                idft->SetT2(t2Vector);
                idft->SetT1(t1Vector);
                idft->SetUseConstantRandSeed(m_UseConstantRandSeed);
                idft->SetParameters(&m_Parameters);
                idft->SetZ((double)z-(double)(images.at(0)->GetLargestPossibleRegion().GetSize(2)-images.at(0)->GetLargestPossibleRegion().GetSize(2)%2)/2.0);
                idft->SetZidx(z);
                idft->SetCoilPosition(coilPositions.at(c));
                idft->SetFiberBundle(m_FiberBundleWorkingCopy);
                idft->SetTranslation(m_Translations.at(g));
                idft->SetRotation(m_Rotations.at(g));
                idft->SetDiffusionGradientDirection(m_Parameters.m_SignalGen.GetGradientDirection(g));
                if (c==spikeCoil)
                    idft->SetSpikesPerSlice(numSpikes);
//                idft->SetNumberOfThreads(1);
                idft->Update();

                ComplexSliceType::Pointer fSlice;
                fSlice = idft->GetOutput();

                // fourier transform slice
                ComplexSliceType::Pointer newSlice;
                itk::DftImageFilter< SliceType::PixelType >::Pointer dft = itk::DftImageFilter< SliceType::PixelType >::New();
                dft->SetInput(fSlice);
                dft->SetParameters(m_Parameters);
                dft->Update();
                newSlice = dft->GetOutput();

                // put slice back into channel g
                for (unsigned int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
                    for (unsigned int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        ComplexSliceType::IndexType index2D; index2D[0]=x; index2D[1]=y;

                        ComplexSliceType::PixelType cPix = newSlice->GetPixel(index2D);
                        double magn = sqrt(cPix.real()*cPix.real()+cPix.imag()*cPix.imag());
                        double phase = 0;
                        if (cPix.real()!=0)
                            phase = atan( cPix.imag()/cPix.real() );


                        DoubleDwiType::PixelType dwiPix = magnitudeDwiImage->GetPixel(index3D);
                        DoubleDwiType::PixelType phasePix = m_PhaseImage->GetPixel(index3D);

                        if (m_Parameters.m_SignalGen.m_NumberOfCoils>1)
                        {
                            dwiPix[g] += magn*magn;
                            phasePix[g] += phase*phase;
                        }
                        else
                        {
                            dwiPix[g] = magn;
                            phasePix[g] = phase;
                        }

#pragma omp critical
                        {
                            magnitudeDwiImage->SetPixel(index3D, dwiPix);
                            m_PhaseImage->SetPixel(index3D, phasePix);

                            // k-space image
                            if (g==0)
                            {
                                DoubleDwiType::PixelType kspacePix = m_KspaceImage->GetPixel(index3D);
                                kspacePix[c] = idft->GetKSpaceImage()->GetPixel(index2D);
                                m_KspaceImage->SetPixel(index3D, kspacePix);
                            }
                        }
                    }
            }

            if (m_Parameters.m_SignalGen.m_NumberOfCoils>1)
            {
#ifdef WIN32
#pragma omp parallel for
#else
#pragma omp parallel for collapse(2)
#endif
                for (int y=0; y<magnitudeDwiImage->GetLargestPossibleRegion().GetSize(1); y++)
                    for (int x=0; x<magnitudeDwiImage->GetLargestPossibleRegion().GetSize(0); x++)
                    {
                        DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
                        DoubleDwiType::PixelType magPix = magnitudeDwiImage->GetPixel(index3D);
                        magPix[g] = sqrt(magPix[g]/m_Parameters.m_SignalGen.m_NumberOfCoils);

                        DoubleDwiType::PixelType phasePix = m_PhaseImage->GetPixel(index3D);
                        phasePix[g] = sqrt(phasePix[g]/m_Parameters.m_SignalGen.m_NumberOfCoils);

#pragma omp critical
                        {
                            magnitudeDwiImage->SetPixel(index3D, magPix);
                            m_PhaseImage->SetPixel(index3D, phasePix);
                        }
                    }
            }

            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned long tick = 0; tick<(newTick-lastTick); tick++)
                m_StatusText += "*";
            lastTick = newTick;
        }
    }
    m_StatusText += "\n\n";
    return magnitudeDwiImage;
}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::ItkDoubleImgType::Pointer TractsToDWIImageFilter< PixelType >::NormalizeInsideMask(ItkDoubleImgType::Pointer image)
{
    double max = itk::NumericTraits< double >::min();
    double min = itk::NumericTraits< double >::max();

    itk::ImageRegionIterator< ItkDoubleImgType > it(image, image->GetLargestPossibleRegion());
    while(!it.IsAtEnd())
    {
        if (m_Parameters.m_SignalGen.m_MaskImage.IsNotNull() && m_Parameters.m_SignalGen.m_MaskImage->GetPixel(it.GetIndex())<=0)
        {
            it.Set(0.0);
            ++it;
            continue;
        }
        //        if (it.Get()>900)
        //            it.Set(900);

        if (it.Get()>max)
            max = it.Get();
        if (it.Get()<min)
            min = it.Get();
        ++it;
    }
    typename itk::ShiftScaleImageFilter< ItkDoubleImgType, ItkDoubleImgType >::Pointer scaler = itk::ShiftScaleImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
    scaler->SetInput(image);
    scaler->SetShift(-min);
    scaler->SetScale(1.0/(max-min));
    scaler->Update();
    return scaler->GetOutput();
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::CheckVolumeFractionImages()
{
    m_UseRelativeNonFiberVolumeFractions = false;

    // check for fiber volume fraction maps
    int fibVolImages = 0;
    for (int i=0; i<m_Parameters.m_FiberModelList.size(); i++)
        if (m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage().IsNotNull())
        {
            m_StatusText += "Using volume fraction map for fiber compartment " + boost::lexical_cast<std::string>(i+1) + "\n";
            MITK_INFO << "Using volume fraction map for fiber compartment " + boost::lexical_cast<std::string>(i+1);
            fibVolImages++;
        }

    // check for non-fiber volume fraction maps
    int nonfibVolImages = 0;
    for (int i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
        if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage().IsNotNull())
        {
            m_StatusText += "Using volume fraction map for non-fiber compartment " + boost::lexical_cast<std::string>(i+1) + "\n";
            MITK_INFO << "Using volume fraction map for non-fiber compartment " + boost::lexical_cast<std::string>(i+1);
            nonfibVolImages++;
        }

    // not all fiber compartments are using volume fraction maps --> non-fiber volume fractions are assumed to be relative to the non-fiber volume and not absolute voxel-volume fractions.
    // this means if two non-fiber compartments are used but only one of them has an associated volume fraction map, the repesctive other volume fraction map can be determined as inverse (1-val) of the present volume fraction map-
    if ( fibVolImages<m_Parameters.m_FiberModelList.size() && nonfibVolImages==1 && m_Parameters.m_NonFiberModelList.size()==2)
    {
        m_StatusText += "Calculating missing non-fiber volume fraction image by inverting the other.\nAssuming non-fiber volume fraction images to contain values relative to the remaining non-fiber volume, not absolute values.\n";
        MITK_INFO << "Calculating missing non-fiber volume fraction image by inverting the other.\nAssuming non-fiber volume fraction images to contain values relative to the remaining non-fiber volume, not absolute values.";

        itk::InvertIntensityImageFilter< ItkDoubleImgType, ItkDoubleImgType >::Pointer inverter = itk::InvertIntensityImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
        inverter->SetMaximum(1.0);
        if ( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage().IsNull() && m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage().IsNotNull() )
        {
            //            m_Parameters.m_NonFiberModelList[1]->SetVolumeFractionImage( NormalizeInsideMask( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage() ) );
            inverter->SetInput( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage() );
            inverter->Update();
            m_Parameters.m_NonFiberModelList[0]->SetVolumeFractionImage(inverter->GetOutput());
        }
        else if ( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage().IsNull() && m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage().IsNotNull() )
        {
            //            m_Parameters.m_NonFiberModelList[0]->SetVolumeFractionImage( NormalizeInsideMask( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage() ) );
            inverter->SetInput( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage() );
            inverter->Update();
            m_Parameters.m_NonFiberModelList[1]->SetVolumeFractionImage(inverter->GetOutput());
        }
        else
        {
            itkExceptionMacro("Something went wrong in automatically calculating the missing non-fiber volume fraction image! Did you use two non fiber compartments but only one volume fraction image? Then it should work and this error is really strange.");
        }
        m_UseRelativeNonFiberVolumeFractions = true;

        nonfibVolImages++;
    }

    // Up to two fiber compartments are allowed without volume fraction maps since the volume fractions can then be determined automatically
    if (m_Parameters.m_FiberModelList.size()>2 && fibVolImages!=m_Parameters.m_FiberModelList.size())
        itkExceptionMacro("More than two fiber compartment selected but no corresponding volume fraction maps set!");

    // One non-fiber compartment is allowed without volume fraction map since the volume fraction can then be determined automatically
    if (m_Parameters.m_NonFiberModelList.size()>1 && nonfibVolImages!=m_Parameters.m_NonFiberModelList.size())
        itkExceptionMacro("More than one non-fiber compartment selected but no volume fraction maps set!");

    if (fibVolImages<m_Parameters.m_FiberModelList.size() && nonfibVolImages>0)
    {
        m_StatusText += "Not all fiber compartments are using an associated volume fraction image.\nAssuming non-fiber volume fraction images to contain values relative to the remaining non-fiber volume, not absolute values.\n";
        MITK_INFO << "Not all fiber compartments are using an associated volume fraction image.\nAssuming non-fiber volume fraction images to contain values relative to the remaining non-fiber volume, not absolute values.";
        m_UseRelativeNonFiberVolumeFractions = true;

        //        itk::ImageFileWriter<ItkDoubleImgType>::Pointer wr = itk::ImageFileWriter<ItkDoubleImgType>::New();
        //        wr->SetInput(m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage());
        //        wr->SetFileName("/local/volumefraction.nrrd");
        //        wr->Update();
    }

    // initialize the images that store the output volume fraction of each compartment
    m_VolumeFractions.clear();
    for (int i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
    {
        ItkDoubleImgType::Pointer doubleImg = ItkDoubleImgType::New();
        doubleImg->SetSpacing( m_WorkingSpacing );
        doubleImg->SetOrigin( m_WorkingOrigin );
        doubleImg->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        doubleImg->SetLargestPossibleRegion( m_WorkingImageRegion );
        doubleImg->SetBufferedRegion( m_WorkingImageRegion );
        doubleImg->SetRequestedRegion( m_WorkingImageRegion );
        doubleImg->Allocate();
        doubleImg->FillBuffer(0);
        m_VolumeFractions.push_back(doubleImg);
    }
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::InitializeData()
{
    // initialize output dwi image
    m_Parameters.m_SignalGen.m_CroppedRegion = m_Parameters.m_SignalGen.m_ImageRegion; m_Parameters.m_SignalGen.m_CroppedRegion.SetSize(1, m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1)*m_Parameters.m_SignalGen.m_CroppingFactor);
    itk::Point<double,3> shiftedOrigin = m_Parameters.m_SignalGen.m_ImageOrigin; shiftedOrigin[1] += (m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1)-m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1))*m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;
    m_OutputImage = OutputImageType::New();
    m_OutputImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    m_OutputImage->SetOrigin( shiftedOrigin );
    m_OutputImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    m_OutputImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_OutputImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_OutputImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    m_OutputImage->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
    m_OutputImage->Allocate();
    typename OutputImageType::PixelType temp;
    temp.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
    temp.Fill(0.0);
    m_OutputImage->FillBuffer(temp);

    // Apply in-plane upsampling for Gibbs ringing artifact
    double upsampling = 1;
    if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
        upsampling = 2;
    m_WorkingSpacing = m_Parameters.m_SignalGen.m_ImageSpacing;
    m_WorkingSpacing[0] /= upsampling;
    m_WorkingSpacing[1] /= upsampling;
    m_WorkingImageRegion = m_Parameters.m_SignalGen.m_ImageRegion;
    m_WorkingImageRegion.SetSize(0, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[0]*upsampling);
    m_WorkingImageRegion.SetSize(1, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[1]*upsampling);
    m_WorkingOrigin = m_Parameters.m_SignalGen.m_ImageOrigin;
    m_WorkingOrigin[0] -= m_Parameters.m_SignalGen.m_ImageSpacing[0]/2; m_WorkingOrigin[0] += m_WorkingSpacing[0]/2;
    m_WorkingOrigin[1] -= m_Parameters.m_SignalGen.m_ImageSpacing[1]/2; m_WorkingOrigin[1] += m_WorkingSpacing[1]/2;
    m_WorkingOrigin[2] -= m_Parameters.m_SignalGen.m_ImageSpacing[2]/2; m_WorkingOrigin[2] += m_WorkingSpacing[2]/2;
    m_VoxelVolume = m_WorkingSpacing[0]*m_WorkingSpacing[1]*m_WorkingSpacing[2];

    // generate double images to store the individual compartment signals
    m_CompartmentImages.clear();
    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();
    for (int i=0; i<numFiberCompartments+numNonFiberCompartments; i++)
    {
        DoubleDwiType::Pointer doubleDwi = DoubleDwiType::New();
        doubleDwi->SetSpacing( m_WorkingSpacing );
        doubleDwi->SetOrigin( m_WorkingOrigin );
        doubleDwi->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        doubleDwi->SetLargestPossibleRegion( m_WorkingImageRegion );
        doubleDwi->SetBufferedRegion( m_WorkingImageRegion );
        doubleDwi->SetRequestedRegion( m_WorkingImageRegion );
        doubleDwi->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
        doubleDwi->Allocate();
        DoubleDwiType::PixelType pix;
        pix.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
        pix.Fill(0.0);
        doubleDwi->FillBuffer(pix);
        m_CompartmentImages.push_back(doubleDwi);
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
            resampler->SetSize(m_WorkingImageRegion.GetSize());
            resampler->SetOutputSpacing(m_WorkingSpacing);
            resampler->SetOutputOrigin(m_WorkingOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_SignalGen.m_MaskImage = resampler->GetOutput();
        }
        // resample frequency map
        if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull())
        {
            itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::Pointer resampler = itk::ResampleImageFilter<ItkDoubleImgType, ItkDoubleImgType>::New();
            resampler->SetInput(m_Parameters.m_SignalGen.m_FrequencyMap);
            resampler->SetOutputParametersFromImage(m_Parameters.m_SignalGen.m_FrequencyMap);
            resampler->SetSize(m_WorkingImageRegion.GetSize());
            resampler->SetOutputSpacing(m_WorkingSpacing);
            resampler->SetOutputOrigin(m_WorkingOrigin);
            itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::Pointer nn_interpolator
                    = itk::NearestNeighborInterpolateImageFunction<ItkDoubleImgType, double>::New();
            resampler->SetInterpolator(nn_interpolator);
            resampler->Update();
            m_Parameters.m_SignalGen.m_FrequencyMap = resampler->GetOutput();
        }
    }

    m_MaskImageSet = true;
    if (m_Parameters.m_SignalGen.m_MaskImage.IsNull())
    {
        // no input tissue mask is set -> create default
        m_StatusText += "No tissue mask set\n";
        MITK_INFO << "No tissue mask set";
        m_Parameters.m_SignalGen.m_MaskImage = ItkUcharImgType::New();
        m_Parameters.m_SignalGen.m_MaskImage->SetSpacing( m_WorkingSpacing );
        m_Parameters.m_SignalGen.m_MaskImage->SetOrigin( m_WorkingOrigin );
        m_Parameters.m_SignalGen.m_MaskImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        m_Parameters.m_SignalGen.m_MaskImage->SetLargestPossibleRegion( m_WorkingImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->SetBufferedRegion( m_WorkingImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->SetRequestedRegion( m_WorkingImageRegion );
        m_Parameters.m_SignalGen.m_MaskImage->Allocate();
        m_Parameters.m_SignalGen.m_MaskImage->FillBuffer(100);
        m_MaskImageSet = false;
    }
    else
    {
        if (m_Parameters.m_SignalGen.m_MaskImage->GetLargestPossibleRegion()!=m_WorkingImageRegion)
            itkExceptionMacro("Mask image and specified DWI geometry are not matching!");

        m_StatusText += "Using tissue mask\n";
        MITK_INFO << "Using tissue mask";
    }

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

        m_MotionLogfile.open((filePath+fileName).c_str());
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
        MITK_INFO << "Motion logfile: " << filePath << fileName;
    }
    if ( m_Parameters.m_SignalGen.m_MotionVolumes.empty() )
    {
        // no motion in first volume
        m_Parameters.m_SignalGen.m_MotionVolumes.push_back(false);

        // motion in all other volumes
        while ( m_Parameters.m_SignalGen.m_MotionVolumes.size()<m_Parameters.m_SignalGen.GetNumVolumes() )
            m_Parameters.m_SignalGen.m_MotionVolumes.push_back(true);
    }
    // we need to know for every volume if there is motion. if this information is missing, then set corresponding fal to false
    while ( m_Parameters.m_SignalGen.m_MotionVolumes.size()<m_Parameters.m_SignalGen.GetNumVolumes() )
        m_Parameters.m_SignalGen.m_MotionVolumes.push_back(false);
    m_NumMotionVolumes = 0;
    for (int i=0; i<m_Parameters.m_SignalGen.GetNumVolumes(); i++)
        if (m_Parameters.m_SignalGen.m_MotionVolumes[i])
            m_NumMotionVolumes++;
    m_MotionCounter = 0;

    // creat image to hold transformed mask (motion artifact)
    m_TransformedMaskImage = ItkUcharImgType::New();
    itk::ImageDuplicator<ItkUcharImgType>::Pointer duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
    duplicator->SetInputImage(m_Parameters.m_SignalGen.m_MaskImage);
    duplicator->Update();
    m_TransformedMaskImage = duplicator->GetOutput();

    // second upsampling needed for motion artifacts
    ImageRegion<3>      upsampledImageRegion = m_WorkingImageRegion;
    DoubleVectorType    upsampledSpacing = m_WorkingSpacing;
    upsampledSpacing[0] /= 4;
    upsampledSpacing[1] /= 4;
    upsampledSpacing[2] /= 4;
    upsampledImageRegion.SetSize(0, m_WorkingImageRegion.GetSize()[0]*4);
    upsampledImageRegion.SetSize(1, m_WorkingImageRegion.GetSize()[1]*4);
    upsampledImageRegion.SetSize(2, m_WorkingImageRegion.GetSize()[2]*4);
    itk::Point<double,3> upsampledOrigin = m_WorkingOrigin;
    upsampledOrigin[0] -= m_WorkingSpacing[0]/2; upsampledOrigin[0] += upsampledSpacing[0]/2;
    upsampledOrigin[1] -= m_WorkingSpacing[1]/2; upsampledOrigin[1] += upsampledSpacing[1]/2;
    upsampledOrigin[2] -= m_WorkingSpacing[2]/2; upsampledOrigin[2] += upsampledSpacing[2]/2;
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
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::InitializeFiberData()
{
    // resample fiber bundle for sufficient voxel coverage
    m_StatusText += "\n"+this->GetTime()+" > Resampling fibers ...\n";
    m_SegmentVolume = 0.0001;
    float minSpacing = 1;
    if(m_WorkingSpacing[0]<m_WorkingSpacing[1] && m_WorkingSpacing[0]<m_WorkingSpacing[2])
        minSpacing = m_WorkingSpacing[0];
    else if (m_WorkingSpacing[1] < m_WorkingSpacing[2])
        minSpacing = m_WorkingSpacing[1];
    else
        minSpacing = m_WorkingSpacing[2];
    m_FiberBundleWorkingCopy = m_FiberBundle->GetDeepCopy();    // working copy is needed because we need to resample the fibers but do not want to change the original bundle
    double volumeAccuracy = 10;
    m_FiberBundleWorkingCopy->ResampleSpline(minSpacing/volumeAccuracy);
    m_mmRadius = m_Parameters.m_SignalGen.m_AxonRadius/1000;
    if (m_mmRadius>0)
        m_SegmentVolume = M_PI*m_mmRadius*m_mmRadius*minSpacing/volumeAccuracy;
    m_FiberBundleTransformed = m_FiberBundleWorkingCopy;    // a secon fiber bundle is needed to store the transformed version of the m_FiberBundleWorkingCopy
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
        itkExceptionMacro("No diffusion model for fiber compartments defined! At least one fiber compartment is necessary.");

    if (m_Parameters.m_NonFiberModelList.empty())
        itkExceptionMacro("No diffusion model for non-fiber compartments defined! At least one non-fiber compartment is necessary.");

    int baselineIndex = m_Parameters.m_SignalGen.GetFirstBaselineIndex();
    if (baselineIndex<0)
        itkExceptionMacro("No baseline index found!");

    if (!m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition)  // No upsampling of input image needed if no k-space simulation is performed
        m_Parameters.m_SignalGen.m_DoAddGibbsRinging = false;

    if (m_UseConstantRandSeed)  // always generate the same random numbers?
        m_RandGen->SetSeed(0);
    else
        m_RandGen->SetSeed();

    InitializeData();
    CheckVolumeFractionImages();
    InitializeFiberData();

    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();

    double maxVolume = 0;
    unsigned long lastTick = 0;
    int signalModelSeed = m_RandGen->GetIntegerVariate();

    m_StatusText += "\n"+this->GetTime()+" > Generating " + boost::lexical_cast<std::string>(numFiberCompartments+numNonFiberCompartments) + "-compartment diffusion-weighted signal.\n";
    MITK_INFO << "Generating " << numFiberCompartments+numNonFiberCompartments << "-compartment diffusion-weighted signal.";
    int numFibers = m_FiberBundleWorkingCopy->GetNumFibers();
    boost::progress_display disp(numFibers*m_Parameters.m_SignalGen.GetNumVolumes());

    switch (m_Parameters.m_SignalGen.m_DiffusionDirectionMode)
    {
    case(SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS):   // use fiber tangent directions to determine diffusion direction
    {
        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";

        for (unsigned int g=0; g<m_Parameters.m_SignalGen.GetNumVolumes(); g++)
        {
            // move fibers
            SimulateMotion(g);

            // Set signal model random generator seeds to get same configuration in each voxel
            for (int i=0; i<m_Parameters.m_FiberModelList.size(); i++)
                m_Parameters.m_FiberModelList.at(i)->SetSeed(signalModelSeed);
            for (int i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
                m_Parameters.m_NonFiberModelList.at(i)->SetSeed(signalModelSeed);

            // storing voxel-wise intra-axonal volume in mm³
            ItkDoubleImgType::Pointer intraAxonalVolumeImage = ItkDoubleImgType::New();
            intraAxonalVolumeImage->SetSpacing( m_WorkingSpacing );
            intraAxonalVolumeImage->SetOrigin( m_WorkingOrigin );
            intraAxonalVolumeImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
            intraAxonalVolumeImage->SetLargestPossibleRegion( m_WorkingImageRegion );
            intraAxonalVolumeImage->SetBufferedRegion( m_WorkingImageRegion );
            intraAxonalVolumeImage->SetRequestedRegion( m_WorkingImageRegion );
            intraAxonalVolumeImage->Allocate();
            intraAxonalVolumeImage->FillBuffer(0);
            maxVolume = 0;

            vtkPolyData* fiberPolyData = m_FiberBundleTransformed->GetFiberPolyData();
            // generate fiber signal (if there are any fiber models present)
            if (!m_Parameters.m_FiberModelList.empty())
                for( int i=0; i<numFibers; i++ )
                {
                    float fiberWeight = m_FiberBundleTransformed->GetFiberWeight(i);
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
                        m_TransformedMaskImage->TransformPhysicalPointToIndex(vertex, idx);
                        m_TransformedMaskImage->TransformPhysicalPointToContinuousIndex(vertex, contIndex);

                        if (!m_TransformedMaskImage->GetLargestPossibleRegion().IsInside(idx) || m_TransformedMaskImage->GetPixel(idx)<=0)
                            continue;

                        // generate signal for each fiber compartment
                        for (int k=0; k<numFiberCompartments; k++)
                        {
                            m_Parameters.m_FiberModelList[k]->SetFiberDirection(dir);
                            DoubleDwiType::PixelType pix = m_CompartmentImages.at(k)->GetPixel(idx);
                            pix[g] += fiberWeight*m_SegmentVolume*m_Parameters.m_FiberModelList[k]->SimulateMeasurement(g);

                            m_CompartmentImages.at(k)->SetPixel(idx, pix);
                        }

                        // update fiber volume image
                        double vol = intraAxonalVolumeImage->GetPixel(idx) + m_SegmentVolume*fiberWeight;
                        intraAxonalVolumeImage->SetPixel(idx, vol);
                        if (vol>maxVolume)  // we assume that the first volume is always unweighted!
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
            ImageRegionIterator<ItkUcharImgType> it3(m_TransformedMaskImage, m_TransformedMaskImage->GetLargestPossibleRegion());
            double fact = 1;    // density correction factor in mm³
            if (m_Parameters.m_SignalGen.m_AxonRadius<0.0001 || maxVolume>m_VoxelVolume)    // the fullest voxel is always completely full
                fact = m_VoxelVolume/maxVolume;
            while(!it3.IsAtEnd())
            {
                if (it3.Get()>0)
                {
                    DoubleDwiType::IndexType index = it3.GetIndex();
                    itk::Point<double, 3> point;
                    m_TransformedMaskImage->TransformIndexToPhysicalPoint(index, point);
                    if (m_Parameters.m_SignalGen.m_DoAddMotion && g>=0 && m_Parameters.m_SignalGen.m_MotionVolumes[g])
                    {
                        if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
                            point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0],-m_Rotation[1],-m_Rotation[2],-m_Translation[0],-m_Translation[1],-m_Translation[2]);
                        else
                            point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0]*m_MotionCounter,-m_Rotation[1]*m_MotionCounter,-m_Rotation[2]*m_MotionCounter,-m_Translation[0]*m_MotionCounter,-m_Translation[1]*m_MotionCounter,-m_Translation[2]*m_MotionCounter);
                    }

                    double iAxVolume = intraAxonalVolumeImage->GetPixel(index);

                    // if volume fraction image is set use it, otherwise use scaling factor to obtain one full fiber voxel
                    double fact2 = fact;
                    if (m_Parameters.m_FiberModelList[0]->GetVolumeFractionImage()!=nullptr && iAxVolume>0.0001)
                    {
                        double val = InterpolateValue(point, m_Parameters.m_FiberModelList[0]->GetVolumeFractionImage());
                        if (val>=0)
                            fact2 = m_VoxelVolume*val/iAxVolume;
                    }

                    // adjust intra-axonal image value
                    for (int i=0; i<numFiberCompartments; i++)
                    {
                        DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                        pix[g] *= fact2;
                        m_CompartmentImages.at(i)->SetPixel(index, pix);
                    }

                    // simulate other compartments
                    SimulateExtraAxonalSignal(index, iAxVolume*fact2, g);
                }
                ++it3;
            }
        }
        break;
    }
    case (SignalGenerationParameters::MAIN_FIBER_DIRECTIONS): // use main fiber directions to determine voxel-wise diffusion directions   // NOT UP TO DATE ANYMORE! NEEDED? REMOVE?
    {
        typedef itk::Image< itk::Vector< float, 3>, 3 >                                 ItkDirectionImage3DType;
        typedef itk::VectorContainer< unsigned int, ItkDirectionImage3DType::Pointer >  ItkDirectionImageContainerType;

        // calculate main fiber directions
        itk::TractsToVectorImageFilter<float>::Pointer fOdfFilter = itk::TractsToVectorImageFilter<float>::New();
        fOdfFilter->SetFiberBundle(m_FiberBundleTransformed);
        fOdfFilter->SetMaskImage(m_TransformedMaskImage);
        fOdfFilter->SetAngularThreshold(cos(m_Parameters.m_SignalGen.m_FiberSeparationThreshold*M_PI/180.0));
        fOdfFilter->SetNormalizeVectors(false);
        fOdfFilter->SetUseWorkingCopy(true);
        fOdfFilter->SetSizeThreshold(0);
        fOdfFilter->SetMaxNumDirections(3);
        fOdfFilter->Update();
        ItkDirectionImageContainerType::Pointer directionImageContainer = fOdfFilter->GetDirectionImageContainer();

        // allocate image storing intra-axonal volume fraction information
        ItkDoubleImgType::Pointer intraAxonalVolumeImage = ItkDoubleImgType::New();
        intraAxonalVolumeImage->SetSpacing( m_WorkingSpacing );
        intraAxonalVolumeImage->SetOrigin( m_WorkingOrigin );
        intraAxonalVolumeImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        intraAxonalVolumeImage->SetLargestPossibleRegion( m_WorkingImageRegion );
        intraAxonalVolumeImage->SetBufferedRegion( m_WorkingImageRegion );
        intraAxonalVolumeImage->SetRequestedRegion( m_WorkingImageRegion );
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
        boost::progress_display disp(m_TransformedMaskImage->GetLargestPossibleRegion().GetNumberOfPixels()*m_Parameters.m_SignalGen.GetNumVolumes());

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
                fOdfFilter->SetMaskImage(m_TransformedMaskImage);
                fOdfFilter->Update();
                directionImageContainer = fOdfFilter->GetDirectionImageContainer();

                tdiFilter->SetFiberBundle(m_FiberBundleTransformed);
                tdiFilter->Update();
                intraAxonalVolumeImage = tdiFilter->GetOutput();
            }

            ImageRegionIterator< ItkUcharImgType > it(m_TransformedMaskImage, m_TransformedMaskImage->GetLargestPossibleRegion());
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
                    SimulateExtraAxonalSignal(it.GetIndex(), intraAxonalVolumeImage->GetPixel(it.GetIndex())*m_VoxelVolume, g);
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
    case (SignalGenerationParameters::RANDOM_DIRECTIONS):   // NOT UP TO DATE ANYMORE! NEEDED? REMOVE?
    {
        ItkUcharImgType::Pointer numDirectionsImage = ItkUcharImgType::New();
        numDirectionsImage->SetSpacing( m_WorkingSpacing );
        numDirectionsImage->SetOrigin( m_WorkingOrigin );
        numDirectionsImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
        numDirectionsImage->SetLargestPossibleRegion( m_WorkingImageRegion );
        numDirectionsImage->SetBufferedRegion( m_WorkingImageRegion );
        numDirectionsImage->SetRequestedRegion( m_WorkingImageRegion );
        numDirectionsImage->Allocate();
        numDirectionsImage->FillBuffer(0);
        double sepAngle = cos(m_Parameters.m_SignalGen.m_FiberSeparationThreshold*M_PI/180.0);

        m_StatusText += "0%   10   20   30   40   50   60   70   80   90   100%\n";
        m_StatusText += "|----|----|----|----|----|----|----|----|----|----|\n*";
        boost::progress_display disp(m_TransformedMaskImage->GetLargestPossibleRegion().GetNumberOfPixels());
        ImageRegionIterator<ItkUcharImgType> it(m_TransformedMaskImage, m_TransformedMaskImage->GetLargestPossibleRegion());
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

    if (m_MotionLogfile.is_open())
    {
        m_MotionLogfile << "DONE";
        m_MotionLogfile.close();
    }
    m_StatusText += "\n\n";
    if (this->GetAbortGenerateData())
    {
        m_StatusText += "\n"+this->GetTime()+" > Simulation aborted\n";
        return;
    }

    DoubleDwiType::Pointer doubleOutImage;
    double signalScale = m_Parameters.m_SignalGen.m_SignalScale;
    if ( m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition ) // do k-space stuff
    {
        m_StatusText += this->GetTime()+" > Simulating k-space acquisition using "+boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_NumberOfCoils)+" coil(s)\n";
        MITK_INFO << "Simulating k-space acquisition using " << m_Parameters.m_SignalGen.m_NumberOfCoils << " coil(s).";

        switch (m_Parameters.m_SignalGen.m_AcquisitionType)
        {
        case SignalGenerationParameters::SingleShotEpi:
            m_StatusText += "Acquisition type: single shot EPI\n";
            break;
        case SignalGenerationParameters::SpinEcho:
            m_StatusText += "Acquisition type: classic spin echo with cartesian k-space trajectory\n";
            break;
        default:
            m_StatusText += "Acquisition type: single shot EPI\n";
        }

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

        doubleOutImage = SimulateKspaceAcquisition(m_CompartmentImages);
        signalScale = 1; // already scaled in SimulateKspaceAcquisition()
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
    if (signalScale>1)
        m_StatusText += " Scaling signal\n";
    if (m_Parameters.m_NoiseModel)
        m_StatusText += " Adding noise\n";
    unsigned int window = 0;
    unsigned int min = itk::NumericTraits<unsigned int>::max();
    ImageRegionIterator<OutputImageType> it4 (m_OutputImage, m_OutputImage->GetLargestPossibleRegion());
    DoubleDwiType::PixelType signal; signal.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
    boost::progress_display disp2(m_OutputImage->GetLargestPossibleRegion().GetNumberOfPixels());

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
        signal = doubleOutImage->GetPixel(index)*signalScale;

        if (m_Parameters.m_NoiseModel)
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
    this->SetNthOutput(0, m_OutputImage);

    m_StatusText += "\n\n";
    m_StatusText += "Finished simulation\n";
    m_StatusText += "Simulation time: "+GetTime();
    m_TimeProbe.Stop();
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::SimulateMotion(int g)
{
    // is motion artifact enabled?
    // is the current volume g affected by motion?
    if (m_Parameters.m_SignalGen.m_DoAddMotion && m_Parameters.m_SignalGen.m_MotionVolumes[g] && g<m_Parameters.m_SignalGen.GetNumVolumes())
    {
        if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
        {
            m_FiberBundleTransformed = m_FiberBundleWorkingCopy->GetDeepCopy(); // either undo last transform or work on fresh copy of untransformed fibers
            m_Rotation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[0]*2)-m_Parameters.m_SignalGen.m_Rotation[0];
            m_Rotation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[1]*2)-m_Parameters.m_SignalGen.m_Rotation[1];
            m_Rotation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[2]*2)-m_Parameters.m_SignalGen.m_Rotation[2];
            m_Translation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[0]*2)-m_Parameters.m_SignalGen.m_Translation[0];
            m_Translation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[1]*2)-m_Parameters.m_SignalGen.m_Translation[1];
            m_Translation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[2]*2)-m_Parameters.m_SignalGen.m_Translation[2];
        }
        else
        {
            m_Rotation = m_Parameters.m_SignalGen.m_Rotation/m_NumMotionVolumes;
            m_Translation = m_Parameters.m_SignalGen.m_Translation/m_NumMotionVolumes;
            m_MotionCounter++;
        }

        // move mask image
        if (m_MaskImageSet)
        {
            ImageRegionIterator<ItkUcharImgType> maskIt(m_UpsampledMaskImage, m_UpsampledMaskImage->GetLargestPossibleRegion());
            m_TransformedMaskImage->FillBuffer(0);

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
                    point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), m_Rotation[0]*m_MotionCounter,m_Rotation[1]*m_MotionCounter,m_Rotation[2]*m_MotionCounter,m_Translation[0]*m_MotionCounter,m_Translation[1]*m_MotionCounter,m_Translation[2]*m_MotionCounter);

                m_TransformedMaskImage->TransformPhysicalPointToIndex(point, index);
                if (m_TransformedMaskImage->GetLargestPossibleRegion().IsInside(index))
                    m_TransformedMaskImage->SetPixel(index,100);
                ++maskIt;
            }
        }

        if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
        {
            m_Rotations.push_back(m_Rotation);
            m_Translations.push_back(m_Translation);
            if (m_MotionLogfile.is_open())
            {
                m_MotionLogfile << g << " rotation: " << m_Rotation[0] << "," << m_Rotation[1] << "," << m_Rotation[2] << ";";
                m_MotionLogfile << " translation: " << m_Translation[0] << "," << m_Translation[1] << "," << m_Translation[2] << "\n";
            }
        }
        else
        {
            m_Rotations.push_back(m_Rotation*m_MotionCounter);
            m_Translations.push_back(m_Translation*m_MotionCounter);
            if (m_MotionLogfile.is_open())
            {
                m_MotionLogfile << g << " rotation: " << m_Rotation[0]*m_MotionCounter << "," << m_Rotation[1]*m_MotionCounter << "," << m_Rotation[2]*m_MotionCounter << ";";
                m_MotionLogfile << " translation: " << m_Translation[0]*m_MotionCounter << "," << m_Translation[1]*m_MotionCounter << "," << m_Translation[2]*m_MotionCounter << "\n";
            }
        }

        m_FiberBundleTransformed->TransformFibers(m_Rotation[0],m_Rotation[1],m_Rotation[2],m_Translation[0],m_Translation[1],m_Translation[2]);
    }
    else
    {
        m_Rotation.Fill(0.0);
        m_Translation.Fill(0.0);
        m_Rotations.push_back(m_Rotation);
        m_Translations.push_back(m_Translation);
        m_MotionLogfile << g << " rotation: " << m_Rotation[0] << "," << m_Rotation[1] << "," << m_Rotation[2] << ";";
        m_MotionLogfile << " translation: " << m_Translation[0] << "," << m_Translation[1] << "," << m_Translation[2] << "\n";
    }
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::SimulateExtraAxonalSignal(ItkUcharImgType::IndexType index, double intraAxonalVolume, int g)
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
        if (g==0)
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
        if (g==0)
            m_VolumeFractions.at(0)->SetPixel(index, intraAxonalVolume/m_VoxelVolume);

        // get non-transformed point (remove headmotion tranformation)
        // this point can then be transformed to each of the original images, regardless of their geometry
        itk::Point<double, 3> point;
        m_TransformedMaskImage->TransformIndexToPhysicalPoint(index, point);
        if (m_Parameters.m_SignalGen.m_DoAddMotion && g>=0 && m_Parameters.m_SignalGen.m_MotionVolumes[g])
        {
            if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
                point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0],-m_Rotation[1],-m_Rotation[2],-m_Translation[0],-m_Translation[1],-m_Translation[2]);
            else
                point = m_FiberBundleWorkingCopy->TransformPoint(point.GetVnlVector(), -m_Rotation[0]*m_MotionCounter,-m_Rotation[1]*m_MotionCounter,-m_Rotation[2]*m_MotionCounter,-m_Translation[0]*m_MotionCounter,-m_Translation[1]*m_MotionCounter,-m_Translation[2]*m_MotionCounter);
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
                    double val = InterpolateValue(point, m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage());
                    if (val<0)
                        continue;
                    else
                        weight = val;
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
                pix[g] += m_Parameters.m_NonFiberModelList[maxVolumeIndex]->SimulateMeasurement(g)*m_VoxelVolume;
            else
                pix += m_Parameters.m_NonFiberModelList[maxVolumeIndex]->SimulateMeasurement()*m_VoxelVolume;
            doubleDwi->SetPixel(index, pix);
            if (g==0)
                m_VolumeFractions.at(maxVolumeIndex+numFiberCompartments)->SetPixel(index, 1);
        }
        else
        {
            double extraAxonalVolume = m_VoxelVolume-intraAxonalVolume;    // non-fiber volume
            if (extraAxonalVolume<0)
            {
                MITK_ERROR << "Coorupted intra-axonal signal voxel detected. Fiber volume larger voxel volume!";
                extraAxonalVolume = 0;
            }
            double interAxonalVolume = 0;
            if (numFiberCompartments>1)
                interAxonalVolume = extraAxonalVolume * intraAxonalVolume/m_VoxelVolume;   // inter-axonal fraction of non fiber compartment
            double other = extraAxonalVolume - interAxonalVolume;        // rest of compartment
            if (other<0)
            {
                MITK_ERROR << "Corrupted signal voxel detected. Fiber volume larger voxel volume!";
                other = 0;
            }

            // adjust non-fiber and intra-axonal signal
            for (int i=1; i<numFiberCompartments; i++)
            {
                double weight = interAxonalVolume;
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                if (intraAxonalVolume>0)    // remove scaling by intra-axonal volume from inter-axonal compartment
                {
                    if (g>=0)
                        pix[g] /= intraAxonalVolume;
                    else
                        pix /= intraAxonalVolume;
                }

                if (m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage()!=nullptr)
                {
                    double val = InterpolateValue(point, m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage());
                    if (val<0)
                        continue;
                    else
                        weight = val*m_VoxelVolume;
                }

                if (g>=0)
                    pix[g] *= weight;
                else
                    pix *= weight;
                m_CompartmentImages.at(i)->SetPixel(index, pix);
                if (g==0)
                    m_VolumeFractions.at(i)->SetPixel(index, weight/m_VoxelVolume);
            }

            for (int i=0; i<numNonFiberCompartments; i++)
            {
                double weight = other;
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i+numFiberCompartments)->GetPixel(index);
                if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()!=nullptr)
                {
                    double val = InterpolateValue(point, m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage());
                    if (val<0)
                        continue;
                    else
                        weight = val*m_VoxelVolume;

                    if (m_UseRelativeNonFiberVolumeFractions)
                        weight *= other/m_VoxelVolume;
                }

                if (g>=0)
                    pix[g] += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement(g)*weight;
                else
                    pix += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement()*weight;
                m_CompartmentImages.at(i+numFiberCompartments)->SetPixel(index, pix);
                if (g==0)
                    m_VolumeFractions.at(i+numFiberCompartments)->SetPixel(index, weight/m_VoxelVolume);
            }
        }
    }
}

template< class PixelType >
double TractsToDWIImageFilter< PixelType >::InterpolateValue(itk::Point<float, 3> itkP, ItkDoubleImgType::Pointer img)
{
    itk::Index<3> idx;
    itk::ContinuousIndex< double, 3> cIdx;
    img->TransformPhysicalPointToIndex(itkP, idx);
    img->TransformPhysicalPointToContinuousIndex(itkP, cIdx);

    double pix = -1;
    if ( img->GetLargestPossibleRegion().IsInside(idx) )
        pix = img->GetPixel(idx);
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
    if (idx[0] >= 0 && idx[0] < img->GetLargestPossibleRegion().GetSize(0)-1 &&
            idx[1] >= 0 && idx[1] < img->GetLargestPossibleRegion().GetSize(1)-1 &&
            idx[2] >= 0 && idx[2] < img->GetLargestPossibleRegion().GetSize(2)-1)
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

        pix = img->GetPixel(idx) * interpWeights[0];
        ItkDoubleImgType::IndexType tmpIdx = idx; tmpIdx[0]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[1];
        tmpIdx = idx; tmpIdx[1]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[2];
        tmpIdx = idx; tmpIdx[2]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[3];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[4];
        tmpIdx = idx; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[5];
        tmpIdx = idx; tmpIdx[2]++; tmpIdx[0]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[6];
        tmpIdx = idx; tmpIdx[0]++; tmpIdx[1]++; tmpIdx[2]++;
        pix +=  img->GetPixel(tmpIdx) * interpWeights[7];
    }

    return pix;
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
