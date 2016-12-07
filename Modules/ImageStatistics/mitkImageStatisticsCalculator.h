#ifndef MITKIMAGESTATISTICSCALCULATOR
#define MITKIMAGESTATISTICSCALCULATOR

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <mitkMaskGenerator.h>
#include <itkImage.h>
#include <limits>
#include <itkObject.h>
#include <itkSmartPointer.h>

namespace mitk
{
    class MITKIMAGESTATISTICS_EXPORT ImageStatisticsCalculator: public itk::Object
    {
    public:
        /** Standard Self typedef */
        typedef ImageStatisticsCalculator        Self;
        typedef itk::Object                         Superclass;
        typedef itk::SmartPointer< Self >           Pointer;
        typedef itk::SmartPointer< const Self >     ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self)

        /** Runtime information support. */
        itkTypeMacro(ImageStatisticsCalculator_v2, itk::Object)

        typedef double statisticsValueType;
        typedef std::map<std::string, statisticsValueType> statisticsMapType;
        typedef itk::Statistics::Histogram<double> HistogramType;
        typedef unsigned short MaskPixelType;

        /**Documentation
         @brief Container class for storing the computed image statistics.

         Container class for storing the computed image statistics. Stored statistics are:
         - N: number of voxels
         - Mean
         - MPP (Mean of positive pixels)
         - Median
         - Skewness
         - Kurtosis
         - Uniformity
         - UPP (Uniformity of positive pixels)
         - Variance
         - Std (Standard Deviation)
         - Min
         - Max
         - RMS (Root Mean Square)
         - Label (if applicable, the label (unsigned short) of the mask the statistics belong to)
         - Entropy

         It furthermore stores the following:
         - MinIndex (Index of Image where the Minimum is located)
         - MaxIndex (Index of Image where the Maximum is located)
         - Histogram of Pixel Values*/
        class MITKIMAGESTATISTICS_EXPORT StatisticsContainer : public itk::Object
        {
        public:
            /** Standard Self typedef */
            typedef StatisticsContainer                 Self;
            typedef itk::Object                         Superclass;
            typedef itk::SmartPointer< Self >           Pointer;
            typedef itk::SmartPointer< const Self >     ConstPointer;

            /** Method for creation through the object factory. */
            itkNewMacro(Self)

            /** Runtime information support. */
            itkTypeMacro(StatisticsContainer, itk::Object)

            typedef double RealType;

            /**Documentation
            @brief Returns a std::map<std::string, StatisticsValueType (aka double)> containing all real valued statistics stored in this class (= all statistics except minIndex, maxIndex and the histogram)*/
            statisticsMapType GetStatisticsAsMap();

            /**Documentation
            @brief Deletes all stored values*/
            void Reset();

            void SetN(long n)
            {
                m_N = n;
            }

            const long& GetN() const
            {
                return m_N;
            }

            void SetMean(RealType mean)
            {
                m_Mean = mean;
            }

            const RealType& GetMean() const
            {
                return m_Mean;
            }

            void SetVariance(RealType variance)
            {
                m_Variance = variance;
            }

            const RealType& GetVariance() const
            {
                return m_Variance;
            }

            void SetStd(RealType std)
            {
                m_Std = std;
            }

            const RealType& GetStd() const
            {
                return m_Std;
            }

            void SetMin(RealType minVal)
            {
                m_Min = minVal;
            }

            const RealType& GetMin() const
            {
                return m_Min;
            }

            void SetMax(RealType maxVal)
            {
                m_Max = maxVal;
            }

            const RealType& GetMax() const
            {
                return m_Max;
            }

            void SetRMS(RealType rms)
            {
                m_RMS = rms;
            }

            const RealType& GetRMS() const
            {
                return m_RMS;
            }

            void SetSkewness(RealType skewness)
            {
                m_Skewness = skewness;
            }

            const RealType& GetSkewness() const
            {
                return m_Skewness;
            }

            void SetKurtosis(RealType kurtosis)
            {
                m_Kurtosis = kurtosis;
            }

            const RealType& GetKurtosis() const
            {
                return m_Kurtosis;
            }

            void SetMPP(RealType mpp)
            {
                m_MPP = mpp;
            }

            const RealType& GetMPP() const
            {
                return m_MPP;
            }

            void SetLabel(unsigned int label)
            {
                m_Label = label;
            }

            const unsigned int& GetLabel() const
            {
                return m_Label;
            }

            void SetMinIndex(vnl_vector<int> minIndex)
            {
                m_minIndex = minIndex;
            }

            vnl_vector<int> GetMinIndex() const
            {
                return m_minIndex;
            }

            void SetMaxIndex(vnl_vector<int> maxIndex)
            {
                m_maxIndex = maxIndex;
            }

            vnl_vector<int> GetMaxIndex() const
            {
                return m_maxIndex;
            }

            void SetHistogram(HistogramType::Pointer hist)
            {
                if (m_Histogram != hist)
                {
                    m_Histogram = hist;
                }
            }

            const HistogramType::Pointer GetHistogram() const
            {
                return m_Histogram;
            }

            void SetEntropy(RealType entropy)
            {
                m_Entropy = entropy;
            }

            const RealType & GetEntropy() const
            {
                return m_Entropy;
            }

            void SetMedian(RealType median)
            {
                m_Median = median;
            }

            const RealType & GetMedian() const
            {
                return m_Median;
            }

            void SetUniformity(RealType uniformity)
            {
                m_Uniformity = uniformity;
            }

            const RealType & GetUniformity() const
            {
                return m_Uniformity;
            }

            void SetUPP(RealType upp)
            {
                m_UPP = upp;
            }

            const RealType & GetUPP() const
            {
                return m_UPP;
            }

            /**Documentation
            @brief Creates a StatisticsMapType containing all real valued statistics stored in this class (= all statistics except minIndex, maxIndex and the histogram) and prints its contents to std::cout*/
            void Print();

            /**Documentation
            @brief Generates a string that contains all real valued statistics stored in this class (= all statistics except minIndex, maxIndex and the histogram)*/
            std::string GetAsString();


        protected:
            StatisticsContainer();

        private:
            itk::LightObject::Pointer InternalClone() const
            {
                itk::LightObject::Pointer ioPtr = Superclass::InternalClone();
                Self::Pointer rval = dynamic_cast<Self*>(ioPtr.GetPointer());
                if (rval.IsNull())
                {
                    itkExceptionMacro(<< "downcast to type "
                                      << "StatisticsContainer"
                                      << " failed.");
                }

                rval->SetEntropy(this->GetEntropy());
                rval->SetKurtosis(this->GetKurtosis());
                rval->SetLabel(this->GetLabel());
                rval->SetMax(this->GetMax());
                rval->SetMin(this->GetMin());
                rval->SetMean(this->GetMean());
                rval->SetMedian(this->GetMedian());
                rval->SetMPP(this->GetMPP());
                rval->SetN(this->GetN());
                rval->SetRMS(this->GetRMS());
                rval->SetSkewness(this->GetSkewness());
                rval->SetStd(this->GetStd());
                rval->SetUniformity(this->GetUniformity());
                rval->SetUPP(this->GetUPP());
                rval->SetVariance(this->GetVariance());
                rval->SetHistogram(this->GetHistogram());
                rval->SetMinIndex(this->GetMinIndex());
                rval->SetMaxIndex(this->GetMaxIndex());
                return ioPtr;
            }

            // not pretty, is temporary
            long m_N;
            RealType m_Mean, m_Min, m_Max, m_Std, m_Variance;
            RealType m_Skewness;
            RealType m_Kurtosis;
            RealType m_RMS;
            RealType m_MPP;
            vnl_vector<int> m_minIndex, m_maxIndex;
            RealType m_Median;
            RealType m_Uniformity;
            RealType m_UPP;
            RealType m_Entropy;
            unsigned int m_Label;
            HistogramType::Pointer m_Histogram;

        };

        /**Documentation
        @brief Set the image for which the statistics are to be computed.*/
        void SetInputImage(mitk::Image::Pointer image);

        /**Documentation
        @brief Set the mask generator that creates the mask which is to be used to calculate statistics. If no more mask is desired simply set @param mask to nullptr*/
        void SetMask(mitk::MaskGenerator::Pointer mask);

        /**Documentation
        @brief Set this if more than one mask should be applied (for instance if a IgnorePixelValueMask were to be used alongside with a segmentation).
        Both masks are combined using pixel wise AND operation. The secondary mask does not have to be the same size than the primary but they need to have some overlap*/
        void SetSecondaryMask(mitk::MaskGenerator::Pointer mask);

        /**Documentation
        @brief Set number of bins to be used for histogram statistics. If Bin size is set after number of bins, bin size will be used instead!*/
        void SetNBinsForHistogramStatistics(unsigned int nBins);

        /**Documentation
        @brief Retrieve the number of bins used for histogram statistics. Careful: The return value does not indicate whether NBins or BinSize is used.
        That solely depends on which parameter has been set last.*/
        unsigned int GetNBinsForHistogramStatistics() const;

        /**Documentation
        @brief Set bin size to be used for histogram statistics. If nbins is set after bin size, nbins will be used instead!*/
        void SetBinSizeForHistogramStatistics(double binSize);

        /**Documentation
        @brief Retrieve the bin size for histogram statistics. Careful: The return value does not indicate whether NBins or BinSize is used.
        That solely depends on which parameter has been set last.*/
        double GetBinSizeForHistogramStatistics() const;

        /**Documentation
        @brief Returns the statistics for label @a label and timeStep @a timeStep. If these requested statistics are not computed yet the computation is done as well.
        For performance reasons, statistics for all labels in the image are computed at once.
         */
        StatisticsContainer::Pointer GetStatistics(unsigned int timeStep=0, unsigned int label=1);

    protected:
        ImageStatisticsCalculator(){
            m_nBinsForHistogramStatistics = 100;
            m_binSizeForHistogramStatistics = 10;
            m_UseBinSizeOverNBins = false;
        };


    private:
        template < typename TPixel, unsigned int VImageDimension > void InternalCalculateStatisticsUnmasked(
                typename itk::Image< TPixel, VImageDimension >* image,
                unsigned int timeStep);

        template < typename TPixel, unsigned int VImageDimension > typename HistogramType::Pointer InternalCalculateHistogramUnmasked(
                typename itk::Image< TPixel, VImageDimension >* image,
                double minVal,
                double maxVal);

        template < typename TPixel, unsigned int VImageDimension > void InternalCalculateStatisticsMasked(
                typename itk::Image< TPixel, VImageDimension >* image,
                unsigned int timeStep);

        bool IsUpdateRequired(unsigned int timeStep) const;

        std::string GetNameOfClass()
        {
            return std::string("ImageStatisticsCalculator_v2");
        }

        mitk::Image::Pointer m_Image;
        mitk::Image::Pointer m_ImageTimeSlice;
        mitk::Image::Pointer m_InternalImageForStatistics;

        mitk::MaskGenerator::Pointer m_MaskGenerator;
        mitk::Image::Pointer m_InternalMask;

        mitk::MaskGenerator::Pointer m_SecondaryMaskGenerator;
        mitk::Image::Pointer m_SecondaryMask;

        unsigned int m_nBinsForHistogramStatistics;
        double m_binSizeForHistogramStatistics;
        bool m_UseBinSizeOverNBins;

        std::vector<std::vector<StatisticsContainer::Pointer>> m_StatisticsByTimeStep;
        std::vector<unsigned long> m_StatisticsUpdateTimePerTimeStep;
    };

}
#endif // MITKIMAGESTATISTICSCALCULATOR

