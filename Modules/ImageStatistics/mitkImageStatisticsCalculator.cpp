
#include <limits>
#include <math.h>

#include <itkImageToHistogramFilter.h>
#include <itkMaskedImageToHistogramFilter.h>
#include <itkHistogram.h>
#include <itkExtractImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkChangeInformationImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkMinimumMaximumImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkExceptionObject.h>

#include <mitkImageStatisticsCalculator.h>
#include <mitkImage.h>
#include <mitkHistogramStatisticsCalculator.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageToItk.h>
#include <mitkExtendedStatisticsImageFilter.h>
#include <mitkExtendedLabelStatisticsImageFilter.h>
#include <mitkImageTimeSelector.h>
#include <mitkMinMaxImageFilterWithIndex.h>
#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <mitkitkMaskImageFilter.h>
#include <mitkImageCast.h>



#include <mitkMaskUtilities.h>

#include "itkImageFileWriter.h"

namespace mitk
{

    void ImageStatisticsCalculator::SetInputImage(mitk::Image::Pointer image)
    {
        if (image != m_Image)
        {
            m_Image = image;
            m_StatisticsByTimeStep.resize(m_Image->GetTimeSteps());
            m_StatisticsUpdateTimePerTimeStep.resize(m_Image->GetTimeSteps());
            std::fill(m_StatisticsUpdateTimePerTimeStep.begin(), m_StatisticsUpdateTimePerTimeStep.end(), 0);
            this->Modified();
        }
    }

    void ImageStatisticsCalculator::SetMask(mitk::MaskGenerator::Pointer mask)
    {

        if (mask != m_MaskGenerator)
        {
            m_MaskGenerator = mask;
            this->Modified();
        }

    }

    void ImageStatisticsCalculator::SetSecondaryMask(mitk::MaskGenerator::Pointer mask)
    {

        if (mask != m_SecondaryMaskGenerator)
        {
            m_SecondaryMaskGenerator = mask;
            this->Modified();
        }

    }


    void ImageStatisticsCalculator::SetNBinsForHistogramStatistics(unsigned int nBins)
    {
        if (nBins != m_nBinsForHistogramStatistics)
        {
            m_nBinsForHistogramStatistics = nBins;
            this->Modified();
            this->m_UseBinSizeOverNBins = false;
        }
        if (m_UseBinSizeOverNBins)
        {
            this->Modified();
            this->m_UseBinSizeOverNBins = false;
        }
    }

    unsigned int ImageStatisticsCalculator::GetNBinsForHistogramStatistics() const
    {
        return m_nBinsForHistogramStatistics;
    }

    void ImageStatisticsCalculator::SetBinSizeForHistogramStatistics(double binSize)
    {
        if (binSize != m_binSizeForHistogramStatistics)
        {
            m_binSizeForHistogramStatistics = binSize;
            this->Modified();
            this->m_UseBinSizeOverNBins = true;
        }
        if (!m_UseBinSizeOverNBins)
        {
            this->Modified();
            this->m_UseBinSizeOverNBins = true;
        }
    }

    double ImageStatisticsCalculator::GetBinSizeForHistogramStatistics() const
    {
        return m_binSizeForHistogramStatistics;
    }

    ImageStatisticsCalculator::StatisticsContainer::Pointer ImageStatisticsCalculator::GetStatistics(unsigned int timeStep, unsigned int label)
    {

        if (timeStep >= m_StatisticsByTimeStep.size())
        {
             mitkThrow() << "invalid timeStep in ImageStatisticsCalculator_v2::GetStatistics";
        }

        if (m_Image.IsNull())
        {
             mitkThrow() << "no image";
        }

        if (!m_Image->IsInitialized())
        {
          mitkThrow() << "Image not initialized!";
        }

        if (IsUpdateRequired(timeStep))
        {
            if (m_MaskGenerator.IsNotNull())
            {
                m_MaskGenerator->SetTimeStep(timeStep);
                m_InternalMask = m_MaskGenerator->GetMask();
                if (m_MaskGenerator->GetReferenceImage().IsNotNull())
                {
                    m_InternalImageForStatistics = m_MaskGenerator->GetReferenceImage();
                }
                else
                {
                    m_InternalImageForStatistics = m_Image;
                }
            }
            else
            {
                m_InternalImageForStatistics = m_Image;
            }

            if (m_SecondaryMaskGenerator.IsNotNull())
            {
                m_SecondaryMaskGenerator->SetTimeStep(timeStep);
                m_SecondaryMask = m_SecondaryMaskGenerator->GetMask();
            }

            ImageTimeSelector::Pointer imgTimeSel = ImageTimeSelector::New();
            imgTimeSel->SetInput(m_InternalImageForStatistics);
            imgTimeSel->SetTimeNr(timeStep);
            imgTimeSel->UpdateLargestPossibleRegion();
            m_ImageTimeSlice = imgTimeSel->GetOutput();


            // Calculate statistics with/without mask
            if (m_MaskGenerator.IsNull() && m_SecondaryMaskGenerator.IsNull())
            {
                // 1) calculate statistics unmasked:
                AccessByItk_1(m_ImageTimeSlice, InternalCalculateStatisticsUnmasked, timeStep)

            }
            else
            {
                // 2) calculate statistics masked
                AccessByItk_1(m_ImageTimeSlice, InternalCalculateStatisticsMasked, timeStep)
            }


            //this->Modified();
        }

        m_StatisticsUpdateTimePerTimeStep[timeStep] = m_StatisticsByTimeStep[timeStep][m_StatisticsByTimeStep[timeStep].size()-1]->GetMTime();

        for (std::vector<StatisticsContainer::Pointer>::iterator it = m_StatisticsByTimeStep[timeStep].begin(); it != m_StatisticsByTimeStep[timeStep].end(); ++it)
        {
            StatisticsContainer::Pointer statCont = *it;
            if (statCont->GetLabel() == label)
            {
                return statCont->Clone();
            }
        }

        // these lines will ony be executed if the requested label could not be found!
        MITK_WARN << "Invalid label: " << label << " in time step: " << timeStep;
        return StatisticsContainer::New();
    }

    template < typename TPixel, unsigned int VImageDimension > void ImageStatisticsCalculator::InternalCalculateStatisticsUnmasked(
            typename itk::Image< TPixel, VImageDimension >* image, unsigned int timeStep)
    {
        typedef typename itk::Image< TPixel, VImageDimension > ImageType;
        typedef typename itk::ExtendedStatisticsImageFilter<ImageType> ImageStatisticsFilterType;
        typedef typename itk::MinMaxImageFilterWithIndex<ImageType> MinMaxFilterType;

        StatisticsContainer::Pointer statisticsResult = StatisticsContainer::New();

        typename ImageStatisticsFilterType::Pointer statisticsFilter = ImageStatisticsFilterType::New();
        statisticsFilter->SetInput(image);
        statisticsFilter->SetCoordinateTolerance(0.001);
        statisticsFilter->SetDirectionTolerance(0.001);

        // TODO: this is single threaded. Implement our own image filter that does this multi threaded
//        typename itk::MinimumMaximumImageCalculator<ImageType>::Pointer imgMinMaxFilter = itk::MinimumMaximumImageCalculator<ImageType>::New();
//        imgMinMaxFilter->SetImage(image);
//        imgMinMaxFilter->Compute();
        vnl_vector<int> minIndex, maxIndex;

        typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
        minMaxFilter->SetInput(image);
        minMaxFilter->UpdateLargestPossibleRegion();
        typename ImageType::PixelType minval = minMaxFilter->GetMin();
        typename ImageType::PixelType maxval = minMaxFilter->GetMax();

        typename ImageType::IndexType tmpMinIndex = minMaxFilter->GetMinIndex();
        typename ImageType::IndexType tmpMaxIndex = minMaxFilter->GetMaxIndex();

//        typename ImageType::IndexType tmpMinIndex = imgMinMaxFilter->GetIndexOfMinimum();
//        typename ImageType::IndexType tmpMaxIndex = imgMinMaxFilter->GetIndexOfMaximum();

        minIndex.set_size(tmpMaxIndex.GetIndexDimension());
        maxIndex.set_size(tmpMaxIndex.GetIndexDimension());

        for (unsigned int i=0; i < tmpMaxIndex.GetIndexDimension(); i++)
        {
            minIndex[i] = tmpMinIndex[i];
            maxIndex[i] = tmpMaxIndex[i];
        }

        statisticsResult->SetMinIndex(minIndex);
        statisticsResult->SetMaxIndex(maxIndex);

        //convert m_binSize in m_nBins if necessary
        unsigned int nBinsForHistogram;
        if (m_UseBinSizeOverNBins)
        {
            nBinsForHistogram = std::max(static_cast<double>(std::ceil(maxval - minval)) / m_binSizeForHistogramStatistics, 10.); // do not allow less than 10 bins
        }
        else
        {
            nBinsForHistogram = m_nBinsForHistogramStatistics;
        }

        statisticsFilter->SetHistogramParameters(nBinsForHistogram, minval, maxval);

        try
        {
          statisticsFilter->Update();
        }
        catch (const itk::ExceptionObject& e)
        {
          mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
        }

        // no mask, therefore just one label = the whole image
        m_StatisticsByTimeStep[timeStep].resize(1);
        statisticsResult->SetLabel(1);
        statisticsResult->SetN(image->GetLargestPossibleRegion().GetNumberOfPixels());
        statisticsResult->SetMean(statisticsFilter->GetMean());
        statisticsResult->SetMin(statisticsFilter->GetMinimum());
        statisticsResult->SetMax(statisticsFilter->GetMaximum());
        statisticsResult->SetVariance(statisticsFilter->GetVariance());
        statisticsResult->SetStd(statisticsFilter->GetSigma());
        statisticsResult->SetSkewness(statisticsFilter->GetSkewness());
        statisticsResult->SetKurtosis(statisticsFilter->GetKurtosis());
        statisticsResult->SetRMS(std::sqrt(std::pow(statisticsFilter->GetMean(), 2.) + statisticsFilter->GetVariance())); // variance = sigma^2
        statisticsResult->SetMPP(statisticsFilter->GetMPP());

        statisticsResult->SetEntropy(statisticsFilter->GetEntropy());
        statisticsResult->SetMedian(statisticsFilter->GetMedian());
        statisticsResult->SetUniformity(statisticsFilter->GetUniformity());
        statisticsResult->SetUPP(statisticsFilter->GetUPP());
        statisticsResult->SetHistogram(statisticsFilter->GetHistogram());

        m_StatisticsByTimeStep[timeStep][0] = statisticsResult;
    }


    template < typename TPixel, unsigned int VImageDimension > void ImageStatisticsCalculator::InternalCalculateStatisticsMasked(
            typename itk::Image< TPixel, VImageDimension >* image,
            unsigned int timeStep)
    {
        typedef itk::Image< TPixel, VImageDimension > ImageType;
        typedef itk::Image< MaskPixelType, VImageDimension > MaskType;
        typedef typename MaskType::PixelType LabelPixelType;
        typedef itk::ExtendedLabelStatisticsImageFilter< ImageType, MaskType > ImageStatisticsFilterType;
        typedef MaskUtilities< TPixel, VImageDimension > MaskUtilType;
        typedef typename itk::MinMaxLabelImageFilterWithIndex<ImageType, MaskType> MinMaxLabelFilterType;
        typedef typename ImageType::PixelType InputImgPixelType;

        // workaround: if m_SecondaryMaskGenerator ist not null but m_MaskGenerator is! (this is the case if we request a 'ignore zuero valued pixels'
        // mask in the gui but do not define a primary mask)
        bool swapMasks = false;
        if (m_SecondaryMask.IsNotNull() && m_InternalMask.IsNull())
        {
            m_InternalMask = m_SecondaryMask;
            m_SecondaryMask = nullptr;
            swapMasks = true;
        }

        // maskImage has to have the same dimension as image
        typename MaskType::Pointer maskImage = MaskType::New();
        try {
            // try to access the pixel values directly (no copying or casting). Only works if mask pixels are of pixelType unsigned short
            maskImage = ImageToItkImage< MaskPixelType, VImageDimension >(m_InternalMask);
        }
        catch (itk::ExceptionObject & e)

        {
            // if the pixel type of the mask is not short, then we have to make a copy of m_InternalMask (and cast the values)
            CastToItkImage(m_InternalMask, maskImage);
        }

        // if we have a secondary mask (say a ignoreZeroPixelMask) we need to combine the masks (corresponds to AND)
        if (m_SecondaryMask.IsNotNull())
        {
            // dirty workaround for a bug when pf mask + any other mask is used in conjunction. We need a proper fix for this (Fabian Isensee is responsible and probably working on it!)
            if (m_InternalMask->GetDimension() == 2 && (m_SecondaryMask->GetDimension() == 3 || m_SecondaryMask->GetDimension() == 4))
            {
                mitk::Image::Pointer old_img = m_SecondaryMaskGenerator->GetReferenceImage();
                m_SecondaryMaskGenerator->SetInputImage(m_MaskGenerator->GetReferenceImage());
                m_SecondaryMask = m_SecondaryMaskGenerator->GetMask();
                m_SecondaryMaskGenerator->SetInputImage(old_img);
            }
            typename MaskType::Pointer secondaryMaskImage = MaskType::New();
            secondaryMaskImage = ImageToItkImage< MaskPixelType, VImageDimension >(m_SecondaryMask);

            // secondary mask should be a ignore zero value pixel mask derived from image. it has to be cropped to the mask region (which may be planar or simply smaller)
            typename MaskUtilities<MaskPixelType, VImageDimension>::Pointer secondaryMaskMaskUtil = MaskUtilities<MaskPixelType, VImageDimension>::New();
            secondaryMaskMaskUtil->SetImage(secondaryMaskImage.GetPointer());
            secondaryMaskMaskUtil->SetMask(maskImage.GetPointer());
            typename MaskType::Pointer adaptedSecondaryMaskImage = secondaryMaskMaskUtil->ExtractMaskImageRegion();

            typename itk::MaskImageFilter2<MaskType, MaskType, MaskType>::Pointer maskFilter = itk::MaskImageFilter2<MaskType, MaskType, MaskType>::New();
            maskFilter->SetInput1(maskImage);
            maskFilter->SetInput2(adaptedSecondaryMaskImage);
            maskFilter->SetMaskingValue(1); // all pixels of maskImage where secondaryMaskImage==1 will be kept, all the others are set to 0
            maskFilter->UpdateLargestPossibleRegion();
            maskImage = maskFilter->GetOutput();
        }

        typename MaskUtilType::Pointer maskUtil = MaskUtilType::New();
        maskUtil->SetImage(image);
        maskUtil->SetMask(maskImage.GetPointer());

        // if mask is smaller than image, extract the image region where the mask is
        typename ImageType::Pointer adaptedImage = ImageType::New();

        adaptedImage = maskUtil->ExtractMaskImageRegion(); // this also checks mask sanity

        // find min, max, minindex and maxindex
        typename MinMaxLabelFilterType::Pointer minMaxFilter = MinMaxLabelFilterType::New();
        minMaxFilter->SetInput(adaptedImage);
        minMaxFilter->SetLabelInput(maskImage);
        minMaxFilter->UpdateLargestPossibleRegion();

        // set histogram parameters for each label individually (min/max may be different for each label)
        typedef typename std::map<LabelPixelType, InputImgPixelType> MapType;
        typedef typename std::pair<LabelPixelType, InputImgPixelType> PairType;

        std::vector<LabelPixelType> relevantLabels = minMaxFilter->GetRelevantLabels();
        MapType minVals;
        MapType maxVals;
        std::map<LabelPixelType, unsigned int> nBins;

        for (LabelPixelType label:relevantLabels)
        {
            minVals.insert(PairType(label, minMaxFilter->GetMin(label)));
            maxVals.insert(PairType(label, minMaxFilter->GetMax(label)));

            unsigned int nBinsForHistogram;
            if (m_UseBinSizeOverNBins)
            {
                nBinsForHistogram = std::max(static_cast<double>(std::ceil(minMaxFilter->GetMax(label) - minMaxFilter->GetMin(label))) / m_binSizeForHistogramStatistics, 10.); // do not allow less than 10 bins
            }
            else
            {
                nBinsForHistogram = m_nBinsForHistogramStatistics;
            }

            nBins.insert(typename std::pair<LabelPixelType, unsigned int>(label, nBinsForHistogram));
        }

        typename ImageStatisticsFilterType::Pointer imageStatisticsFilter = ImageStatisticsFilterType::New();
        imageStatisticsFilter->SetDirectionTolerance(0.001);
        imageStatisticsFilter->SetCoordinateTolerance(0.001);
        imageStatisticsFilter->SetInput(adaptedImage);
        imageStatisticsFilter->SetLabelInput(maskImage);
        imageStatisticsFilter->SetHistogramParametersForLabels(nBins, minVals, maxVals);
        imageStatisticsFilter->Update();

        std::list<int> labels = imageStatisticsFilter->GetRelevantLabels();
        std::list<int>::iterator it = labels.begin();
        m_StatisticsByTimeStep[timeStep].resize(0);

        while(it != labels.end())
        {
            StatisticsContainer::Pointer statisticsResult = StatisticsContainer::New();

            // find min, max, minindex and maxindex
            // make sure to only look in the masked region, use a masker for this

            vnl_vector<int> minIndex, maxIndex;
            mitk::Point3D worldCoordinateMin;
            mitk::Point3D worldCoordinateMax;
            mitk::Point3D indexCoordinateMin;
            mitk::Point3D indexCoordinateMax;
            m_InternalImageForStatistics->GetGeometry()->IndexToWorld(minMaxFilter->GetMinIndex(*it), worldCoordinateMin);
            m_InternalImageForStatistics->GetGeometry()->IndexToWorld(minMaxFilter->GetMaxIndex(*it), worldCoordinateMax);
            m_Image->GetGeometry()->WorldToIndex(worldCoordinateMin, indexCoordinateMin);
            m_Image->GetGeometry()->WorldToIndex(worldCoordinateMax, indexCoordinateMax);

            //typename ImageType::IndexType tmpMinIndex = minMaxFilter->GetMinIndex(*it);
            //typename ImageType::IndexType tmpMaxIndex = minMaxFilter->GetMaxIndex(*it);

            //minIndex.set_size(tmpMaxIndex.GetIndexDimension());
            //maxIndex.set_size(tmpMaxIndex.GetIndexDimension());
            minIndex.set_size(3);
            maxIndex.set_size(3);

            //for (unsigned int i=0; i < tmpMaxIndex.GetIndexDimension(); i++)
            for (unsigned int i=0; i < 3; i++)
            {
                //minIndex[i] = tmpMinIndex[i] + (maskImage->GetOrigin()[i] - image->GetOrigin()[i]) / (double) maskImage->GetSpacing()[i];
                //maxIndex[i] = tmpMaxIndex[i] + (maskImage->GetOrigin()[i] - image->GetOrigin()[i]) / (double) maskImage->GetSpacing()[i];
                minIndex[i] = indexCoordinateMin[i];
                maxIndex[i] = indexCoordinateMax[i];
            }

            statisticsResult->SetMinIndex(minIndex);
            statisticsResult->SetMaxIndex(maxIndex);

            // just debug
            TPixel min_Filter = minMaxFilter->GetMin(*it);
            TPixel max_Filter = minMaxFilter->GetMax(*it);
            TPixel min_Itk = imageStatisticsFilter->GetMinimum(*it);
            TPixel max_Itk = imageStatisticsFilter->GetMaximum(*it);

            assert(abs(minMaxFilter->GetMax(*it) - imageStatisticsFilter->GetMaximum(*it)) < mitk::eps);
            assert(abs(minMaxFilter->GetMin(*it) - imageStatisticsFilter->GetMinimum(*it)) < mitk::eps);


            statisticsResult->SetN(imageStatisticsFilter->GetSum(*it) / (double) imageStatisticsFilter->GetMean(*it));
            statisticsResult->SetMean(imageStatisticsFilter->GetMean(*it));
            statisticsResult->SetMin(imageStatisticsFilter->GetMinimum(*it));
            statisticsResult->SetMax(imageStatisticsFilter->GetMaximum(*it));
            statisticsResult->SetVariance(imageStatisticsFilter->GetVariance(*it));
            statisticsResult->SetStd(imageStatisticsFilter->GetSigma(*it));
            statisticsResult->SetSkewness(imageStatisticsFilter->GetSkewness(*it));
            statisticsResult->SetKurtosis(imageStatisticsFilter->GetKurtosis(*it));
            statisticsResult->SetRMS(std::sqrt(std::pow(imageStatisticsFilter->GetMean(*it), 2.) + imageStatisticsFilter->GetVariance(*it))); // variance = sigma^2
            statisticsResult->SetMPP(imageStatisticsFilter->GetMPP(*it));
            statisticsResult->SetLabel(*it);

            statisticsResult->SetEntropy(imageStatisticsFilter->GetEntropy(*it));
            statisticsResult->SetMedian(imageStatisticsFilter->GetMedian(*it));
            statisticsResult->SetUniformity(imageStatisticsFilter->GetUniformity(*it));
            statisticsResult->SetUPP(imageStatisticsFilter->GetUPP(*it));
            statisticsResult->SetHistogram(imageStatisticsFilter->GetHistogram(*it));

            m_StatisticsByTimeStep[timeStep].push_back(statisticsResult);
            ++it;
        }

        // swap maskGenerators back
        if (swapMasks)
        {
            m_SecondaryMask = m_InternalMask;
            m_InternalMask = nullptr;
        }
    }

    bool ImageStatisticsCalculator::IsUpdateRequired(unsigned int timeStep) const
    {
        unsigned long thisClassTimeStamp = this->GetMTime();
        unsigned long inputImageTimeStamp = m_Image->GetMTime();
        unsigned long statisticsTimeStamp = m_StatisticsUpdateTimePerTimeStep[timeStep];

        if (thisClassTimeStamp > statisticsTimeStamp) // inputs have changed
        {
            return true;
        }

        if (inputImageTimeStamp > statisticsTimeStamp) // image has changed
        {
            return true;
        }

        if (m_MaskGenerator.IsNotNull())
        {
            unsigned long maskGeneratorTimeStamp = m_MaskGenerator->GetMTime();
            if (maskGeneratorTimeStamp > statisticsTimeStamp) // there is a mask generator and it has changed
            {
                return true;
            }
        }

        if (m_SecondaryMaskGenerator.IsNotNull())
        {
            unsigned long maskGeneratorTimeStamp = m_SecondaryMaskGenerator->GetMTime();
            if (maskGeneratorTimeStamp > statisticsTimeStamp) // there is a secondary mask generator and it has changed
            {
                return true;
            }
        }

        return false;
    }


    ImageStatisticsCalculator::StatisticsContainer::StatisticsContainer():
        m_N(nan("")),
        m_Mean(nan("")),
        m_Min(nan("")),
        m_Max(nan("")),
        m_Std(nan("")),
        m_Variance(nan("")),
        m_Skewness(nan("")),
        m_Kurtosis(nan("")),
        m_RMS(nan("")),
        m_MPP(nan("")),
        m_Median(nan("")),
        m_Uniformity(nan("")),
        m_UPP(nan("")),
        m_Entropy(nan(""))
    {
        m_minIndex.set_size(0);
        m_maxIndex.set_size(0);
    }

    ImageStatisticsCalculator::statisticsMapType ImageStatisticsCalculator::StatisticsContainer::GetStatisticsAsMap()
    {
        ImageStatisticsCalculator::statisticsMapType statisticsAsMap;

        statisticsAsMap["N"] = m_N;
        statisticsAsMap["Mean"] = m_Mean;
        statisticsAsMap["Min"] = m_Min;
        statisticsAsMap["Max"] = m_Max;
        statisticsAsMap["StandardDeviation"] = m_Std;
        statisticsAsMap["Variance"] = m_Variance;
        statisticsAsMap["Skewness"] = m_Skewness;
        statisticsAsMap["Kurtosis"] = m_Kurtosis;
        statisticsAsMap["RMS"] = m_RMS;
        statisticsAsMap["MPP"] = m_MPP;
        statisticsAsMap["Median"] = m_Median;
        statisticsAsMap["Uniformity"] = m_Uniformity;
        statisticsAsMap["UPP"] = m_UPP;
        statisticsAsMap["Entropy"] = m_Entropy;
        statisticsAsMap["Label"] = m_Label;

        return statisticsAsMap;
    }


    void ImageStatisticsCalculator::StatisticsContainer::Reset()
    {
        m_N = nan("");
        m_Mean = nan("");
        m_Min = nan("");
        m_Max = nan("");
        m_Std = nan("");
        m_Variance = nan("");
        m_Skewness = nan("");
        m_Kurtosis = nan("");
        m_RMS = nan("");
        m_MPP = nan("");
        m_Median = nan("");
        m_Uniformity = nan("");
        m_UPP = nan("");
        m_Entropy = nan("");
        m_Histogram = HistogramType::New();
        m_minIndex.set_size(0);
        m_maxIndex.set_size(0);
        m_Label = 0;
    }

    void ImageStatisticsCalculator::StatisticsContainer::Print()
    {
        ImageStatisticsCalculator::statisticsMapType statMap = this->GetStatisticsAsMap();
        // print all map key value pairs
        // const auto& val:statMap
        for (auto it = statMap.begin(); it != statMap.end(); ++it)
        {
            std::cout << it->first << ": " << it->second << std::endl;
        }

        // print the min and max index
        std::cout << "Min Index:" << std::endl;
        for (auto it = this->GetMinIndex().begin(); it != this->GetMinIndex().end(); ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // print the min and max index
        std::cout << "Max Index:" << std::endl;
        for (auto it = this->GetMaxIndex().begin(); it != this->GetMaxIndex().end(); ++it)
        {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }

    std::string ImageStatisticsCalculator::StatisticsContainer::GetAsString()
    {
        std::string res = "";
        ImageStatisticsCalculator::statisticsMapType statMap = this->GetStatisticsAsMap();
        // print all map key value pairs
        // const auto& val:statMap
        for (auto it = statMap.begin(); it != statMap.end(); ++it)
        {
            res += std::string(it->first) + ": " + std::to_string(it->second) + "\n";
        }

        // print the min and max index
        res += "Min Index:" + std::string("\n");
        for (auto it = this->GetMinIndex().begin(); it != this->GetMinIndex().end(); it++)
        {
            res += std::to_string(*it) + std::string(" ");
        }
        res += "\n";

        // print the min and max index
        res += "Max Index:" + std::string("\n");
        for (auto it = this->GetMaxIndex().begin(); it != this->GetMaxIndex().end(); it++)
        {
            res += std::to_string(*it) + " ";
        }
        res +=  "\n";
        return res;
    }


}
