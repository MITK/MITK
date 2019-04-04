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
#include <exception>
#include <itkImageDuplicator.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkDiffusionTensor3D.h>
#include <itkTractDensityImageFilter.h>
#include <mitkLexicalCast.h>
#include <itkTractsToVectorImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkResampleDwiImageFilter.h>
#include <boost/algorithm/string/replace.hpp>
#include <omp.h>
#include <cmath>
#include <thread>

namespace itk
{

template< class PixelType >
TractsToDWIImageFilter< PixelType >::TractsToDWIImageFilter()
  : m_StatusText("")
  , m_UseConstantRandSeed(false)
  , m_RandGen(itk::Statistics::MersenneTwisterRandomVariateGenerator::New())
{
  m_DoubleInterpolator = itk::LinearInterpolateImageFunction< ItkDoubleImgType, float >::New();
  m_NullDir.Fill(0);
}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::~TractsToDWIImageFilter()
{

}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::DoubleDwiType::Pointer TractsToDWIImageFilter< PixelType >::
SimulateKspaceAcquisition( std::vector< DoubleDwiType::Pointer >& compartment_images )
{
  unsigned int numFiberCompartments = m_Parameters.m_FiberModelList.size();
  // create slice object
  ImageRegion<2> sliceRegion;
  sliceRegion.SetSize(0, m_WorkingImageRegion.GetSize()[0]);
  sliceRegion.SetSize(1, m_WorkingImageRegion.GetSize()[1]);
  Vector< double, 2 > sliceSpacing;
  sliceSpacing[0] = m_WorkingSpacing[0];
  sliceSpacing[1] = m_WorkingSpacing[1];

  DoubleDwiType::PixelType nullPix; nullPix.SetSize(compartment_images.at(0)->GetVectorLength()); nullPix.Fill(0.0);
  auto magnitudeDwiImage = DoubleDwiType::New();
  magnitudeDwiImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
  magnitudeDwiImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
  magnitudeDwiImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
  magnitudeDwiImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  magnitudeDwiImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  magnitudeDwiImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  magnitudeDwiImage->SetVectorLength( compartment_images.at(0)->GetVectorLength() );
  magnitudeDwiImage->Allocate();
  magnitudeDwiImage->FillBuffer(nullPix);

  m_PhaseImage = DoubleDwiType::New();
  m_PhaseImage->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
  m_PhaseImage->SetOrigin( m_Parameters.m_SignalGen.m_ImageOrigin );
  m_PhaseImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
  m_PhaseImage->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  m_PhaseImage->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  m_PhaseImage->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
  m_PhaseImage->SetVectorLength( compartment_images.at(0)->GetVectorLength() );
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
  for (unsigned int c=0; c<m_Parameters.m_SignalGen.m_NumberOfCoils; c++)
  {
    coilPositions.push_back(pos);
    m_CoilPointset->InsertPoint(c, pos*1000 + m_Parameters.m_SignalGen.m_ImageOrigin.GetVectorFromOrigin() + center );

    double rz = 360.0/m_Parameters.m_SignalGen.m_NumberOfCoils * itk::Math::pi/180;
    vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
    rotZ[0][0] = cos(rz);
    rotZ[1][1] = rotZ[0][0];
    rotZ[0][1] = -sin(rz);
    rotZ[1][0] = -rotZ[0][1];

    pos.SetVnlVector(rotZ*pos.GetVnlVector());
  }

  auto num_slices = compartment_images.at(0)->GetLargestPossibleRegion().GetSize(2);
  auto num_gradient_volumes = static_cast<int>(compartment_images.at(0)->GetVectorLength());
  auto max_threads = omp_get_max_threads();
  int out_threads = Math::ceil(std::sqrt(max_threads));
  int in_threads = Math::floor(std::sqrt(max_threads));
  if (out_threads > num_gradient_volumes)
  {
    out_threads = num_gradient_volumes;
    in_threads = Math::floor(static_cast<float>(max_threads/out_threads));
  }
  PrintToLog("Parallel volumes: " + boost::lexical_cast<std::string>(out_threads), false, true, true);
  PrintToLog("Threads per slice: " + boost::lexical_cast<std::string>(in_threads), false, true, true);

  std::list< std::tuple<unsigned int, unsigned int, unsigned int> > spikes;
  if (m_Parameters.m_Misc.m_DoAddSpikes)
    for (unsigned int i=0; i<m_Parameters.m_SignalGen.m_Spikes; i++)
    {
      // gradient volume and slice index
      auto spike = std::tuple<unsigned int, unsigned int, unsigned int>(
            m_RandGen->GetIntegerVariate()%num_gradient_volumes,
            m_RandGen->GetIntegerVariate()%num_slices,
            m_RandGen->GetIntegerVariate()%m_Parameters.m_SignalGen.m_NumberOfCoils);
      spikes.push_back(spike);
    }

  bool output_timing = m_Parameters.m_Misc.m_OutputAdditionalImages;

  PrintToLog("0%   10   20   30   40   50   60   70   80   90   100%", false, true, false);
  PrintToLog("|----|----|----|----|----|----|----|----|----|----|\n*", false, false, false);
  unsigned long lastTick = 0;

  boost::progress_display disp(static_cast<unsigned long>(num_gradient_volumes)*compartment_images.at(0)->GetLargestPossibleRegion().GetSize(2));

#pragma omp parallel for num_threads(out_threads)
  for (int g=0; g<num_gradient_volumes; g++)
  {
    if (this->GetAbortGenerateData())
      continue;

    std::list< std::tuple<unsigned int, unsigned int> > spikeSlice;
#pragma omp critical
    {

      for (auto spike : spikes)
        if (std::get<0>(spike) == static_cast<unsigned int>(g))
          spikeSlice.push_back(std::tuple<unsigned int, unsigned int>(std::get<1>(spike), std::get<2>(spike)));
    }

    for (unsigned int z=0; z<num_slices; z++)
    {
      std::vector< Float2DImageType::Pointer > compartment_slices;
      std::vector< float > t2Vector;
      std::vector< float > t1Vector;

      for (unsigned int i=0; i<compartment_images.size(); i++)
      {
        DiffusionSignalModel<double>* signalModel;
        if (i<numFiberCompartments)
          signalModel = m_Parameters.m_FiberModelList.at(i);
        else
          signalModel = m_Parameters.m_NonFiberModelList.at(i-numFiberCompartments);

        auto slice = Float2DImageType::New();
        slice->SetLargestPossibleRegion( sliceRegion );
        slice->SetBufferedRegion( sliceRegion );
        slice->SetRequestedRegion( sliceRegion );
        slice->SetSpacing(sliceSpacing);
        slice->Allocate();
        slice->FillBuffer(0.0);

        // extract slice from channel g
        for (unsigned int y=0; y<compartment_images.at(0)->GetLargestPossibleRegion().GetSize(1); y++)
          for (unsigned int x=0; x<compartment_images.at(0)->GetLargestPossibleRegion().GetSize(0); x++)
          {
            Float2DImageType::IndexType index2D; index2D[0]=x; index2D[1]=y;
            DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;

            slice->SetPixel(index2D, compartment_images.at(i)->GetPixel(index3D)[g]);
          }

        compartment_slices.push_back(slice);
        t2Vector.push_back(signalModel->GetT2());
        t1Vector.push_back(signalModel->GetT1());
      }

      if (this->GetAbortGenerateData())
        continue;

      for (unsigned int c=0; c<m_Parameters.m_SignalGen.m_NumberOfCoils; c++)
      {
        int numSpikes = 0;
        for (auto ss : spikeSlice)
          if (std::get<0>(ss) == z && std::get<1>(ss) == c)
            ++numSpikes;

        // create k-sapce (inverse fourier transform slices)
        auto idft = itk::KspaceImageFilter< Float2DImageType::PixelType >::New();
        idft->SetCompartmentImages(compartment_slices);
        idft->SetT2(t2Vector);
        idft->SetT1(t1Vector);
        if (m_UseConstantRandSeed)
        {
          int linear_seed = g + num_gradient_volumes*z + num_gradient_volumes*compartment_images.at(0)->GetLargestPossibleRegion().GetSize(2)*c;
          idft->SetRandSeed(linear_seed);
        }
        idft->SetParameters(&m_Parameters);
        idft->SetZ((float)z-(float)( compartment_images.at(0)->GetLargestPossibleRegion().GetSize(2)
                                     -compartment_images.at(0)->GetLargestPossibleRegion().GetSize(2)%2 ) / 2.0);
        idft->SetZidx(z);
        idft->SetCoilPosition(coilPositions.at(c));
        idft->SetFiberBundle(m_FiberBundle);
        idft->SetTranslation(m_Translations.at(g));
        idft->SetRotationMatrix(m_RotationsInv.at(g));
        idft->SetDiffusionGradientDirection(m_Parameters.m_SignalGen.GetGradientDirection(g)*m_Parameters.m_SignalGen.GetBvalue()/1000.0);
        idft->SetSpikesPerSlice(numSpikes);
        idft->SetNumberOfThreads(in_threads);
#pragma omp critical
        if (output_timing)
        {
            idft->SetStoreTimings(true);
            output_timing = false;
        }
        idft->Update();

#pragma omp critical
        if (numSpikes>0)
        {
          m_SpikeLog += "Volume " + boost::lexical_cast<std::string>(g) + " Coil " + boost::lexical_cast<std::string>(c) + "\n";
          m_SpikeLog += idft->GetSpikeLog();
        }

        Complex2DImageType::Pointer fSlice;
        fSlice = idft->GetOutput();

        if (idft->GetTickImage().IsNotNull())
            m_TickImage = idft->GetTickImage();
        if (idft->GetRfImage().IsNotNull())
            m_RfImage = idft->GetRfImage();

        // fourier transform slice
        Complex2DImageType::Pointer newSlice;
        auto dft = itk::DftImageFilter< Float2DImageType::PixelType >::New();
        dft->SetInput(fSlice);
        dft->SetParameters(m_Parameters);
        dft->SetNumberOfThreads(in_threads);
        dft->Update();
        newSlice = dft->GetOutput();

        // put slice back into channel g
        for (unsigned int y=0; y<fSlice->GetLargestPossibleRegion().GetSize(1); y++)
          for (unsigned int x=0; x<fSlice->GetLargestPossibleRegion().GetSize(0); x++)
          {
            DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
            Complex2DImageType::IndexType index2D; index2D[0]=x; index2D[1]=y;

            Complex2DImageType::PixelType cPix = newSlice->GetPixel(index2D);
            double magn = sqrt(cPix.real()*cPix.real()+cPix.imag()*cPix.imag());
            double phase = 0;
            if (cPix.real()!=0)
              phase = atan( cPix.imag()/cPix.real() );

            DoubleDwiType::PixelType real_pix = m_OutputImagesReal.at(c)->GetPixel(index3D);
            real_pix[g] = cPix.real();
            m_OutputImagesReal.at(c)->SetPixel(index3D, real_pix);

            DoubleDwiType::PixelType imag_pix = m_OutputImagesImag.at(c)->GetPixel(index3D);
            imag_pix[g] = cPix.imag();
            m_OutputImagesImag.at(c)->SetPixel(index3D, imag_pix);

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

            //#pragma omp critical
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
        for (int y=0; y<static_cast<int>(magnitudeDwiImage->GetLargestPossibleRegion().GetSize(1)); y++)
          for (int x=0; x<static_cast<int>(magnitudeDwiImage->GetLargestPossibleRegion().GetSize(0)); x++)
          {
            DoubleDwiType::IndexType index3D; index3D[0]=x; index3D[1]=y; index3D[2]=z;
            DoubleDwiType::PixelType magPix = magnitudeDwiImage->GetPixel(index3D);
            magPix[g] = sqrt(magPix[g]/m_Parameters.m_SignalGen.m_NumberOfCoils);

            DoubleDwiType::PixelType phasePix = m_PhaseImage->GetPixel(index3D);
            phasePix[g] = sqrt(phasePix[g]/m_Parameters.m_SignalGen.m_NumberOfCoils);

            //#pragma omp critical
            {
              magnitudeDwiImage->SetPixel(index3D, magPix);
              m_PhaseImage->SetPixel(index3D, phasePix);
            }
          }
      }

      ++disp;
      unsigned long newTick = 50*disp.count()/disp.expected_count();
      for (unsigned long tick = 0; tick<(newTick-lastTick); tick++)
        PrintToLog("*", false, false, false);
      lastTick = newTick;
    }
  }


  PrintToLog("\n", false);
  return magnitudeDwiImage;
}

template< class PixelType >
TractsToDWIImageFilter< PixelType >::ItkDoubleImgType::Pointer TractsToDWIImageFilter< PixelType >::
NormalizeInsideMask(ItkDoubleImgType::Pointer image)
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

    if (it.Get()>max)
      max = it.Get();
    if (it.Get()<min)
      min = it.Get();
    ++it;
  }
  auto scaler = itk::ShiftScaleImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
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
  unsigned int fibVolImages = 0;
  for (std::size_t i=0; i<m_Parameters.m_FiberModelList.size(); i++)
  {
    if (m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage().IsNotNull())
    {
      PrintToLog("Using volume fraction map for fiber compartment " + boost::lexical_cast<std::string>(i+1), false);
      fibVolImages++;
    }
  }

  // check for non-fiber volume fraction maps
  unsigned int nonfibVolImages = 0;
  for (std::size_t i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
  {
    if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage().IsNotNull())
    {
      PrintToLog("Using volume fraction map for non-fiber compartment " + boost::lexical_cast<std::string>(i+1), false);
      nonfibVolImages++;
    }
  }

  // not all fiber compartments are using volume fraction maps
  // --> non-fiber volume fractions are assumed to be relative to the
  // non-fiber volume and not absolute voxel-volume fractions.
  // this means if two non-fiber compartments are used but only one of them
  // has an associated volume fraction map, the repesctive other volume fraction map
  // can be determined as inverse (1-val) of the present volume fraction map-
  if ( fibVolImages<m_Parameters.m_FiberModelList.size() && nonfibVolImages==1 && m_Parameters.m_NonFiberModelList.size()==2)
  {
    PrintToLog("Calculating missing non-fiber volume fraction image by inverting the other.\n"
               "Assuming non-fiber volume fraction images to contain values relative to the"
               " remaining non-fiber volume, not absolute values.", false);

    auto inverter = itk::InvertIntensityImageFilter< ItkDoubleImgType, ItkDoubleImgType >::New();
    inverter->SetMaximum(1.0);
    if ( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage().IsNull()
         && m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage().IsNotNull() )
    {
      // m_Parameters.m_NonFiberModelList[1]->SetVolumeFractionImage(
      // NormalizeInsideMask( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage() ) );
      inverter->SetInput( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage() );
      inverter->Update();
      m_Parameters.m_NonFiberModelList[0]->SetVolumeFractionImage(inverter->GetOutput());
    }
    else if ( m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage().IsNull()
              && m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage().IsNotNull() )
    {
      // m_Parameters.m_NonFiberModelList[0]->SetVolumeFractionImage(
      // NormalizeInsideMask( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage() ) );
      inverter->SetInput( m_Parameters.m_NonFiberModelList[0]->GetVolumeFractionImage() );
      inverter->Update();
      m_Parameters.m_NonFiberModelList[1]->SetVolumeFractionImage(inverter->GetOutput());
    }
    else
    {
      itkExceptionMacro("Something went wrong in automatically calculating the missing non-fiber volume fraction image!"
                        " Did you use two non fiber compartments but only one volume fraction image?"
                        " Then it should work and this error is really strange.");
    }
    m_UseRelativeNonFiberVolumeFractions = true;

    nonfibVolImages++;
  }

  // Up to two fiber compartments are allowed without volume fraction maps since the volume fractions can then be determined automatically
  if (m_Parameters.m_FiberModelList.size()>2
      && fibVolImages!=m_Parameters.m_FiberModelList.size())
    itkExceptionMacro("More than two fiber compartment selected but no corresponding volume fraction maps set!");

  // One non-fiber compartment is allowed without volume fraction map since the volume fraction can then be determined automatically
  if (m_Parameters.m_NonFiberModelList.size()>1
      && nonfibVolImages!=m_Parameters.m_NonFiberModelList.size())
    itkExceptionMacro("More than one non-fiber compartment selected but no volume fraction maps set!");

  if (fibVolImages<m_Parameters.m_FiberModelList.size() && nonfibVolImages>0)
  {
    PrintToLog("Not all fiber compartments are using an associated volume fraction image.\n"
               "Assuming non-fiber volume fraction images to contain values relative to the"
               " remaining non-fiber volume, not absolute values.", false);
    m_UseRelativeNonFiberVolumeFractions = true;

//    mitk::LocaleSwitch localeSwitch("C");
    //        itk::ImageFileWriter<ItkDoubleImgType>::Pointer wr = itk::ImageFileWriter<ItkDoubleImgType>::New();
    //        wr->SetInput(m_Parameters.m_NonFiberModelList[1]->GetVolumeFractionImage());
    //        wr->SetFileName("/local/volumefraction.nrrd");
    //        wr->Update();
  }

  // initialize the images that store the output volume fraction of each compartment
  m_VolumeFractions.clear();
  for (std::size_t i=0; i<m_Parameters.m_FiberModelList.size()+m_Parameters.m_NonFiberModelList.size(); i++)
  {
    auto doubleImg = ItkDoubleImgType::New();
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
  m_Rotations.clear();
  m_Translations.clear();
  m_MotionLog = "";
  m_SpikeLog = "";

  m_TickImage = nullptr;
  m_RfImage = nullptr;

  // initialize output dwi image
  m_Parameters.m_SignalGen.m_CroppedRegion = m_Parameters.m_SignalGen.m_ImageRegion;
  if (m_Parameters.m_Misc.m_DoAddAliasing)
    m_Parameters.m_SignalGen.m_CroppedRegion.SetSize( 1, m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1)
                                                      *m_Parameters.m_SignalGen.m_CroppingFactor);

  itk::Point<double,3> shiftedOrigin = m_Parameters.m_SignalGen.m_ImageOrigin;
  shiftedOrigin[1] += (m_Parameters.m_SignalGen.m_ImageRegion.GetSize(1)
                       -m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1))*m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;

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

  PrintToLog("Output image spacing: [" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_ImageSpacing[0]) + "," + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_ImageSpacing[1]) + "," + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_ImageSpacing[2]) + "]", false);
  PrintToLog("Output image size: [" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(0)) + "," + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1)) + "," + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(2)) + "]", false);

  // images containing real and imaginary part of the dMRI signal for each coil
  m_OutputImagesReal.clear();
  m_OutputImagesImag.clear();
  for (unsigned int i=0; i<m_Parameters.m_SignalGen.m_NumberOfCoils; ++i)
  {
    typename DoubleDwiType::Pointer outputImageReal = DoubleDwiType::New();
    outputImageReal->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    outputImageReal->SetOrigin( shiftedOrigin );
    outputImageReal->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    outputImageReal->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageReal->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageReal->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageReal->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
    outputImageReal->Allocate();
    outputImageReal->FillBuffer(temp);
    m_OutputImagesReal.push_back(outputImageReal);

    typename DoubleDwiType::Pointer outputImageImag = DoubleDwiType::New();
    outputImageImag->SetSpacing( m_Parameters.m_SignalGen.m_ImageSpacing );
    outputImageImag->SetOrigin( shiftedOrigin );
    outputImageImag->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
    outputImageImag->SetLargestPossibleRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageImag->SetBufferedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageImag->SetRequestedRegion( m_Parameters.m_SignalGen.m_CroppedRegion );
    outputImageImag->SetVectorLength( m_Parameters.m_SignalGen.GetNumVolumes() );
    outputImageImag->Allocate();
    outputImageImag->FillBuffer(temp);
    m_OutputImagesImag.push_back(outputImageImag);
  }

  // Apply in-plane upsampling for Gibbs ringing artifact
  double upsampling = 1;
  if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging && m_Parameters.m_SignalGen.m_ZeroRinging==0)
    upsampling = 2;
  m_WorkingSpacing = m_Parameters.m_SignalGen.m_ImageSpacing;
  m_WorkingSpacing[0] /= upsampling;
  m_WorkingSpacing[1] /= upsampling;
  m_WorkingImageRegion = m_Parameters.m_SignalGen.m_ImageRegion;
  m_WorkingImageRegion.SetSize(0, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[0]*upsampling);
  m_WorkingImageRegion.SetSize(1, m_Parameters.m_SignalGen.m_ImageRegion.GetSize()[1]*upsampling);
  m_WorkingOrigin = m_Parameters.m_SignalGen.m_ImageOrigin;
  m_WorkingOrigin[0] -= m_Parameters.m_SignalGen.m_ImageSpacing[0]/2;
  m_WorkingOrigin[0] += m_WorkingSpacing[0]/2;
  m_WorkingOrigin[1] -= m_Parameters.m_SignalGen.m_ImageSpacing[1]/2;
  m_WorkingOrigin[1] += m_WorkingSpacing[1]/2;
  m_WorkingOrigin[2] -= m_Parameters.m_SignalGen.m_ImageSpacing[2]/2;
  m_WorkingOrigin[2] += m_WorkingSpacing[2]/2;
  m_VoxelVolume = m_WorkingSpacing[0]*m_WorkingSpacing[1]*m_WorkingSpacing[2];

  PrintToLog("Working image spacing: [" + boost::lexical_cast<std::string>(m_WorkingSpacing[0]) + "," + boost::lexical_cast<std::string>(m_WorkingSpacing[1]) + "," + boost::lexical_cast<std::string>(m_WorkingSpacing[2]) + "]", false);
  PrintToLog("Working image size: [" + boost::lexical_cast<std::string>(m_WorkingImageRegion.GetSize(0)) + "," + boost::lexical_cast<std::string>(m_WorkingImageRegion.GetSize(1)) + "," + boost::lexical_cast<std::string>(m_WorkingImageRegion.GetSize(2)) + "]", false);

  // generate double images to store the individual compartment signals
  m_CompartmentImages.clear();
  int numFiberCompartments = m_Parameters.m_FiberModelList.size();
  int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();
  for (int i=0; i<numFiberCompartments+numNonFiberCompartments; i++)
  {
    auto doubleDwi = DoubleDwiType::New();
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

  if (m_FiberBundle.IsNull() && m_InputImage.IsNotNull())
  {
    m_CompartmentImages.clear();

    m_Parameters.m_SignalGen.m_DoAddMotion = false;
    m_Parameters.m_SignalGen.m_DoSimulateRelaxation = false;

    PrintToLog("Simulating acquisition for input diffusion-weighted image.", false);
    auto caster = itk::CastImageFilter< OutputImageType, DoubleDwiType >::New();
    caster->SetInput(m_InputImage);
    caster->Update();

    if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging && m_Parameters.m_SignalGen.m_ZeroRinging==0)
    {
      PrintToLog("Upsampling input diffusion-weighted image for Gibbs ringing simulation.", false);

      auto resampler = itk::ResampleDwiImageFilter< double >::New();
      resampler->SetInput(caster->GetOutput());
      itk::Vector< double, 3 > samplingFactor;
      samplingFactor[0] = upsampling;
      samplingFactor[1] = upsampling;
      samplingFactor[2] = 1;
      resampler->SetSamplingFactor(samplingFactor);
      resampler->SetInterpolation(itk::ResampleDwiImageFilter< double >::Interpolate_WindowedSinc);
      resampler->Update();
      m_CompartmentImages.push_back(resampler->GetOutput());
    }
    else
      m_CompartmentImages.push_back(caster->GetOutput());

    VectorType translation; translation.Fill(0.0);
    MatrixType rotation; rotation.SetIdentity();
    for (unsigned int g=0; g<m_Parameters.m_SignalGen.GetNumVolumes(); g++)
    {
      m_Rotations.push_back(rotation);
      m_RotationsInv.push_back(rotation);
      m_Translations.push_back(translation);
    }
  }

  // resample mask image and frequency map to fit upsampled geometry
  if (m_Parameters.m_SignalGen.m_MaskImage.IsNotNull() && m_Parameters.m_SignalGen.m_MaskImage->GetLargestPossibleRegion()!=m_WorkingImageRegion)
  {
    PrintToLog("Resampling tissue mask", false);
    // rescale mask image (otherwise there are problems with the resampling)
    auto rescaler = itk::RescaleIntensityImageFilter<ItkUcharImgType,ItkUcharImgType>::New();
    rescaler->SetInput(0,m_Parameters.m_SignalGen.m_MaskImage);
    rescaler->SetOutputMaximum(100);
    rescaler->SetOutputMinimum(0);
    rescaler->Update();

    // resample mask image
    auto resampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
    resampler->SetInput(rescaler->GetOutput());
    resampler->SetSize(m_WorkingImageRegion.GetSize());
    resampler->SetOutputSpacing(m_WorkingSpacing);
    resampler->SetOutputOrigin(m_WorkingOrigin);
    resampler->SetOutputDirection(m_Parameters.m_SignalGen.m_ImageDirection);
    resampler->SetOutputStartIndex ( m_WorkingImageRegion.GetIndex() );
    auto nn_interpolator = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
    resampler->SetInterpolator(nn_interpolator);
    resampler->Update();
    m_Parameters.m_SignalGen.m_MaskImage = resampler->GetOutput();
  }
  // resample frequency map
  if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull() && m_Parameters.m_SignalGen.m_FrequencyMap->GetLargestPossibleRegion()!=m_WorkingImageRegion)
  {
    PrintToLog("Resampling frequency map", false);
    auto resampler = itk::ResampleImageFilter<ItkFloatImgType, ItkFloatImgType>::New();
    resampler->SetInput(m_Parameters.m_SignalGen.m_FrequencyMap);
    resampler->SetSize(m_WorkingImageRegion.GetSize());
    resampler->SetOutputSpacing(m_WorkingSpacing);
    resampler->SetOutputOrigin(m_WorkingOrigin);
    resampler->SetOutputDirection(m_Parameters.m_SignalGen.m_ImageDirection);
    resampler->SetOutputStartIndex ( m_WorkingImageRegion.GetIndex() );
    auto nn_interpolator = itk::NearestNeighborInterpolateImageFunction<ItkFloatImgType, double>::New();
    resampler->SetInterpolator(nn_interpolator);
    resampler->Update();
    m_Parameters.m_SignalGen.m_FrequencyMap = resampler->GetOutput();
  }

  m_MaskImageSet = true;
  if (m_Parameters.m_SignalGen.m_MaskImage.IsNull())
  {
    // no input tissue mask is set -> create default
    PrintToLog("No tissue mask set", false);
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
    PrintToLog("Using tissue mask", false);
  }

  if (m_Parameters.m_SignalGen.m_DoAddMotion)
  {
    if (m_Parameters.m_SignalGen.m_DoRandomizeMotion)
    {
      PrintToLog("Random motion artifacts:", false);
      PrintToLog("Maximum rotation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Rotation) + "°", false);
      PrintToLog("Maximum translation: +/-" + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Translation) + "mm", false);
    }
    else
    {
      PrintToLog("Linear motion artifacts:", false);
      PrintToLog("Maximum rotation: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Rotation) + "°", false);
      PrintToLog("Maximum translation: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Translation) + "mm", false);
    }
  }
  if ( m_Parameters.m_SignalGen.m_MotionVolumes.empty() )
  {
    // no motion in first volume
    m_Parameters.m_SignalGen.m_MotionVolumes.push_back(false);

    // motion in all other volumes
    while ( m_Parameters.m_SignalGen.m_MotionVolumes.size() < m_Parameters.m_SignalGen.GetNumVolumes() )
    {
      m_Parameters.m_SignalGen.m_MotionVolumes.push_back(true);
    }
  }
  // we need to know for every volume if there is motion. if this information is missing, then set corresponding fal to false
  while ( m_Parameters.m_SignalGen.m_MotionVolumes.size()<m_Parameters.m_SignalGen.GetNumVolumes() )
  {
    m_Parameters.m_SignalGen.m_MotionVolumes.push_back(false);
  }

  m_NumMotionVolumes = 0;
  for (unsigned int i=0; i<m_Parameters.m_SignalGen.GetNumVolumes(); ++i)
  {
    if (m_Parameters.m_SignalGen.m_MotionVolumes[i])
      ++m_NumMotionVolumes;
  }
  m_MotionCounter = 0;

  // creat image to hold transformed mask (motion artifact)
  m_TransformedMaskImage = ItkUcharImgType::New();
  auto duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
  duplicator->SetInputImage(m_Parameters.m_SignalGen.m_MaskImage);
  duplicator->Update();
  m_TransformedMaskImage = duplicator->GetOutput();

  // second upsampling needed for motion artifacts
  ImageRegion<3>      upsampledImageRegion = m_WorkingImageRegion;
  VectorType    upsampledSpacing = m_WorkingSpacing;
  upsampledSpacing[0] /= 4;
  upsampledSpacing[1] /= 4;
  upsampledSpacing[2] /= 4;
  upsampledImageRegion.SetSize(0, m_WorkingImageRegion.GetSize()[0]*4);
  upsampledImageRegion.SetSize(1, m_WorkingImageRegion.GetSize()[1]*4);
  upsampledImageRegion.SetSize(2, m_WorkingImageRegion.GetSize()[2]*4);
  itk::Point<double,3> upsampledOrigin = m_WorkingOrigin;
  upsampledOrigin[0] -= m_WorkingSpacing[0]/2;
  upsampledOrigin[0] += upsampledSpacing[0]/2;
  upsampledOrigin[1] -= m_WorkingSpacing[1]/2;
  upsampledOrigin[1] += upsampledSpacing[1]/2;
  upsampledOrigin[2] -= m_WorkingSpacing[2]/2;
  upsampledOrigin[2] += upsampledSpacing[2]/2;

  m_UpsampledMaskImage = ItkUcharImgType::New();
  auto upsampler = itk::ResampleImageFilter<ItkUcharImgType, ItkUcharImgType>::New();
  upsampler->SetInput(m_Parameters.m_SignalGen.m_MaskImage);
  upsampler->SetOutputParametersFromImage(m_Parameters.m_SignalGen.m_MaskImage);
  upsampler->SetSize(upsampledImageRegion.GetSize());
  upsampler->SetOutputSpacing(upsampledSpacing);
  upsampler->SetOutputOrigin(upsampledOrigin);

  auto nn_interpolator = itk::NearestNeighborInterpolateImageFunction<ItkUcharImgType, double>::New();
  upsampler->SetInterpolator(nn_interpolator);
  upsampler->Update();
  m_UpsampledMaskImage = upsampler->GetOutput();
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::InitializeFiberData()
{
  m_mmRadius = m_Parameters.m_SignalGen.m_AxonRadius/1000;

  auto caster = itk::CastImageFilter< itk::Image<unsigned char, 3>, itk::Image<double, 3> >::New();
  caster->SetInput(m_TransformedMaskImage);
  caster->Update();

  vtkSmartPointer<vtkFloatArray> weights = m_FiberBundle->GetFiberWeights();
  float mean_weight = 0;
  for (int i=0; i<weights->GetSize(); i++)
    mean_weight += weights->GetValue(i);
  mean_weight /= weights->GetSize();

  if (mean_weight>0.000001)
    for (int i=0; i<weights->GetSize(); i++)
      m_FiberBundle->SetFiberWeight(i, weights->GetValue(i)/mean_weight);
  else
    PrintToLog("\nWarning: streamlines have VERY low weights. Average weight: " + boost::lexical_cast<std::string>(mean_weight) + ". Possible source of calculation errors.", false, true, true);


  auto density_calculator = itk::TractDensityImageFilter< itk::Image<double, 3> >::New();
  density_calculator->SetFiberBundle(m_FiberBundle);
  density_calculator->SetInputImage(caster->GetOutput());
  density_calculator->SetBinaryOutput(false);
  density_calculator->SetUseImageGeometry(true);
  density_calculator->SetOutputAbsoluteValues(true);
  density_calculator->Update();
  double max_density = density_calculator->GetMaxDensity();

  double voxel_volume = m_WorkingSpacing[0]*m_WorkingSpacing[1]*m_WorkingSpacing[2];
  if (m_mmRadius>0)
  {
    std::stringstream stream;
    stream << std::fixed << setprecision(2) << itk::Math::pi*m_mmRadius*m_mmRadius*max_density;
    std::string s = stream.str();
    PrintToLog("\nMax. fiber volume: " + s + "mm².", false, true, true);

    {
      double full_radius = 1000*std::sqrt(voxel_volume/(max_density*itk::Math::pi));
      std::stringstream stream;
      stream << std::fixed << setprecision(2) << full_radius;
      std::string s = stream.str();
      PrintToLog("\nA full fiber voxel corresponds to a fiber radius of ~" + s + "µm, given the current fiber configuration.", false, true, true);
    }
  }
  else
  {
    m_mmRadius = std::sqrt(voxel_volume/(max_density*itk::Math::pi));
    std::stringstream stream;
    stream << std::fixed << setprecision(2) << m_mmRadius*1000;
    std::string s = stream.str();
    PrintToLog("\nSetting fiber radius to " + s + "µm to obtain full voxel.", false, true, true);
  }

  // a second fiber bundle is needed to store the transformed version of the m_FiberBundleWorkingCopy
  m_FiberBundleTransformed = m_FiberBundle->GetDeepCopy();
}


template< class PixelType >
bool TractsToDWIImageFilter< PixelType >::PrepareLogFile()
{
  if(m_Logfile.is_open())
    m_Logfile.close();

  std::string filePath;
  std::string fileName;

  // Get directory name:
  if (m_Parameters.m_Misc.m_OutputPath.size() > 0)
  {
    filePath = m_Parameters.m_Misc.m_OutputPath;
    if( *(--(filePath.cend())) != '/')
    {
      filePath.push_back('/');
    }
  }
  else
    return false;

  // Get file name:
  if( ! m_Parameters.m_Misc.m_ResultNode->GetName().empty() )
  {
    fileName = m_Parameters.m_Misc.m_ResultNode->GetName();
  }
  else
  {
    fileName = "";
  }

  if( ! m_Parameters.m_Misc.m_OutputPrefix.empty() )
  {
    fileName = m_Parameters.m_Misc.m_OutputPrefix + fileName;
  }

  try
  {
    m_Logfile.open( ( filePath + '/' + fileName + ".log" ).c_str() );
  }
  catch (const std::ios_base::failure &fail)
  {
    MITK_ERROR << "itkTractsToDWIImageFilter.cpp: Exception " << fail.what()
               << " while trying to open file" << filePath << '/' << fileName << ".log";
    return false;
  }

  if ( m_Logfile.is_open() )
  {
    PrintToLog( "Logfile: " + filePath + '/' + fileName + ".log", false );
    return true;
  }
  else
    return false;
}


template< class PixelType >
void TractsToDWIImageFilter< PixelType >::GenerateData()
{
  PrintToLog("\n**********************************************", false);
  // prepare logfile
  PrepareLogFile();
  PrintToLog("Starting Fiberfox dMRI simulation");

  m_TimeProbe.Start();

  // check input data
  if (m_FiberBundle.IsNull() && m_InputImage.IsNull())
    itkExceptionMacro("Input fiber bundle and input diffusion-weighted image is nullptr!");

  if (m_Parameters.m_FiberModelList.empty() && m_InputImage.IsNull())
    itkExceptionMacro("No diffusion model for fiber compartments defined and input diffusion-weighted"
                      " image is nullptr! At least one fiber compartment is necessary to simulate diffusion.");

  if (m_Parameters.m_NonFiberModelList.empty() && m_InputImage.IsNull())
    itkExceptionMacro("No diffusion model for non-fiber compartments defined and input diffusion-weighted"
                      " image is nullptr! At least one non-fiber compartment is necessary to simulate diffusion.");

  if (m_Parameters.m_SignalGen.m_DoDisablePartialVolume)  // no partial volume? remove all but first fiber compartment
    while (m_Parameters.m_FiberModelList.size()>1)
      m_Parameters.m_FiberModelList.pop_back();

  if (!m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition)  // No upsampling of input image needed if no k-space simulation is performed
    m_Parameters.m_SignalGen.m_DoAddGibbsRinging = false;

  if (m_UseConstantRandSeed)  // always generate the same random numbers?
    m_RandGen->SetSeed(0);
  else
    m_RandGen->SetSeed();

  InitializeData();
  if ( m_FiberBundle.IsNotNull() )    // if no fiber bundle is found, we directly proceed to the k-space acquisition simulation
  {
    CheckVolumeFractionImages();
    InitializeFiberData();

    int numFiberCompartments = m_Parameters.m_FiberModelList.size();
    int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();

    double maxVolume = 0;
    unsigned long lastTick = 0;
    int signalModelSeed = m_RandGen->GetIntegerVariate();

    PrintToLog("\n", false, false);
    PrintToLog("Generating " + boost::lexical_cast<std::string>(numFiberCompartments+numNonFiberCompartments)
               + "-compartment diffusion-weighted signal.");

    std::vector< int > bVals = m_Parameters.m_SignalGen.GetBvalues();
    PrintToLog("b-values: ", false, false, true);
    for (auto v : bVals)
      PrintToLog(boost::lexical_cast<std::string>(v) + " ", false, false, true);
    PrintToLog("\nVolumes: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.GetNumVolumes()), false, true, true);

    PrintToLog("\n", false, false, true);
    PrintToLog("\n", false, false, true);

    unsigned int image_size_x = m_WorkingImageRegion.GetSize(0);
    unsigned int region_size_y = m_WorkingImageRegion.GetSize(1);
    unsigned int num_gradients = m_Parameters.m_SignalGen.GetNumVolumes();
    int numFibers = m_FiberBundle->GetNumFibers();
    boost::progress_display disp(numFibers*num_gradients);

    if (m_FiberBundle->GetMeanFiberLength()<5.0)
      omp_set_num_threads(2);

    PrintToLog("0%   10   20   30   40   50   60   70   80   90   100%", false, true, false);
    PrintToLog("|----|----|----|----|----|----|----|----|----|----|\n*", false, false, false);

    for (unsigned int g=0; g<num_gradients; ++g)
    {
      // move fibers
      SimulateMotion(g);

      // Set signal model random generator seeds to get same configuration in each voxel
      for (std::size_t i=0; i<m_Parameters.m_FiberModelList.size(); i++)
        m_Parameters.m_FiberModelList.at(i)->SetSeed(signalModelSeed);
      for (std::size_t i=0; i<m_Parameters.m_NonFiberModelList.size(); i++)
        m_Parameters.m_NonFiberModelList.at(i)->SetSeed(signalModelSeed);

      // storing voxel-wise intra-axonal volume in mm³
      auto intraAxonalVolumeImage = ItkDoubleImgType::New();
      intraAxonalVolumeImage->SetSpacing( m_WorkingSpacing );
      intraAxonalVolumeImage->SetOrigin( m_WorkingOrigin );
      intraAxonalVolumeImage->SetDirection( m_Parameters.m_SignalGen.m_ImageDirection );
      intraAxonalVolumeImage->SetLargestPossibleRegion( m_WorkingImageRegion );
      intraAxonalVolumeImage->SetBufferedRegion( m_WorkingImageRegion );
      intraAxonalVolumeImage->SetRequestedRegion( m_WorkingImageRegion );
      intraAxonalVolumeImage->Allocate();
      intraAxonalVolumeImage->FillBuffer(0);
      maxVolume = 0;
      double* intraAxBuffer = intraAxonalVolumeImage->GetBufferPointer();

      if (this->GetAbortGenerateData())
        continue;

      vtkPolyData* fiberPolyData = m_FiberBundleTransformed->GetFiberPolyData();
      // generate fiber signal (if there are any fiber models present)
      if (!m_Parameters.m_FiberModelList.empty())
      {
        std::vector< double* > buffers;
        for (unsigned int i=0; i<m_CompartmentImages.size(); ++i)
          buffers.push_back(m_CompartmentImages.at(i)->GetBufferPointer());
#pragma omp parallel for
        for( int i=0; i<numFibers; ++i )
        {
          if (this->GetAbortGenerateData())
            continue;

          float fiberWeight = m_FiberBundleTransformed->GetFiberWeight(i);

          int numPoints = -1;
          std::vector< itk::Vector<double, 3> > points_copy;
#pragma omp critical
          {
            vtkCell* cell = fiberPolyData->GetCell(i);
            numPoints = cell->GetNumberOfPoints();
            vtkPoints* points = cell->GetPoints();
            for (int j=0; j<numPoints; j++)
              points_copy.push_back(GetItkVector(points->GetPoint(j)));
          }

          if (numPoints<2)
            continue;

          double seg_volume = fiberWeight*itk::Math::pi*m_mmRadius*m_mmRadius;
          for( int j=0; j<numPoints - 1; ++j)
          {
            if (this->GetAbortGenerateData())
            {
              j=numPoints;
              continue;
            }

            itk::Vector<double> v = points_copy.at(j);

            itk::Vector<double, 3> dir = points_copy.at(j+1)-v;
            if ( dir.GetSquaredNorm()<0.0001 || dir[0]!=dir[0] || dir[1]!=dir[1] || dir[2]!=dir[2] )
              continue;
            dir.Normalize();

            itk::Point<float, 3> startVertex = points_copy.at(j);
            itk::Index<3> startIndex;
            itk::ContinuousIndex<float, 3> startIndexCont;
            m_TransformedMaskImage->TransformPhysicalPointToIndex(startVertex, startIndex);
            m_TransformedMaskImage->TransformPhysicalPointToContinuousIndex(startVertex, startIndexCont);

            itk::Point<float, 3> endVertex = points_copy.at(j+1);
            itk::Index<3> endIndex;
            itk::ContinuousIndex<float, 3> endIndexCont;
            m_TransformedMaskImage->TransformPhysicalPointToIndex(endVertex, endIndex);
            m_TransformedMaskImage->TransformPhysicalPointToContinuousIndex(endVertex, endIndexCont);

            std::vector< std::pair< itk::Index<3>, double > > segments = mitk::imv::IntersectImage(m_WorkingSpacing, startIndex, endIndex, startIndexCont, endIndexCont);

            // generate signal for each fiber compartment
            for (int k=0; k<numFiberCompartments; ++k)
            {
              double signal_add = m_Parameters.m_FiberModelList[k]->SimulateMeasurement(g, dir)*seg_volume;
              for (std::pair< itk::Index<3>, double > seg : segments)
              {
                if (!m_TransformedMaskImage->GetLargestPossibleRegion().IsInside(seg.first) || m_TransformedMaskImage->GetPixel(seg.first)<=0)
                  continue;

                double seg_signal = seg.second*signal_add;

                unsigned int linear_index = g + num_gradients*seg.first[0] + num_gradients*image_size_x*seg.first[1] + num_gradients*image_size_x*region_size_y*seg.first[2];

                // update dMRI volume
#pragma omp atomic
                buffers[k][linear_index] += seg_signal;

                // update fiber volume image
                if (k==0)
                {
                  linear_index = seg.first[0] + image_size_x*seg.first[1] + image_size_x*region_size_y*seg.first[2];
#pragma omp atomic
                  intraAxBuffer[linear_index] += seg.second*seg_volume;
                  double vol = intraAxBuffer[linear_index];
                  if (vol>maxVolume) { maxVolume = vol; }
                }
              }

            }
          }

#pragma omp critical
          {
            // progress report
            ++disp;
            unsigned long newTick = 50*disp.count()/disp.expected_count();
            for (unsigned int tick = 0; tick<(newTick-lastTick); ++tick)
              PrintToLog("*", false, false, false);
            lastTick = newTick;
          }
        }
      }

      // axon radius not manually defined --> set fullest voxel (maxVolume) to full fiber voxel
      double density_correctiony_global = 1.0;
      if (m_Parameters.m_SignalGen.m_AxonRadius<0.0001)
        density_correctiony_global = m_VoxelVolume/maxVolume;

      // generate non-fiber signal
      ImageRegionIterator<ItkUcharImgType> it3(m_TransformedMaskImage, m_TransformedMaskImage->GetLargestPossibleRegion());
      while(!it3.IsAtEnd())
      {
        if (it3.Get()>0)
        {
          DoubleDwiType::IndexType index = it3.GetIndex();
          double iAxVolume = intraAxonalVolumeImage->GetPixel(index);

          // get non-transformed point (remove headmotion tranformation)
          // this point lives in the volume fraction image space
          itk::Point<float, 3> volume_fraction_point;
          if ( m_Parameters.m_SignalGen.m_DoAddMotion )
            volume_fraction_point = GetMovedPoint(index, false);
          else
            m_TransformedMaskImage->TransformIndexToPhysicalPoint(index, volume_fraction_point);

          if (m_Parameters.m_SignalGen.m_DoDisablePartialVolume)
          {
            if (iAxVolume>0.0001) // scale fiber compartment to voxel
            {
              DoubleDwiType::PixelType pix = m_CompartmentImages.at(0)->GetPixel(index);
              pix[g] *= m_VoxelVolume/iAxVolume;
              m_CompartmentImages.at(0)->SetPixel(index, pix);

              if (g==0)
                m_VolumeFractions.at(0)->SetPixel(index, 1);
            }
            else
            {
              DoubleDwiType::PixelType pix = m_CompartmentImages.at(0)->GetPixel(index);
              pix[g] = 0;
              m_CompartmentImages.at(0)->SetPixel(index, pix);
              SimulateExtraAxonalSignal(index, volume_fraction_point, 0, g);
            }
          }
          else
          {
            // manually defined axon radius and voxel overflow --> rescale to voxel volume
            if ( m_Parameters.m_SignalGen.m_AxonRadius>=0.0001 && iAxVolume>m_VoxelVolume )
            {
              for (int i=0; i<numFiberCompartments; ++i)
              {
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                pix[g] *= m_VoxelVolume/iAxVolume;
                m_CompartmentImages.at(i)->SetPixel(index, pix);
              }
              iAxVolume = m_VoxelVolume;
            }

            // if volume fraction image is set use it, otherwise use global scaling factor
            double density_correction_voxel = density_correctiony_global;
            if ( m_Parameters.m_FiberModelList[0]->GetVolumeFractionImage()!=nullptr && iAxVolume>0.0001 )
            {
              m_DoubleInterpolator->SetInputImage(m_Parameters.m_FiberModelList[0]->GetVolumeFractionImage());
              double volume_fraction = mitk::imv::GetImageValue<double>(volume_fraction_point, true, m_DoubleInterpolator);
              if (volume_fraction<0)
                mitkThrow() << "Volume fraction image (index 1) contains negative values (intra-axonal compartment)!";
              density_correction_voxel = m_VoxelVolume*volume_fraction/iAxVolume; // remove iAxVolume sclaing and scale to volume_fraction
            }
            else if (m_Parameters.m_FiberModelList[0]->GetVolumeFractionImage()!=nullptr)
              density_correction_voxel = 0.0;

            // adjust intra-axonal compartment volume by density correction factor
            DoubleDwiType::PixelType pix = m_CompartmentImages.at(0)->GetPixel(index);
            pix[g] *= density_correction_voxel;
            m_CompartmentImages.at(0)->SetPixel(index, pix);

            // normalize remaining fiber volume fractions (they are rescaled in SimulateExtraAxonalSignal)
            if (iAxVolume>0.0001)
            {
              for (int i=1; i<numFiberCompartments; i++)
              {
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                pix[g] /= iAxVolume;
                m_CompartmentImages.at(i)->SetPixel(index, pix);
              }
            }
            else
            {
              for (int i=1; i<numFiberCompartments; i++)
              {
                DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
                pix[g] = 0;
                m_CompartmentImages.at(i)->SetPixel(index, pix);
              }
            }

            iAxVolume = density_correction_voxel*iAxVolume; // new intra-axonal volume = old intra-axonal volume * correction factor

            // simulate other compartments
            SimulateExtraAxonalSignal(index, volume_fraction_point, iAxVolume, g);
          }
        }
        ++it3;
      }
    }

    PrintToLog("\n", false);
  }

  if (this->GetAbortGenerateData())
  {
    PrintToLog("\n", false, false);
    PrintToLog("Simulation aborted");
    return;
  }

  DoubleDwiType::Pointer doubleOutImage;
  double signalScale = m_Parameters.m_SignalGen.m_SignalScale;
  if ( m_Parameters.m_SignalGen.m_SimulateKspaceAcquisition ) // do k-space stuff
  {
    PrintToLog("\n", false, false);
    PrintToLog("Simulating k-space acquisition using "
               +boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_NumberOfCoils)
               +" coil(s)");

    switch (m_Parameters.m_SignalGen.m_AcquisitionType)
    {
    case SignalGenerationParameters::SingleShotEpi:
    {
      PrintToLog("Acquisition type: single shot EPI", false);
      break;
    }
    case SignalGenerationParameters::ConventionalSpinEcho:
    {
      PrintToLog("Acquisition type: conventional spin echo (one RF pulse per line) with cartesian k-space trajectory", false);
      break;
    }
    case SignalGenerationParameters::FastSpinEcho:
    {
      PrintToLog("Acquisition type: fast spin echo (one RF pulse per ETL lines) with cartesian k-space trajectory (ETL: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_EchoTrainLength) + ")", false);
      break;
    }
    default:
    {
      PrintToLog("Acquisition type: single shot EPI", false);
      break;
    }
    }
    if(m_Parameters.m_SignalGen.m_tInv>0)
      PrintToLog("Using inversion pulse with TI " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_tInv) + "ms", false);

    if (m_Parameters.m_SignalGen.m_DoSimulateRelaxation)
      PrintToLog("Simulating signal relaxation", false);
    if (m_Parameters.m_SignalGen.m_NoiseVariance>0 && m_Parameters.m_Misc.m_DoAddNoise)
      PrintToLog("Simulating complex Gaussian noise: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_NoiseVariance), false);
    if (m_Parameters.m_SignalGen.m_FrequencyMap.IsNotNull() && m_Parameters.m_Misc.m_DoAddDistortions)
      PrintToLog("Simulating distortions", false);
    if (m_Parameters.m_SignalGen.m_DoAddGibbsRinging)
    {
      if (m_Parameters.m_SignalGen.m_ZeroRinging > 0)
        PrintToLog("Simulating ringing artifacts by zeroing " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_ZeroRinging) + "% of k-space frequencies", false);
      else
        PrintToLog("Simulating ringing artifacts by cropping high resolution inputs during k-space simulation", false);
    }
    if (m_Parameters.m_Misc.m_DoAddEddyCurrents && m_Parameters.m_SignalGen.m_EddyStrength>0)
      PrintToLog("Simulating eddy currents: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_EddyStrength), false);
    if (m_Parameters.m_Misc.m_DoAddSpikes && m_Parameters.m_SignalGen.m_Spikes>0)
      PrintToLog("Simulating spikes: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Spikes), false);
    if (m_Parameters.m_Misc.m_DoAddAliasing && m_Parameters.m_SignalGen.m_CroppingFactor<1.0)
      PrintToLog("Simulating aliasing: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_CroppingFactor), false);
    if (m_Parameters.m_Misc.m_DoAddGhosts && m_Parameters.m_SignalGen.m_KspaceLineOffset>0)
      PrintToLog("Simulating ghosts: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_KspaceLineOffset), false);

    doubleOutImage = SimulateKspaceAcquisition(m_CompartmentImages);
    signalScale = 1; // already scaled in SimulateKspaceAcquisition()
  }
  else    // don't do k-space stuff, just sum compartments
  {
    PrintToLog("Summing compartments");
    doubleOutImage = m_CompartmentImages.at(0);

    for (unsigned int i=1; i<m_CompartmentImages.size(); i++)
    {
      auto adder = itk::AddImageFilter< DoubleDwiType, DoubleDwiType, DoubleDwiType>::New();
      adder->SetInput1(doubleOutImage);
      adder->SetInput2(m_CompartmentImages.at(i));
      adder->Update();
      doubleOutImage = adder->GetOutput();
    }
  }
  if (this->GetAbortGenerateData())
  {
    PrintToLog("\n", false, false);
    PrintToLog("Simulation aborted");
    return;
  }

  PrintToLog("Finalizing image");
  if (m_Parameters.m_SignalGen.m_DoAddDrift && m_Parameters.m_SignalGen.m_Drift>0.0)
    PrintToLog("Adding signal drift: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_Drift), false);
  if (signalScale>1)
    PrintToLog("Scaling signal", false);
  if (m_Parameters.m_NoiseModel)
    PrintToLog("Adding noise: " + boost::lexical_cast<std::string>(m_Parameters.m_SignalGen.m_NoiseVariance), false);
  ImageRegionIterator<OutputImageType> it4 (m_OutputImage, m_OutputImage->GetLargestPossibleRegion());
  DoubleDwiType::PixelType signal; signal.SetSize(m_Parameters.m_SignalGen.GetNumVolumes());
  boost::progress_display disp2(m_OutputImage->GetLargestPossibleRegion().GetNumberOfPixels());

  PrintToLog("0%   10   20   30   40   50   60   70   80   90   100%", false, true, false);
  PrintToLog("|----|----|----|----|----|----|----|----|----|----|\n*", false, false, false);
  int lastTick = 0;

  while(!it4.IsAtEnd())
  {
    if (this->GetAbortGenerateData())
    {
      PrintToLog("\n", false, false);
      PrintToLog("Simulation aborted");
      return;
    }

    ++disp2;
    unsigned long newTick = 50*disp2.count()/disp2.expected_count();
    for (unsigned long tick = 0; tick<(newTick-lastTick); tick++)
      PrintToLog("*", false, false, false);
    lastTick = newTick;

    typename OutputImageType::IndexType index = it4.GetIndex();
    signal = doubleOutImage->GetPixel(index)*signalScale;

    for (unsigned int i=0; i<signal.Size(); i++)
    {
      if (m_Parameters.m_SignalGen.m_DoAddDrift)
      {
        double df = -m_Parameters.m_SignalGen.m_Drift/(signal.Size()*signal.Size());
        signal[i] += signal[i]*df*i*i;
      }
    }

    if (m_Parameters.m_NoiseModel)
      m_Parameters.m_NoiseModel->AddNoise(signal);

    for (unsigned int i=0; i<signal.Size(); i++)
    {
      if (signal[i]>0)
        signal[i] = floor(signal[i]+0.5);
      else
        signal[i] = ceil(signal[i]-0.5);
    }
    it4.Set(signal);
    ++it4;
  }
  this->SetNthOutput(0, m_OutputImage);

  PrintToLog("\n", false);
  PrintToLog("Finished simulation");
  m_TimeProbe.Stop();

  if (m_Parameters.m_SignalGen.m_DoAddMotion)
  {
    PrintToLog("\nHead motion log:", false);
    PrintToLog(m_MotionLog, false, false);
  }

  if (m_Parameters.m_Misc.m_DoAddSpikes && m_Parameters.m_SignalGen.m_Spikes>0)
  {
    PrintToLog("\nSpike log:", false);
    PrintToLog(m_SpikeLog, false, false);
  }

  if (m_Logfile.is_open())
    m_Logfile.close();
}


template< class PixelType >
void TractsToDWIImageFilter< PixelType >::PrintToLog(std::string m, bool addTime, bool linebreak, bool stdOut)
{
  // timestamp
  if (addTime)
  {
    if ( m_Logfile.is_open() )
      m_Logfile << this->GetTime() << " > ";
    m_StatusText += this->GetTime() + " > ";
    if (stdOut)
      std::cout << this->GetTime() << " > ";
  }

  // message
  if (m_Logfile.is_open())
    m_Logfile << m;
  m_StatusText += m;
  if (stdOut)
    std::cout << m;

  // new line
  if (linebreak)
  {
    if (m_Logfile.is_open())
      m_Logfile << "\n";
    m_StatusText += "\n";
    if (stdOut)
      std::cout << "\n";
  }
  if ( m_Logfile.is_open() )
    m_Logfile.flush();
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::SimulateMotion(int g)
{
  if ( m_Parameters.m_SignalGen.m_DoAddMotion &&
       m_Parameters.m_SignalGen.m_DoRandomizeMotion &&
       g>0 &&
       m_Parameters.m_SignalGen.m_MotionVolumes[g-1])
  {
    // The last volume was randomly moved, so we have to reset to fiberbundle and the mask.
    // Without motion or with linear motion, we keep the last position --> no reset.
    m_FiberBundleTransformed = m_FiberBundle->GetDeepCopy();

    if (m_MaskImageSet)
    {
      auto duplicator = itk::ImageDuplicator<ItkUcharImgType>::New();
      duplicator->SetInputImage(m_Parameters.m_SignalGen.m_MaskImage);
      duplicator->Update();
      m_TransformedMaskImage = duplicator->GetOutput();
    }
  }

  VectorType rotation;
  VectorType translation;

  // is motion artifact enabled?
  // is the current volume g affected by motion?
  if ( m_Parameters.m_SignalGen.m_DoAddMotion
       && m_Parameters.m_SignalGen.m_MotionVolumes[g]
       && g<static_cast<int>(m_Parameters.m_SignalGen.GetNumVolumes()) )
  {
    // adjust motion transforms
    if ( m_Parameters.m_SignalGen.m_DoRandomizeMotion )
    {
      // randomly
      rotation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[0]*2)
          -m_Parameters.m_SignalGen.m_Rotation[0];
      rotation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[1]*2)
          -m_Parameters.m_SignalGen.m_Rotation[1];
      rotation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Rotation[2]*2)
          -m_Parameters.m_SignalGen.m_Rotation[2];

      translation[0] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[0]*2)
          -m_Parameters.m_SignalGen.m_Translation[0];
      translation[1] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[1]*2)
          -m_Parameters.m_SignalGen.m_Translation[1];
      translation[2] = m_RandGen->GetVariateWithClosedRange(m_Parameters.m_SignalGen.m_Translation[2]*2)
          -m_Parameters.m_SignalGen.m_Translation[2];

      m_FiberBundleTransformed->TransformFibers(rotation[0], rotation[1], rotation[2], translation[0], translation[1], translation[2]);

    }
    else
    {
      // linearly
      rotation = m_Parameters.m_SignalGen.m_Rotation / m_NumMotionVolumes;
      translation = m_Parameters.m_SignalGen.m_Translation / m_NumMotionVolumes;
      m_MotionCounter++;

      m_FiberBundleTransformed->TransformFibers(rotation[0], rotation[1], rotation[2], translation[0], translation[1], translation[2]);

      rotation *= m_MotionCounter;
      translation *= m_MotionCounter;
    }
    MatrixType rotationMatrix = mitk::imv::GetRotationMatrixItk(rotation[0], rotation[1], rotation[2]);
    MatrixType rotationMatrixInv = mitk::imv::GetRotationMatrixItk(-rotation[0], -rotation[1], -rotation[2]);

    m_Rotations.push_back(rotationMatrix);
    m_RotationsInv.push_back(rotationMatrixInv);
    m_Translations.push_back(translation);

    // move mask image accoring to new transform
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
        m_TransformedMaskImage->TransformPhysicalPointToIndex(GetMovedPoint(index, true), index);

        if (m_TransformedMaskImage->GetLargestPossibleRegion().IsInside(index))
          m_TransformedMaskImage->SetPixel(index, 100);
        ++maskIt;
      }
    }
  }
  else
  {
    if (m_Parameters.m_SignalGen.m_DoAddMotion && !m_Parameters.m_SignalGen.m_DoRandomizeMotion && g>0)
    {
      rotation = m_Parameters.m_SignalGen.m_Rotation / m_NumMotionVolumes;
      rotation *= m_MotionCounter;
      m_Rotations.push_back(m_Rotations.back());
      m_RotationsInv.push_back(m_RotationsInv.back());
      m_Translations.push_back(m_Translations.back());
    }
    else
    {
      rotation.Fill(0.0);
      VectorType translation; translation.Fill(0.0);
      MatrixType rotation_matrix; rotation_matrix.SetIdentity();

      m_Rotations.push_back(rotation_matrix);
      m_RotationsInv.push_back(rotation_matrix);
      m_Translations.push_back(translation);
    }
  }

  if (m_Parameters.m_SignalGen.m_DoAddMotion)
  {
    m_MotionLog += boost::lexical_cast<std::string>(g) + " rotation: " + boost::lexical_cast<std::string>(rotation[0])
        + "," + boost::lexical_cast<std::string>(rotation[1])
        + "," + boost::lexical_cast<std::string>(rotation[2]) + ";";

    m_MotionLog += " translation: " + boost::lexical_cast<std::string>(m_Translations.back()[0])
        + "," + boost::lexical_cast<std::string>(m_Translations.back()[1])
        + "," + boost::lexical_cast<std::string>(m_Translations.back()[2]) + "\n";
  }
}

template< class PixelType >
itk::Point<float, 3> TractsToDWIImageFilter< PixelType >::GetMovedPoint(itk::Index<3>& index, bool forward)
{
  itk::Point<float, 3> transformed_point;
  float tx = m_Translations.back()[0];
  float ty = m_Translations.back()[1];
  float tz = m_Translations.back()[2];

  if (forward)
  {
    m_UpsampledMaskImage->TransformIndexToPhysicalPoint(index, transformed_point);
    m_FiberBundle->TransformPoint<>(transformed_point, m_Rotations.back(), tx, ty, tz);
  }
  else
  {
    tx *= -1; ty *= -1; tz *= -1;
    m_TransformedMaskImage->TransformIndexToPhysicalPoint(index, transformed_point);
    m_FiberBundle->TransformPoint<>(transformed_point, m_RotationsInv.back(), tx, ty, tz);
  }
  return transformed_point;
}

template< class PixelType >
void TractsToDWIImageFilter< PixelType >::
SimulateExtraAxonalSignal(ItkUcharImgType::IndexType& index, itk::Point<float, 3>& volume_fraction_point, double intraAxonalVolume, int g)
{
  int numFiberCompartments = m_Parameters.m_FiberModelList.size();
  int numNonFiberCompartments = m_Parameters.m_NonFiberModelList.size();

  if (m_Parameters.m_SignalGen.m_DoDisablePartialVolume)
  {
    // simulate signal for largest non-fiber compartment
    int max_compartment_index = 0;
    double max_fraction = 0;
    if (numNonFiberCompartments>1)
    {
      for (int i=0; i<numNonFiberCompartments; ++i)
      {
        m_DoubleInterpolator->SetInputImage(m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage());
        double compartment_fraction = mitk::imv::GetImageValue<double>(volume_fraction_point, true, m_DoubleInterpolator);
        if (compartment_fraction<0)
          mitkThrow() << "Volume fraction image (index " << i << ") contains values less than zero!";

        if (compartment_fraction>max_fraction)
        {
          max_fraction = compartment_fraction;
          max_compartment_index = i;
        }
      }
    }

    DoubleDwiType::Pointer doubleDwi = m_CompartmentImages.at(max_compartment_index+numFiberCompartments);
    DoubleDwiType::PixelType pix = doubleDwi->GetPixel(index);
    pix[g] += m_Parameters.m_NonFiberModelList[max_compartment_index]->SimulateMeasurement(g, m_NullDir)*m_VoxelVolume;
    doubleDwi->SetPixel(index, pix);

    if (g==0)
      m_VolumeFractions.at(max_compartment_index+numFiberCompartments)->SetPixel(index, 1);
  }
  else
  {
    std::vector< double > fractions;
    if (g==0)
      m_VolumeFractions.at(0)->SetPixel(index, intraAxonalVolume/m_VoxelVolume);

    double extraAxonalVolume = m_VoxelVolume-intraAxonalVolume;    // non-fiber volume
    if (extraAxonalVolume<0)
    {
      if (extraAxonalVolume<-0.001)
        MITK_ERROR << "Corrupted intra-axonal signal voxel detected. Fiber volume larger voxel volume! " << m_VoxelVolume << "<" << intraAxonalVolume;
      extraAxonalVolume = 0;
    }
    double interAxonalVolume = 0;
    if (numFiberCompartments>1)
      interAxonalVolume = extraAxonalVolume * intraAxonalVolume/m_VoxelVolume;   // inter-axonal fraction of non fiber compartment
    double nonFiberVolume = extraAxonalVolume - interAxonalVolume;        // rest of compartment
    if (nonFiberVolume<0)
    {
      if (nonFiberVolume<-0.001)
        MITK_ERROR << "Corrupted signal voxel detected. Fiber volume larger voxel volume!";
      nonFiberVolume = 0;
      interAxonalVolume = extraAxonalVolume;
    }

    double compartmentSum = intraAxonalVolume;
    fractions.push_back(intraAxonalVolume/m_VoxelVolume);

    // rescale extra-axonal fiber signal
    for (int i=1; i<numFiberCompartments; ++i)
    {
      if (m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage()!=nullptr)
      {
        m_DoubleInterpolator->SetInputImage(m_Parameters.m_FiberModelList[i]->GetVolumeFractionImage());
        interAxonalVolume = mitk::imv::GetImageValue<double>(volume_fraction_point, true, m_DoubleInterpolator)*m_VoxelVolume;
        if (interAxonalVolume<0)
          mitkThrow() << "Volume fraction image (index " << i+1 << ") contains negative values!";
      }

      DoubleDwiType::PixelType pix = m_CompartmentImages.at(i)->GetPixel(index);
      pix[g] *= interAxonalVolume;
      m_CompartmentImages.at(i)->SetPixel(index, pix);

      compartmentSum += interAxonalVolume;
      fractions.push_back(interAxonalVolume/m_VoxelVolume);
      if (g==0)
        m_VolumeFractions.at(i)->SetPixel(index, interAxonalVolume/m_VoxelVolume);
    }

    for (int i=0; i<numNonFiberCompartments; ++i)
    {
      double volume = nonFiberVolume;
      if (m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage()!=nullptr)
      {
        m_DoubleInterpolator->SetInputImage(m_Parameters.m_NonFiberModelList[i]->GetVolumeFractionImage());
        volume = mitk::imv::GetImageValue<double>(volume_fraction_point, true, m_DoubleInterpolator)*m_VoxelVolume;
        if (volume<0)
          mitkThrow() << "Volume fraction image (index " << numFiberCompartments+i+1 << ") contains negative values (non-fiber compartment)!";

        if (m_UseRelativeNonFiberVolumeFractions)
          volume *= nonFiberVolume/m_VoxelVolume;
      }

      DoubleDwiType::PixelType pix = m_CompartmentImages.at(i+numFiberCompartments)->GetPixel(index);
      pix[g] += m_Parameters.m_NonFiberModelList[i]->SimulateMeasurement(g, m_NullDir)*volume;
      m_CompartmentImages.at(i+numFiberCompartments)->SetPixel(index, pix);

      compartmentSum += volume;
      fractions.push_back(volume/m_VoxelVolume);
      if (g==0)
        m_VolumeFractions.at(i+numFiberCompartments)->SetPixel(index, volume/m_VoxelVolume);
    }

    if (compartmentSum/m_VoxelVolume>1.05)
    {
      MITK_ERROR << "Compartments do not sum to 1 in voxel " << index << " (" << compartmentSum/m_VoxelVolume << ")";
      for (auto val : fractions)
        MITK_ERROR << val;
    }
  }
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
