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


#ifndef _MITK_PartialVolumeAnalysisClusteringCalculator_H
#define _MITK_PartialVolumeAnalysisClusteringCalculator_H

#include "MitkDiffusionImagingExports.h"

#include "mitkCommon.h"
#include "mitkImage.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkImage.h>
#include <itkRGBAPixel.h>

#include <vnl/vnl_vector.h>

namespace mitk
{

  class MitkDiffusionImaging_EXPORT PartialVolumeAnalysisClusteringCalculator : public itk::Object
  {
  public:

    typedef vnl_vector<double> VecType;
    typedef mitk::Image::HistogramType MitkHistType;

    class HistType
    {
    public:

      HistType()
      {
      }

      HistType(const HistType& p)
      {
        this->xVals = p.xVals;
        this->hVals = p.hVals;
      }

      ~HistType()
      {
      }

      HistType operator= (const HistType& p)
                         {
        if (this != &p) // protect against invalid self-assignment
        {
          this->xVals = p.xVals;
          this->hVals = p.hVals;
        }
        return *this;
      }

      void InitByMitkHistogram(const MitkHistType* histogram)
      {
        xVals.set_size(histogram->GetSize()[0]);
        hVals.set_size(histogram->GetSize()[0]);
        double sum = histogram->GetTotalFrequency();
        MitkHistType::ConstIterator endIt = histogram->End();
        MitkHistType::ConstIterator it;
        bool firstNonEmptyBinFound = false;
        it = histogram->Begin();
        //++it;
        int i=0;
        while (it != endIt)
        {
          if(it.GetFrequency() || firstNonEmptyBinFound)
          {
            firstNonEmptyBinFound = true;
            xVals(i) = it.GetMeasurementVector().GetElement(0);
            hVals(i) = it.GetFrequency()/sum;
          }
          ++i;
          ++it;
        }

      }

      void SetXVals(VecType x)
      {xVals = x;}

      void SetHVals(VecType h)
      {hVals = h;}

      void SetXVals(std::vector<double> x)
      {
        int s = x.size();
        xVals.set_size(s);
        for(int i=0; i<s; i++)
        {
          xVals(i) = x[i];
        }
      }

      void SetHVals(std::vector<double> h)
      {
        int s = h.size();
        hVals.set_size(s);
        for(int i=0; i<s; i++)
        {
          hVals(i) = h[i];
        }
      }

      std::vector<double>* GetXVals()
      {
        int s = xVals.size();
        std::vector<double>* retval = new std::vector<double>(s);
        for(int i=0; i<s; i++)
        {
          (*retval)[i] = xVals(i);
        }
        return retval;
      }

      std::vector<double>* GetHVals()
      {
        int s = hVals.size();
        std::vector<double>* retval = new std::vector<double>(s);
        for(int i=0; i<s; i++)
        {
          (*retval)[i] = hVals(i);
        }
        return retval;
      }

      void Print(int nr=10)
      {
        int sz = xVals.size();
        int incr = (int)((1.0*sz)/(1.0*nr));
      }

      VecType xVals;
      VecType hVals;
    };

    class ClusterResultType
    {
    public:

      //      ClusterResultType()
      //      {
      //        vals.push_back(VecType());
      //        vals.push_back(VecType());
      //        mixedVals.push_back(VecType());
      //      }

      ClusterResultType(int sz)
      {
        vals.push_back(VecType(sz));
        vals.push_back(VecType(sz));
        mixedVals.push_back(VecType(sz));
        combiVals.set_size(sz);
      }

      ~ClusterResultType()
      {
      }

//      ClusterResultType operator= (const ClusterResultType *p)
//      {

//        MITK_DEBUG << "AOJHIFAHFOF";
//        if (this != &p) // protect against invalid self-assignment
//        {
//          MITK_DEBUG << "HRRRHRHRR";
//          this->vals.clear();
//          this->mixedVals.clear();

//          int s = p.vals.size();
//          for(int i=0; i<s;i++)
//          {
//            VecType v = p.vals[i];
//            this->vals.push_back(v);
//          }

//          s = p.mixedVals.size();
//          for(int i=0; i<s;i++)
//          {
//            VecType v = p.mixedVals[i];
//            this->mixedVals.push_back(v);
//          }

//          this->combiVals = p.combiVals;
//        }
//        return *this;
//      }

      void Initialize (const ClusterResultType *p)
      {
        if (this != p) // protect against invalid self-assignment
        {
          this->vals.clear();
          this->mixedVals.clear();

          int s = p->vals.size();
          for(int i=0; i<s;i++)
          {
            VecType v = p->vals[i];
            this->vals.push_back(v);
          }

          s = p->mixedVals.size();
          for(int i=0; i<s;i++)
          {
            VecType v = p->mixedVals[i];
            this->mixedVals.push_back(v);
          }

          this->combiVals = p->combiVals;
        }
      }

      std::vector<double> GetFiberVals()
      {
        if(vals.size()==2 && vals[1].data_block())
        {
          int s = vals[1].size();
          std::vector<double> retval(s);
          for(int i=0; i<s; i++)
          {
            retval[i] = vals[1](i);
          }
          return retval;
        }
        else
        {
          MITK_ERROR << "GetFiberVals() struct not initialized!!";
          return std::vector<double>(0);
        }
      }

      std::vector<double> GetNonFiberVals()
      {
        if(vals.size()==2 && vals[0].data_block())
        {
          int s = vals[0].size();
          std::vector<double> retval(s);
          for(int i=0; i<s; i++)
          {
            retval[i] = vals[0](i);
          }
          return retval;
        }
        else
        {
          MITK_ERROR << "GetNonFiberVals() struct not initialized!!";
          return std::vector<double>(0);
        }
      }

      std::vector<double> GetMixedVals()
      {
        if(mixedVals.size()==1 && mixedVals[0].data_block())
        {
          int s = mixedVals[0].size();
          std::vector<double> retval(s);
          for(int i=0; i<s; i++)
          {
            retval[i] = mixedVals[0](i);
          }
          return retval;
        }
        else
        {
          MITK_ERROR << "GetMixedVals() struct not initialized!!";
          return std::vector<double>(0);
        }
      }

      std::vector<double> GetCombiVals()
      {
        if(combiVals.data_block())
        {
          int s = combiVals.size();
          std::vector<double> retval(s);
          for(int i=0; i<s; i++)
          {
            retval[i] = combiVals(i);
          }
          return retval;
        }
        else
        {
          MITK_ERROR << "GetCombiVals() struct not initialized!!";
          return std::vector<double>(0);
        }
      }


//      void Print(VecType vec, int nr=10)
//      {
//        int sz = vec.size();
//        int incr = (int)((1.0*sz)/(1.0*nr));
//        for(int i=0; i<sz; i = i+incr)
//        {
//          std::cout << vec(i) << " ";
//        }
//        std::cout << std::endl;
//      }

//      void Print(int nr=10)
//      {
//        MITK_DEBUG << "CURVES" << std::endl;
//        MITK_DEBUG << "Fiber Vals: ";
//        Print(vals[0],nr);
//        MITK_DEBUG << "Non-Fiber Vals: ";
//        Print(vals[1],nr);
//        MITK_DEBUG << "Mixed Vals: ";
//        Print(mixedVals[0],nr);
//        MITK_DEBUG << "Combined Vals: ";
//        Print(combiVals,nr);
//      }

      std::vector<VecType> vals;
      std::vector<VecType> mixedVals;
      VecType combiVals;

    };

    class ParamsType
    {
    public:
      ParamsType()
      {
      }

      ~ParamsType()
      {
      }

      void Initialize(const ParamsType *p)
                           {
        if (this != p) // protect against invalid self-assignment
        {
          means[0] = p->means[0];
          means[1] = p->means[1];
          sigmas[0] = p->sigmas[0];
          sigmas[1] = p->sigmas[1];
          ps[0] = p->ps[0];
          ps[1] = p->ps[1];
        }
      }

//      void Print()
//      {
//        MITK_DEBUG << "PARAMS" << std::endl;
//        MITK_DEBUG << "Class 1:     " << means[0] << " +- " << sqrt(sigmas[0]) << " (p=" << ps[0] << ")" << std::endl;
//        MITK_DEBUG << "Class 2:     " << means[1] << " +- " << sqrt(sigmas[1]) << " (p=" << ps[1] << ")" << std::endl;
//        MITK_DEBUG << "Partial V: p=" << 1.0-ps[0]-ps[1] << std::endl;
//      }

      double means[2];
      double sigmas[2];
      double ps[2];
    };

    struct HelperStructClusteringResults
    {
      MitkHistType *interestingHist;
      MitkHistType *totalHist;
      double p_interesting;
    };

    struct HelperStructRGBChannels
    {
      mitk::Image::Pointer r;
      mitk::Image::Pointer g;
      mitk::Image::Pointer b;

      ~HelperStructRGBChannels()
      {
        r = 0;
        g = 0;
        b = 0;
      }
    };

    struct HelperStructPerformRGBClusteringRetval
    {
      HelperStructRGBChannels *rgbChannels;
      mitk::Image::Pointer rgb;

      ParamsType *params;
      ClusterResultType *result;
      HistType *hist;

      HelperStructPerformRGBClusteringRetval() :
          rgbChannels(0), params(0), result(0), hist(0)
      {
      }

      ~HelperStructPerformRGBClusteringRetval()
      {
        rgb = 0;
        delete rgbChannels;
      }
    };

    struct HelperStructPerformClusteringRetval
    {
      mitk::Image::Pointer clusteredImage;
      mitk::Image::Pointer displayImage;

      ParamsType *params;
      ClusterResultType *result;
      HistType *hist;

      HelperStructPerformClusteringRetval() :
          clusteredImage(0), displayImage(0),
          params(0), result(0), hist(0)
      {
      }

      ~HelperStructPerformClusteringRetval()
      {
        clusteredImage = 0;
        displayImage = 0;
      }
    };

    mitkClassMacro( PartialVolumeAnalysisClusteringCalculator, itk::Object )
        itkNewMacro( PartialVolumeAnalysisClusteringCalculator )

        ParamsType *InitialGuess(HistType h) const;

    ParamsType *Cluster(const HistType h, ParamsType* initialGuess) const;

    ClusterResultType CalculateCurves(ParamsType params, VecType xVals) const;

    void Normalize(ParamsType params, ClusterResultType* curves) const;

    HelperStructPerformClusteringRetval* PerformClustering(mitk::Image::ConstPointer image, const MitkHistType *histogram, int classIdent, HelperStructPerformClusteringRetval* precResult = 0) const;

    HelperStructPerformRGBClusteringRetval* PerformRGBClustering(mitk::Image::ConstPointer image, const MitkHistType *histogram) const;

    HelperStructPerformClusteringRetval* PerformQuantiles(mitk::Image::ConstPointer image, const MitkHistType *histogram, double p1, double p2) const;

    HelperStructPerformRGBClusteringRetval* PerformRGBQuantiles(mitk::Image::ConstPointer image, const MitkHistType *histogram, double p1, double p2 ) const;

    double* PerformQuantification(mitk::Image::ConstPointer image, mitk::Image::Pointer clusteredImage, mitk::Image::Pointer mask = 0) const;

    mitk::Image::Pointer CaculateAngularErrorImage(
        mitk::Image::Pointer comp1, mitk::Image::Pointer comp2, mitk::Image::Pointer probImg) const;

    template < unsigned int VImageDimension >
    void InternalGenerateRGB( HelperStructRGBChannels *rgb, mitk::Image::Pointer retval ) const;

    template < typename TPixel, unsigned int VImageDimension >
        void InternalGenerateProbabilityImage(
            const itk::Image< TPixel, VImageDimension > *image,
            const HelperStructClusteringResults clusterResults,
            mitk::Image::Pointer outImage1, mitk::Image::Pointer outImage2 ) const;

    template < typename TPixel, unsigned int VImageDimension >
        void InternalQuantify(
            const itk::Image< TPixel, VImageDimension > *image,
            mitk::Image::Pointer clusteredImage, double* retval, mitk::Image::Pointer mask ) const;

    template < typename TPixel, unsigned int VImageDimension >
        void InternalGenerateQuantileImage(
            const itk::Image< TPixel, VImageDimension > *image,
            double* q,
            mitk::Image::Pointer outImage1, mitk::Image::Pointer outImage2 ) const;

    ParamsType* Cluster(const HistType h) const
    {return Cluster(h, InitialGuess(h));}

    void SetMaxIt(unsigned int it)
    { m_MaxIt = it; }

    unsigned int GetMaxIt()
    { return m_MaxIt; }

    void SetStepsNumIntegration(unsigned int n)
    { m_StepsNumIntegration = n; }

    unsigned int GetStepsNumIntegration()
    { return m_StepsNumIntegration; }

  protected:

    PartialVolumeAnalysisClusteringCalculator();

    virtual ~PartialVolumeAnalysisClusteringCalculator();

    unsigned int m_MaxIt;
    unsigned int m_StepsNumIntegration;

  };

}

#endif // #define _MITK_PartialVolumeAnalysisClusteringCalculator_H
