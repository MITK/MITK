/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPartialVolumeAnalysisClusteringCalculator.h"

#include "mitkImageAccessByItk.h"
#include "mitkImageToItk.h"
#include "itkScalarImageToHistogramGenerator.h"
#include "itkListSample.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define PVA_PI M_PI

namespace mitk
{

  PartialVolumeAnalysisClusteringCalculator::PartialVolumeAnalysisClusteringCalculator()
    : m_MaxIt(100), m_StepsNumIntegration(100)
  {
  }


  PartialVolumeAnalysisClusteringCalculator::~PartialVolumeAnalysisClusteringCalculator()
  {
  }

  PartialVolumeAnalysisClusteringCalculator::ParamsType*
      PartialVolumeAnalysisClusteringCalculator::InitialGuess(HistType h) const
  {
    int s = h.xVals.size();
    int s30 = 0.3 * s;
    int s70 = 0.7 * s;
    ParamsType* params = new ParamsType();
    params->means[0] = h.xVals(s30);
    params->means[1] = h.xVals(s70);
    params->sigmas[0] = (h.xVals(s-1) - h.xVals(0)) / 5.0;
    params->sigmas[1] = (h.xVals(s-1) - h.xVals(0)) / 5.0;
    params->ps[0] = 0.4;
    params->ps[1] = 0.4;

    return params;
  }

  PartialVolumeAnalysisClusteringCalculator::ParamsType*
      PartialVolumeAnalysisClusteringCalculator::Cluster(HistType h, ParamsType *initialGuess) const
  {
    ParamsType *params = new ParamsType();
    params->Initialize(initialGuess);

    double ll = 9999999999999999999.9;
    double oll;
    double sml = (h.xVals[1] - h.xVals[0]) / 1000;

    int arraysz = h.xVals.size();

    for (unsigned int it = 0; it < m_MaxIt; it++)
    {
      // wie sehen basisfunktionen zu aktuellen params aus?
      ClusterResultType curves  = CalculateCurves(*params,h.xVals);

      // summe der basisfunktionen
      for(int i=0; i<arraysz; i++)
      {
        if(curves.combiVals(i) == 0)
        {
          curves.combiVals(i) = 0.00000000000000000001;
        }
      }

      // alte log likelihood
      oll  = ll;
      ll = 0;

      // neue likelihood
      for(int i=0; i<arraysz; i++)
      {
        ll -= h.hVals(i) * log(curves.combiVals(i));
      }

      if( it>2 &&  oll-ll < arraysz/2*1e-15 )
      {
        break;
      }

      for(int j=0; j<2; j++)
      {
        VecType array, p(arraysz);
        array = curves.vals[j];

        for(int i=0; i<arraysz; i++)
        {
          p(i) = h.hVals(i)*array(i)/curves.combiVals(i);
        }

        params->ps[j] = 0;
        params->means[j] = 0;
        for(int i=0; i<arraysz; i++)
        {
          params->ps[j] += p(i);
          params->means[j] += h.xVals(i)*p(i);
        }
        params->means[j] /= params->ps[j];

        VecType vr = h.xVals;
        for(int i=0; i<arraysz; i++)
        {
          vr(i) -= params->means[j];
        }

        params->sigmas[j] = 0;
        for(int i=0; i<arraysz; i++)
        {
          params->sigmas[j] += vr(i)*vr(i)*p(i);
        }
        params->sigmas[j] /= params->ps[j];
        params->sigmas[j] += sml;

      }

      double p3 = 0;
      for(int i=0; i<arraysz; i++)
      {
        p3 += h.hVals(i)*curves.mixedVals[0](i)/curves.combiVals(i);
      }
      p3 += 1e-3;

      double sum = 0;
      for(int j=0; j<2; j++)
      {
        params->ps[j] = params->ps[j] + 1e-3;
        sum += params->ps[j];
      }
      sum += p3;

      for(int j=0; j<2; j++)
      {
        params->ps[j] = params->ps[j] / sum;
      }
      p3 /= sum;

    }

    return params;

  }

  void PartialVolumeAnalysisClusteringCalculator::Normalize(ParamsType params, ClusterResultType* curves) const
  {
    double sum1=0, sum2=0, sum3=0;
    int arraysz = curves->vals[0].size();

    for(int i=0; i<arraysz; i++)
    {
      sum1 += curves->vals[0](i);
      sum2 += curves->vals[1](i);
      sum3 += curves->mixedVals[0](i);
    }

    sum1 /= params.ps[0];
    sum2 /= params.ps[1];
    sum3 /= 1.0 -params.ps[0] -params.ps[1];

    for(int i=0; i<arraysz; i++)
    {
      curves->vals[0](i) /= sum1;
      curves->vals[1](i) /= sum2;
      curves->mixedVals[0](i) /= sum3;
    }

    for(int i=0; i<arraysz; i++)
    {
      curves->combiVals(i) = curves->mixedVals[0](i) + curves->vals[0](i) + curves->vals[1](i);
    }
  }

  PartialVolumeAnalysisClusteringCalculator::ClusterResultType
      PartialVolumeAnalysisClusteringCalculator::CalculateCurves(ParamsType params, VecType xVals) const
  {

    int arraysz = xVals.size();
    ClusterResultType result(arraysz);

    for( int j=0; j<2; j++)
    {
      for(int i=0; i<arraysz; i++)
      {
        double d   = xVals(i)-params.means[j];
        double amp = params.ps[j]/sqrt(2*PVA_PI*params.sigmas[j]);

        result.vals[j](i) = amp*exp(-0.5 * (d*d)/params.sigmas[j]);
      }
    }

    for(int i=0; i<arraysz; i++)
    {
      result.mixedVals[0](i) = 0;
    }

    for(double t=0; t<=1; t = t + 1.0/(m_StepsNumIntegration-1.0))
    {
      for(int i=0; i<arraysz; i++)
      {
        double d = xVals(i)-(t*params.means[0]+(1-t)*params.means[1]);
        double v = t*params.sigmas[0]+(1-t)*params.sigmas[1];
        double p = 1.0 - params.ps[0] - params.ps[1];
        double amp = (1.0/m_StepsNumIntegration) * p / sqrt(2.0*PVA_PI*v);

        result.mixedVals[0](i) = result.mixedVals[0](i) + amp*exp(-0.5 * (d*d)/v);

        //        MITK_INFO << "d=" << d << "v=" <<v << "p=" << p << "amp=" << amp << "result=" << amp*exp(-0.5 * (d*d)/v) << std::endl;
      }
      //      MITK_INFO << "t=" << t << std::endl;
      //      params.Print();
      //      result.Print();
    }

    for(int i=0; i<arraysz; i++)
    {
      result.combiVals(i) = result.mixedVals[0](i) + result.vals[0](i) + result.vals[1](i);
    }

    return result;
  }

  PartialVolumeAnalysisClusteringCalculator::HelperStructPerformRGBClusteringRetval*
      PartialVolumeAnalysisClusteringCalculator::PerformRGBClustering(mitk::Image::ConstPointer image, const MitkHistType *histogram) const
  {

    HelperStructRGBChannels *rgbChannels = new HelperStructRGBChannels();

    HelperStructPerformClusteringRetval *resultr = PerformClustering(image, histogram, 2);
    rgbChannels->r = resultr->clusteredImage;

    HelperStructPerformClusteringRetval *resultg = PerformClustering(image, histogram, 0, resultr);
    rgbChannels->g = resultg->clusteredImage;

    HelperStructPerformClusteringRetval *resultb = PerformClustering(image, histogram, 1, resultr);
    rgbChannels->b = resultb->clusteredImage;

    mitk::Image::Pointer outImage = mitk::Image::New();

    AccessFixedDimensionByItk_2(
        rgbChannels->r.GetPointer(),
        InternalGenerateRGB,
        3,
        rgbChannels,
        outImage);

    HelperStructPerformRGBClusteringRetval *retval
        = new HelperStructPerformRGBClusteringRetval();
    retval->rgbChannels = rgbChannels;
    retval->rgb     = outImage;
    retval->params  = resultr->params;
    retval->result  = resultr->result;
    retval->hist    = resultr->hist;

    delete resultr;
    delete resultg;

    return retval;

  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisClusteringCalculator::InternalGenerateRGB(
          const itk::Image< TPixel, VImageDimension > *image,
          HelperStructRGBChannels *rgbin, mitk::Image::Pointer retval ) const
  {
    typedef itk::Image< float, VImageDimension > ProbImageType;
    typedef itk::Image< typename itk::RGBAPixel<unsigned char>, VImageDimension > RGBImageType;

    typedef mitk::ImageToItk<ProbImageType> CastFilterType;
    typename CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput( rgbin->r );
    castFilter->Update();
    typename ProbImageType::Pointer r = castFilter->GetOutput();

    castFilter = CastFilterType::New();
    castFilter->SetInput( rgbin->g );
    castFilter->Update();
    typename ProbImageType::Pointer g = castFilter->GetOutput();

    typename RGBImageType::Pointer rgb = RGBImageType::New();
    rgb->SetSpacing( g->GetSpacing() );   // Set the image spacing
    rgb->SetOrigin( g->GetOrigin() );     // Set the image origin
    rgb->SetDirection( g->GetDirection() );  // Set the image direction
    rgb->SetRegions( g->GetLargestPossibleRegion() );
    rgb->Allocate();

    itk::ImageRegionConstIterator<ProbImageType>
        itr(r, r->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<ProbImageType>
        itg(g, g->GetLargestPossibleRegion());

    itk::ImageRegionIterator<RGBImageType>
        itrgb(rgb, rgb->GetLargestPossibleRegion());

    itr = itr.Begin();
    itg = itg.Begin();

    float maxr = 0;
    float maxg = 0;

    while( !itr.IsAtEnd() )
    {
      typename ProbImageType::PixelType pr = itr.Get();
      typename ProbImageType::PixelType pg = itg.Get();

      if(pr > maxr)
      {
        maxr = pr;
      }

      if(pg > maxg)
      {
        maxg = pg;
      }

      ++itr;
      ++itg;
    }

    itr = itr.Begin();
    itg = itg.Begin();
    itrgb = itrgb.Begin();

    while( !itr.IsAtEnd() )
    {
      typename ProbImageType::PixelType pr = itr.Get();
      typename ProbImageType::PixelType pg = itg.Get();

      typename RGBImageType::PixelType prgb;

      float valr = (pr/maxr)*255.0f;
      float valg = (pg/maxg)*255.0f;
      float alpha = valr>valg ? valr : valg;
      prgb.Set(valr, valg, 0.0f, alpha);

      itrgb.Set(prgb);

      ++itr;
      ++itg;
      ++itrgb;
    }

    retval->InitializeByItk(rgb.GetPointer());
    retval->SetVolume(rgb->GetBufferPointer());

  }


  PartialVolumeAnalysisClusteringCalculator::HelperStructPerformClusteringRetval*
      PartialVolumeAnalysisClusteringCalculator::PerformClustering(mitk::Image::ConstPointer image, const MitkHistType *histogram, int classIdent, HelperStructPerformClusteringRetval* precResult) const
  {

    HelperStructPerformClusteringRetval *retval =
        new HelperStructPerformClusteringRetval();

    if(precResult == 0)
    {
      retval->hist = new HistType();
      retval->hist->InitByMitkHistogram(histogram);

      ParamsType params;
      params.Initialize( Cluster(*(retval->hist)) );
      ClusterResultType result = CalculateCurves(params,retval->hist->xVals);
      Normalize(params, &result);

      retval->params = new ParamsType();
      retval->params->Initialize(&params);
      retval->result = new ClusterResultType(10);
      retval->result->Initialize(&result);
    }
    else
    {
      retval->params = new ParamsType();
      retval->params->Initialize(precResult->params);
      retval->result = new ClusterResultType(10);
      retval->result->Initialize(precResult->result);
    }

    VecType totalProbs = retval->result->combiVals;
    VecType pvProbs    = retval->result->mixedVals[0];
    VecType fiberProbs;
    VecType nonFiberProbs;
    VecType interestingProbs;
    double p_fiber;
    double p_nonFiber;
    double p_interesting;
//    if(retval->params->means[0]<retval->params->means[1])
//    {
      fiberProbs    = retval->result->vals[1];
      nonFiberProbs = retval->result->vals[0];
      p_fiber       = retval->params->ps[1];
      p_nonFiber    = retval->params->ps[0];
//    }
//    else
//    {
//      fiberProbs    = retval->result->vals[0];
//      nonFiberProbs = retval->result->vals[1];
//      p_fiber       = retval->params->ps[0];
//      p_nonFiber    = retval->params->ps[1];
//    }

    switch(classIdent)
    {
    case 0:
      interestingProbs = nonFiberProbs;
      p_interesting = p_nonFiber;
      break;
    case 1:
      interestingProbs = pvProbs;
      p_interesting = 1 - p_fiber - p_nonFiber;
      break;
    case 2:
    default:
      interestingProbs = fiberProbs;
      p_interesting = p_fiber;
      break;
    }

    double sum = histogram->GetTotalFrequency();

    // initialize two histograms for class and total probabilities
    MitkHistType::MeasurementVectorType min;
    MitkHistType::MeasurementVectorType max;
    min.Fill(histogram->GetDimensionMins(0)[0]);
    max.Fill(histogram->GetDimensionMaxs(0)[histogram->GetDimensionMaxs(0).size()-1]);

    MitkHistType::Pointer interestingHist = MitkHistType::New();
    interestingHist->Initialize(histogram->GetSize(),min,max);
    MitkHistType::Iterator newIt = interestingHist->Begin();
    MitkHistType::Iterator newEnd = interestingHist->End();

    MitkHistType::Pointer totalHist = MitkHistType::New();
    totalHist->Initialize(histogram->GetSize(),min,max);
    MitkHistType::Iterator totalIt = totalHist->Begin();
    MitkHistType::Iterator totalEnd = totalHist->End();

    int i=0;
    while (newIt != newEnd)
    {
      newIt.SetFrequency(interestingProbs(i)*sum);
      totalIt.SetFrequency(totalProbs(i)*sum);
      ++newIt;
      ++totalIt;
      ++i;
    }

    mitk::Image::Pointer outImage1 = mitk::Image::New();
    mitk::Image::Pointer outImage2 = mitk::Image::New();

    HelperStructClusteringResults clusterResults;
    clusterResults.interestingHist = interestingHist;
    clusterResults.totalHist = totalHist;
    clusterResults.p_interesting = p_interesting;

    AccessFixedDimensionByItk_3(
        image.GetPointer(),
        InternalGenerateProbabilityImage,
        3,
        clusterResults,
        outImage1, outImage2);

    retval->clusteredImage = outImage1;
    retval->displayImage = outImage2;

    return retval;

  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisClusteringCalculator::InternalGenerateProbabilityImage(
          const itk::Image< TPixel, VImageDimension > *image,
          const HelperStructClusteringResults clusterResults,
          mitk::Image::Pointer outImage1, mitk::Image::Pointer outImage2 ) const
  {

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< itk::RGBAPixel<unsigned char>, VImageDimension > DisplayImageType;
    typedef itk::Image< float, VImageDimension > ProbImageType;

    typename ProbImageType::Pointer probimage = ProbImageType::New();
    probimage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    probimage->SetOrigin( image->GetOrigin() );     // Set the image origin
    probimage->SetDirection( image->GetDirection() );  // Set the image direction
    probimage->SetRegions( image->GetLargestPossibleRegion() );
    probimage->Allocate();
    probimage->FillBuffer(0);

    typename DisplayImageType::Pointer displayimage = DisplayImageType::New();
    displayimage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    displayimage->SetOrigin( image->GetOrigin() );     // Set the image origin
    displayimage->SetDirection( image->GetDirection() );  // Set the image direction
    displayimage->SetRegions( image->GetLargestPossibleRegion() );
    displayimage->Allocate();

    typename DisplayImageType::PixelType rgba;
    rgba.Set(0.0f, 0.0f, 0.0f, 0.0f);
    displayimage->FillBuffer(rgba);

    itk::ImageRegionConstIterator<ImageType>
        itimage(image, image->GetLargestPossibleRegion());

    itk::ImageRegionIterator<ProbImageType>
        itprob(probimage, probimage->GetLargestPossibleRegion());

    itk::ImageRegionIterator<DisplayImageType>
        itdisp(displayimage, displayimage->GetLargestPossibleRegion());

    itimage = itimage.Begin();
    itprob = itprob.Begin();

    MitkHistType::IndexType index;
    float maxp = 0;
    while( !itimage.IsAtEnd() )
    {
      if(itimage.Get())
      {
        MitkHistType::MeasurementVectorType meas;
        meas.Fill(itimage.Get());
        double aposteriori = 0;
        bool success = clusterResults.interestingHist->GetIndex(meas, index );
        if(success)
        {
          double aprioriProb = clusterResults.interestingHist->GetFrequency(index);
          double intensityProb = clusterResults.totalHist->GetFrequency(index);
          double p_interesting = clusterResults.p_interesting;
          aposteriori = p_interesting * aprioriProb / intensityProb;
        }
        else
        {
          MITK_ERROR << "index not found in histogram";
        }

        if(aposteriori > 0.0000000000000001)
        {
          itprob.Set( aposteriori );
          maxp = aposteriori > maxp ? aposteriori : maxp;
        }
        else
        {
          itprob.Set(0.0f);
        }
      }

      ++itimage;
      ++itprob;
    }

    itprob = itprob.Begin();
    itdisp = itdisp.Begin();

    while( !itprob.IsAtEnd() )
    {
      if(itprob.Get())
      {
        typename DisplayImageType::PixelType rgba;
        rgba.Set(255.0f, 0.0f, 0.0f, 255.0f*(itprob.Get()/maxp));
        itdisp.Set( rgba );
      }
      ++itprob;
      ++itdisp;
    }

    outImage1->InitializeByItk(probimage.GetPointer());
    outImage1->SetVolume(probimage->GetBufferPointer());

    outImage2->InitializeByItk(displayimage.GetPointer());
    outImage2->SetVolume(displayimage->GetBufferPointer());

  }


  double* PartialVolumeAnalysisClusteringCalculator::PerformQuantification(
      mitk::Image::ConstPointer image, mitk::Image::Pointer clusteredImage, mitk::Image::Pointer mask) const
  {

    double *retval = new double[2];

    AccessFixedDimensionByItk_3(
        image.GetPointer(),
        InternalQuantify,
        3,
        clusteredImage.GetPointer(),
        retval, mask );

    return retval;

  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisClusteringCalculator::InternalQuantify(
          const itk::Image< TPixel, VImageDimension > *image,
          mitk::Image::ConstPointer clusteredImage, double* retval, mitk::Image::Pointer mask ) const
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< float, VImageDimension > ProbImageType;
    typedef itk::Image< unsigned char, VImageDimension > MaskImageType;

    typedef mitk::ImageToItk<ProbImageType> CastFilterType;
    typename CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput( clusteredImage );
    castFilter->Update();
    typename ProbImageType::Pointer clusterImage = castFilter->GetOutput();

    typename MaskImageType::Pointer itkmask = 0;
    if(mask.IsNotNull())
    {
      typedef mitk::ImageToItk<MaskImageType> CastFilterType2;
      typename CastFilterType2::Pointer castFilter2 = CastFilterType2::New();
      castFilter2->SetInput( mask );
      castFilter2->Update();
      itkmask = castFilter2->GetOutput();
    }
    else
    {
      itkmask = MaskImageType::New();
      itkmask->SetSpacing( clusterImage->GetSpacing() );   // Set the image spacing
      itkmask->SetOrigin( clusterImage->GetOrigin() );     // Set the image origin
      itkmask->SetDirection( clusterImage->GetDirection() );  // Set the image direction
      itkmask->SetRegions( clusterImage->GetLargestPossibleRegion() );
      itkmask->Allocate();
      itkmask->FillBuffer(1);
    }

    itk::ImageRegionConstIterator<ImageType>
        itimage(image, image->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<ProbImageType>
        itprob(clusterImage, clusterImage->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<MaskImageType>
        itmask(itkmask, itkmask->GetLargestPossibleRegion());

    itimage = itimage.Begin();
    itprob = itprob.Begin();
    itmask = itmask.Begin();

    double totalProb = 0;
    double measurement = 0;
    double error = 0;

    while( !itimage.IsAtEnd() && !itprob.IsAtEnd() && !itmask.IsAtEnd() )
    {
      double valImag = itimage.Get();
      double valProb = itprob.Get();
      double valMask = itmask.Get();

      typename ProbImageType::PixelType prop = valProb * valMask;

      totalProb   += prop;
      measurement += valImag * prop;
      error       += valImag * valImag * prop;

      ++itimage;
      ++itprob;
      ++itmask;
    }

    measurement = measurement / totalProb;
    error       = error       / totalProb;
    retval[0]   = measurement;
    retval[1]   = sqrt( error - measurement*measurement );

  }


  mitk::Image::Pointer PartialVolumeAnalysisClusteringCalculator::CaculateAngularErrorImage(
      mitk::Image::Pointer comp1, mitk::Image::Pointer comp2, mitk::Image::Pointer probImg) const
  {

    // cast input images to itk
    typedef itk::Image<float, 3> ImageType;
    typedef mitk::ImageToItk<ImageType> CastType;
    CastType::Pointer caster = CastType::New();
    caster->SetInput(comp1);
    caster->Update();
    ImageType::Pointer comp1Image = caster->GetOutput();

    caster = CastType::New();
    caster->SetInput(comp2);
    caster->Update();
    ImageType::Pointer comp2Image = caster->GetOutput();

    caster = CastType::New();
    caster->SetInput(probImg);
    caster->Update();
    ImageType::Pointer probImage = caster->GetOutput();

    // figure out maximum probability for fiber class
    float maxProb = 0;
    itk::ImageRegionConstIterator<ImageType>
        itprob(probImage, probImage->GetLargestPossibleRegion());
    itprob = itprob.Begin();
    while( !itprob.IsAtEnd() )
    {
      maxProb = itprob.Get() > maxProb ? itprob.Get() : maxProb;
      ++itprob;
    }

    // generate a list sample of angles at positions
    // where the fiber-prob is higher than .2*maxprob
    typedef float MeasurementType;
    const unsigned int MeasurementVectorLength = 2;
    typedef itk::Vector< MeasurementType , MeasurementVectorLength >
                                                                 MeasurementVectorType;
    typedef itk::Statistics::ListSample< MeasurementVectorType > ListSampleType;
    ListSampleType::Pointer listSample = ListSampleType::New();
    listSample->SetMeasurementVectorSize( MeasurementVectorLength );

    itk::ImageRegionIterator<ImageType>
        it1(comp1Image, comp1Image->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ImageType>
        it2(comp2Image, comp2Image->GetLargestPossibleRegion());

    it1 = it1.Begin();
    it2 = it2.Begin();
    itprob = itprob.Begin();

    while( !itprob.IsAtEnd() )
    {
      if(itprob.Get() > 0.2 * maxProb)
      {
        MeasurementVectorType mv;
        mv[0] = ( MeasurementType ) it1.Get();
        mv[1] = ( MeasurementType ) it2.Get();
        listSample->PushBack(mv);
      }
      ++it1;
      ++it2;
      ++itprob;
    }

    // generate a histogram from the list sample
    typedef float HistogramMeasurementType;
    typedef itk::Statistics::ListSampleToHistogramGenerator
        < ListSampleType, HistogramMeasurementType,
        itk::Statistics::DenseFrequencyContainer,
        MeasurementVectorLength > GeneratorType;
    GeneratorType::Pointer generator = GeneratorType::New();

    GeneratorType::HistogramType::SizeType size;
    size.Fill(30);
    generator->SetNumberOfBins( size );

    generator->SetListSample( listSample );
    generator->SetMarginalScale( 10.0 );

    MeasurementVectorType min;
    min[0] = ( MeasurementType ) 0;
    min[1] = ( MeasurementType ) 0;
    generator->SetHistogramMin(min);

    MeasurementVectorType max;
    max[0] = ( MeasurementType ) PVA_PI;
    max[1] = ( MeasurementType ) PVA_PI;
    generator->SetHistogramMax(max);

    generator->Update();

    // look for frequency mode in the histogram
    GeneratorType::HistogramType::ConstPointer histogram = generator->GetOutput();
    GeneratorType::HistogramType::ConstIterator iter = histogram->Begin();
    float maxFreq = 0;
    MeasurementVectorType maxValue;
    while ( iter != histogram->End() )
    {
      if(iter.GetFrequency() > maxFreq)
      {
        maxFreq = iter.GetFrequency();
        maxValue[0] = iter.GetMeasurementVector()[0];
        maxValue[1] = iter.GetMeasurementVector()[1];
      }
      ++iter;
    }

    // generate return image that contains the angular
    // error of the voxels to the histogram max measurement
    ImageType::Pointer returnImage = ImageType::New();
    returnImage->SetSpacing( comp1Image->GetSpacing() );   // Set the image spacing
    returnImage->SetOrigin( comp1Image->GetOrigin() );     // Set the image origin
    returnImage->SetDirection( comp1Image->GetDirection() );  // Set the image direction
    returnImage->SetRegions( comp1Image->GetLargestPossibleRegion() );
    returnImage->Allocate();

    itk::ImageRegionConstIterator<ImageType>
        cit1(comp1Image, comp1Image->GetLargestPossibleRegion());

    itk::ImageRegionConstIterator<ImageType>
        cit2(comp2Image, comp2Image->GetLargestPossibleRegion());

    itk::ImageRegionIterator<ImageType>
        itout(returnImage, returnImage->GetLargestPossibleRegion());

    cit1 = cit1.Begin();
    cit2 = cit2.Begin();
    itout = itout.Begin();

    vnl_vector<float> v(3);
    v[0] = cos( maxValue[0] ) * sin( maxValue[1] );
    v[1] = sin( maxValue[0] ) * sin( maxValue[1] );
    v[2] = cos( maxValue[1] );
//    MITK_INFO << "max vector: " << v;
    while( !cit1.IsAtEnd() )
    {
      vnl_vector<float> v1(3);
      v1[0] = cos( cit1.Get() ) * sin( cit2.Get() );
      v1[1] = sin( cit1.Get() ) * sin( cit2.Get() );
      v1[2] = cos( cit2.Get() );

      itout.Set(fabs(angle(v,v1)));
//      MITK_INFO << "ang_error " << v1 << ": " << fabs(angle(v,v1));

      ++cit1;
      ++cit2;
      ++itout;
    }

    mitk::Image::Pointer retval = mitk::Image::New();
    retval->InitializeByItk(returnImage.GetPointer());
    retval->SetVolume(returnImage->GetBufferPointer());
    return retval;
  }

  PartialVolumeAnalysisClusteringCalculator::HelperStructPerformRGBClusteringRetval*
      PartialVolumeAnalysisClusteringCalculator::PerformRGBQuantiles(mitk::Image::ConstPointer image, const MitkHistType *histogram, double p1, double p2) const
  {

    HelperStructRGBChannels *rgbChannels = new HelperStructRGBChannels();

    HelperStructPerformClusteringRetval *resultr = PerformQuantiles(image, histogram, p2, 999999999.0 );
    rgbChannels->r = resultr->clusteredImage;

    HelperStructPerformClusteringRetval *resultg = PerformQuantiles(image, histogram, -999999999.0, p1 );
    rgbChannels->g = resultg->clusteredImage;

    HelperStructPerformClusteringRetval *resultb = PerformQuantiles(image, histogram, p1, p2 );
    rgbChannels->b = resultb->clusteredImage;

    mitk::Image::Pointer outImage = mitk::Image::New();

    AccessFixedDimensionByItk_2(
        rgbChannels->r.GetPointer(),
        InternalGenerateRGB,
        3,
        rgbChannels,
        outImage);

    HelperStructPerformRGBClusteringRetval *retval
        = new HelperStructPerformRGBClusteringRetval();
    retval->rgbChannels = rgbChannels;
    retval->rgb     = outImage;
    retval->params  = resultr->params;
    retval->result  = resultr->result;
    retval->hist    = resultr->hist;

    delete resultr;
    delete resultg;

    return retval;

  }

  PartialVolumeAnalysisClusteringCalculator::HelperStructPerformClusteringRetval*
      PartialVolumeAnalysisClusteringCalculator::PerformQuantiles(mitk::Image::ConstPointer image, const MitkHistType *histogram, double p1, double p2 ) const
  {

    HelperStructPerformClusteringRetval *retval =
        new HelperStructPerformClusteringRetval();

    retval->hist = new HistType();
    retval->hist->InitByMitkHistogram(histogram);

    double sum = histogram->GetTotalFrequency();

    double* q = new double[2];
    q[0] = histogram->Quantile(0, p1);
    q[1] = histogram->Quantile(0, p2);

    mitk::Image::Pointer outImage1 = mitk::Image::New();
    mitk::Image::Pointer outImage2 = mitk::Image::New();

    AccessFixedDimensionByItk_3(
        image.GetPointer(),
        InternalGenerateQuantileImage,
        3, q,
        outImage1, outImage2);

    retval->clusteredImage = outImage1;
    retval->displayImage = outImage2;

    delete[] q;
    return retval;

  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisClusteringCalculator::InternalGenerateQuantileImage(
          const itk::Image< TPixel, VImageDimension > *image, double* q,
          mitk::Image::Pointer outImage1, mitk::Image::Pointer outImage2 ) const
  {

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< itk::RGBAPixel<unsigned char>, VImageDimension > DisplayImageType;
    typedef itk::Image< float, VImageDimension > ProbImageType;

    typename ProbImageType::Pointer probimage = ProbImageType::New();
    probimage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    probimage->SetOrigin( image->GetOrigin() );     // Set the image origin
    probimage->SetDirection( image->GetDirection() );  // Set the image direction
    probimage->SetRegions( image->GetLargestPossibleRegion() );
    probimage->Allocate();
    probimage->FillBuffer(0);

    typename DisplayImageType::Pointer displayimage = DisplayImageType::New();
    displayimage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    displayimage->SetOrigin( image->GetOrigin() );     // Set the image origin
    displayimage->SetDirection( image->GetDirection() );  // Set the image direction
    displayimage->SetRegions( image->GetLargestPossibleRegion() );
    displayimage->Allocate();

    typename DisplayImageType::PixelType rgba;
    rgba.Set(0.0f, 0.0f, 0.0f, 0.0f);
    displayimage->FillBuffer(rgba);

    itk::ImageRegionConstIterator<ImageType>
        itimage(image, image->GetLargestPossibleRegion());

    itk::ImageRegionIterator<ProbImageType>
        itprob(probimage, probimage->GetLargestPossibleRegion());

    itk::ImageRegionIterator<DisplayImageType>
        itdisp(displayimage, displayimage->GetLargestPossibleRegion());

    itimage = itimage.Begin();
    itprob = itprob.Begin();

    while( !itimage.IsAtEnd() )
    {
      if(itimage.Get() > q[0] && itimage.Get() < q[1])
      {
        itprob.Set(1.0f);
      }

      ++itimage;
      ++itprob;
    }

    itprob = itprob.Begin();
    itdisp = itdisp.Begin();

    while( !itprob.IsAtEnd() )
    {
      if(itprob.Get())
      {
        typename DisplayImageType::PixelType rgba;
        rgba.Set(255.0f, 0.0f, 0.0f, 255.0f);
        itdisp.Set( rgba );
      }
      ++itprob;
      ++itdisp;
    }

    outImage1->InitializeByItk(probimage.GetPointer());
    outImage1->SetVolume(probimage->GetBufferPointer());

    outImage2->InitializeByItk(displayimage.GetPointer());
    outImage2->SetVolume(displayimage->GetBufferPointer());

  }

}
