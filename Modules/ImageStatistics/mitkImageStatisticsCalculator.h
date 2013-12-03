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

#ifndef mitkImageStatisticsCalculator_h
#define mitkImageStatisticsCalculator_h

#include "mitkImage.h"
#include "mitkPlanarFigure.h"

// TODO DM: why the ifndef?
#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <itkImageRegionIteratorWithIndex.h>


#include <vtkSmartPointer.h>

#include "ImageStatisticsExports.h"

namespace mitk
{

/**
 * \brief Class for calculating statistics and histogram for an (optionally
 * masked) image.
 *
 * Images can be masked by either a label image (of the same dimensions as
 * the original image) or by a closed mitk::PlanarFigure, e.g. a circle or
 * polygon. When masking with a planar figure, the slice corresponding to the
 * plane containing the figure is extracted and then clipped with contour
 * defined by the figure. Planar figures need to be aligned along the main axes
 * of the image (axial, sagittal, coronal). Planar figures on arbitrary
 * rotated planes are not supported.
 *
 * For each operating mode (no masking, masking by image, masking by planar
 * figure), the calculated statistics and histogram are cached so that, when
 * switching back and forth between operation modes without modifying mask or
 * image, the information doesn't need to be recalculated.
 *
 * The class also has the possibility to calculate the location and separate
 * statistics for a region called “hotspot”. The hotspot is a sphere of
 * user-defined size and its location is chosen in a way that the average
 * pixel value within the sphere is maximized.
 *
 * Note: currently time-resolved and multi-channel pictures are not properly
 * supported.
 */

  /**
 * \section HotspotStatistics_caption Calculation of hotspot statistics
 *
 * Since calculation of hotspot location and statistics is not
 * straight-forward, the following paragraphs will describe it in more detail.
 *
 * <b>Note: Calculation of hotspot statistics is optional.</b>
 *
 * \subsection HotspotStatistics_description Hotspot Definition
 *
 * The “hotspot” of an image is motivated from PET readings. It is defined
 * as a spherical region of fixed size which maximizes the average pixel value
 * within the region.  The following image illustrates the concept: the
 * colored areas are different image intensities and the hotspot is located
 * in the “hottest” region of the image.
 *
 * [image with pixelvalue scale]
 *
 * \subsection HotspotStatistics_calculation Hotspot Calculation
 *
 * Since only the size of the hotspot is known initially, we need to calculate
 * two aspects (both implemented in ComputeHotspotStatistics):
 * - the hotspot location
 * - statistics of the pixels within the hotspot
 * Finding the hotspot location requires to calculate the average value at each
 * position. This is done by convolution of the image with a sperical kernel
 * image which reflects partial volumes (important in the case of low-resolution
 * PET images).
 *
 * Once the hotspot location is known, calculating the actual statistics is a
 * simple task which is implemented in ...TODO...
 *
 * \b Step 1: Finding the hotspot by image convolution
 *
 * As described above, we use image convolution with a rasterized sphere to
 * average the image at each position. To handle coarse resolutions, which would
 * normally force us to decide for partially contained voxels whether to count
 * them or not, we supersample the kernel image and use non-integer kernel values
 * (see TODO-MethodName), which reflect the volume part that is contained in the
 * sphere. For example, if three subvoxels are inside the sphere, the corresponding
 *  kernel voxel gets a value of 0.75 (3 out of 4 subvoxels, see 2D example below).
 *
 * [image with subsampled pixel]
 *
 * Convolution itself is done by means of the itk::FFTConvolutionImageFilter.
 * To find the hotspot location, we simply iterate the averaged image and find a
 * maximum location (see CalculateExtrema()).
 * TODO? Discuss cases with multiple maxima!?
 * -> "In case of images with multiple maxima the method returns value and corresponding
 * index of the extrema that is found by the iterator first"
 *
 * \b Step 2: Computation of hotspot statistics
 *
 * Once the hotspot location is found, statistics for the region are calculated
 * by simply iterating the input image and regarding all pixel centers inside the
 * hotspot-sphere for statistics.
 *
 * \subsection HotspotStatistics_tests Tests
 *
 * To check the correctness of the hotspot calculation, a special class
 * (mitkImageStatisticsHotspotTest) has been created, which generates images with
 * known hotspot location and statistics. A number of unit tests use this class
 * to first generate an image of known properites and then verify that
 * ImageStatisticsCalculator is able to reproduce the known statistics.
 *
*/
class ImageStatistics_EXPORT ImageStatisticsCalculator : public itk::Object
{
public:

  /**
    TODO DM: document
  */
  enum
  {
    MASKING_MODE_NONE = 0,
    MASKING_MODE_IMAGE = 1,
    MASKING_MODE_PLANARFIGURE = 2
  };

  typedef itk::Statistics::Histogram<double> HistogramType;
  typedef HistogramType::ConstIterator HistogramConstIteratorType;

  /**
    TODO DM: document
  */
  class ImageStatistics_EXPORT Statistics
  {
  public:

    Statistics();

    /** \brief Initialize every member of Statistics to zero. */
    //void Reset()

    unsigned int GetN() const;
    double GetMin() const;
    double GetMax() const;
    double GetMean() const;
    double GetMedian() const;
    double GetVariance() const;
    double GetSigma() const;
    double GetRMS() const;
    vnl_vector<int> GetMaxIndex() const;
    vnl_vector<int> GetMinIndex() const;

    void SetLabel(unsigned int label);
    void SetN(unsigned int n);
    void SetMin(double min);
    void SetMax(double max);
    void SetMean(double mean);
    void SetMedian(double median);
    void SetVariance(double variance);
    void SetSigma(double sigma);
    void SetRMS(double rms);
    void SetMaxIndex(vnl_vector<int> maxIndex);
    void SetMinIndex(vnl_vector<int> minIndex);

    unsigned int GetHotspotN() const;
    double GetHotspotMin() const;
    double GetHotspotMax() const;
    double GetHotspotMean() const;
    double GetHotspotMedian() const;
    double GetHotspotVariance() const;
    double GetHotspotSigma() const;
    double GetHotspotRMS() const;
    vnl_vector<int> GetHotspotMaxIndex() const;
    vnl_vector<int> GetHotspotMinIndex() const;
    vnl_vector<int> GetHotspotIndex() const;

    void SetHotspotLabel(unsigned int label);
    void SetHotspotN(unsigned int n);
    void SetHotspotMin(double min);
    void SetHotspotMax(double max);
    void SetHotspotMean(double mean);
    void SetHotspotMedian(double median);
    void SetHotspotVariance(double variance);
    void SetHotspotSigma(double sigma);
    void SetHotspotRMS(double rms);
    void SetHotspotMaxIndex(vnl_vector<int> hotspotMaxIndex);
    void SetHotspotMinIndex(vnl_vector<int> hotspotMinIndex);
    void SetHotspotIndex(vnl_vector<int> hotspotIndex);

  private:

    int m_Label;
    unsigned int m_N;                   //< number of voxels
    double m_Min;                       //< mimimum value
    double m_Max;                       //< maximum value
    double m_Mean;                      //< mean value
    double m_Median;                    //< median value
    double m_Variance;
    double m_Sigma;                     //< standard deviation of values (== square root of variance)
    double m_RMS;                       //< root mean square

    vnl_vector< int > m_MinIndex;       //< index of minimum value
    vnl_vector< int > m_MaxIndex;       //< index of maximum value

    unsigned int m_HotspotN;            //< number of voxels inside hotspot
    double m_HotspotMin;                //< mimimum value inside hotspot
    double m_HotspotMax;                //< maximum value inside hotspot
    double m_HotspotMean;               //< mean value of hotspot
    double m_HotspotMedian;             //< median value of hotspot
    double m_HotspotVariance;
    double m_HotspotSigma;              //< standard deviation of values inside hotspot
    double m_HotspotRMS;                //< root mean square of hotspot

    vnl_vector<int> m_HotspotMinIndex;  //< index of minimum value inside hotspot
    vnl_vector<int> m_HotspotMaxIndex;  //< index of maximum value inside hotspot
    vnl_vector<int> m_HotspotIndex;     //< index of hotspot origin
  };

  typedef std::vector< HistogramType::ConstPointer > HistogramContainer;
  typedef std::vector< Statistics > StatisticsContainer;

  mitkClassMacro( ImageStatisticsCalculator, itk::Object );
  itkNewMacro( ImageStatisticsCalculator );

  /** \brief Set image from which to compute statistics. */
  void SetImage( const mitk::Image *image );

  /** \brief Set image for masking. */
  void SetImageMask( const mitk::Image *imageMask );

  /** \brief Set planar figure for masking. */
  void SetPlanarFigure( mitk::PlanarFigure *planarFigure );


  /** \brief Set/Get operation mode for masking */
  void SetMaskingMode( unsigned int mode );

  /** \brief Set/Get operation mode for masking */
  itkGetMacro( MaskingMode, unsigned int );

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToNone();

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToImage();

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToPlanarFigure();

  /** \brief Set a pixel value for pixels that will be ignored in the statistics */
  void SetIgnorePixelValue(double value);

  /** \brief Get the pixel value for pixels that will be ignored in the statistics */
  double GetIgnorePixelValue();

  /** \brief Set whether a pixel value should be ignored in the statistics */
  void SetDoIgnorePixelValue(bool doit);

  /** \brief Get whether a pixel value will be ignored in the statistics */
  bool GetDoIgnorePixelValue();

  /** \brief Sets the radius for the hotspot */
  void SetHotspotRadiusInMM (double hotspotRadiusInMM);

  /** \brief Returns the radius of the hotspot */
  double GetHotspotRadiusInMM();

  /** \brief Sets whether the hotspot should be calculated */
  void SetCalculateHotspot(bool calculateHotspot);

  /** \brief Returns true whether the hotspot should be calculated, otherwise false */
  bool IsHotspotCalculated();

  /** \brief Compute statistics (together with histogram) for the current
   * masking mode.
   *
   * Computation is not executed if statistics is already up to date. In this
   * case, false is returned; otherwise, true.*/
  virtual bool ComputeStatistics( unsigned int timeStep = 0 );


  /** \brief Retrieve the histogram depending on the current masking mode.
   *
   * \param label The label for which to retrieve the histogram in multi-label situations (ascending order).
   */
  const HistogramType *GetHistogram(  unsigned int timeStep = 0, unsigned int label = 0 ) const;

  /** \brief Retrieve the histogram depending on the current masking mode (for all image labels. */
  const HistogramContainer &GetHistogramVector(  unsigned int timeStep = 0 ) const;

  /** \brief Retrieve statistics depending on the current masking mode.
   *
   * \param label The label for which to retrieve the statistics in multi-label situations (ascending order).
   */
  const Statistics &GetStatistics( unsigned int timeStep = 0, unsigned int label = 0 ) const;


  /** \brief Retrieve statistics depending on the current masking mode (for all image labels). */
  const StatisticsContainer &GetStatisticsVector( unsigned int timeStep = 0 ) const;



protected:
  typedef std::vector< HistogramContainer > HistogramVector;
  typedef std::vector< StatisticsContainer > StatisticsVector;

  typedef std::vector< itk::TimeStamp > TimeStampVectorType;
  typedef std::vector< bool > BoolVectorType;

  typedef itk::Image< unsigned short, 3 > MaskImage3DType;
  typedef itk::Image< unsigned short, 2 > MaskImage2DType;

  ImageStatisticsCalculator();

  virtual ~ImageStatisticsCalculator();

  /** \brief Depending on the masking mode, the image and mask from which to
   * calculate statistics is extracted from the original input image and mask
   * data.
   *
   * For example, a when using a PlanarFigure as mask, the 2D image slice
   * corresponding to the PlanarFigure will be extracted from the original
   * image. If masking is disabled, the original image is simply passed
   * through. */
  void ExtractImageAndMask( unsigned int timeStep = 0 );


  /** \brief If the passed vector matches any of the three principal axes
   * of the passed geometry, the ínteger value corresponding to the axis
   * is set and true is returned. */
  bool GetPrincipalAxis( const Geometry3D *geometry, Vector3D vector,
    unsigned int &axis );

  template < typename TPixel, unsigned int VImageDimension >
  void InternalCalculateStatisticsUnmasked(
    const itk::Image< TPixel, VImageDimension > *image,
    StatisticsContainer* statisticsContainer,
    HistogramContainer *histogramContainer );

  template < typename TPixel, unsigned int VImageDimension >
  void InternalCalculateStatisticsMasked(
    const itk::Image< TPixel, VImageDimension > *image,
    itk::Image< unsigned short, VImageDimension > *maskImage,
    StatisticsContainer* statisticsContainer,
    HistogramContainer* histogramContainer );

  template < typename TPixel, unsigned int VImageDimension >
  void InternalCalculateMaskFromPlanarFigure(
    const itk::Image< TPixel, VImageDimension > *image, unsigned int axis );

  template < typename TPixel, unsigned int VImageDimension >
  void InternalMaskIgnoredPixels(
    const itk::Image< TPixel, VImageDimension > *image,
    itk::Image< unsigned short, VImageDimension > *maskImage );

  struct ImageExtrema
  {
    double Max;
    double Min;
    vnl_vector<int> MaxIndex;
    vnl_vector<int> MinIndex;
  };


  /** \brief Calculates minimum, maximum, mean value and their
  * corresponding indices in a given ROI. As input the function
  * needs an image and a mask. It returns a ImageExtrema object. */
  template <typename TPixel, unsigned int VImageDimension >
  ImageExtrema CalculateExtremaWorld(
    const itk::Image<TPixel, VImageDimension> *inputImage,
    itk::Image<unsigned short, VImageDimension> *maskImage);


  /** \brief Calculates the hotspot statistics within a given
  * ROI. As input the function needs an image, a mask which
  * represents the ROI and a radius which defines the size of
  * the sphere. The function returns a Statistics object. */
  template < typename TPixel, unsigned int VImageDimension>
  Statistics CalculateHotspotStatistics(
    const itk::Image<TPixel, VImageDimension> *inputImage,
    itk::Image<unsigned short, VImageDimension> *maskImage,
    double radiusInMM);

  /** Connection from ITK to VTK */
  template <typename ITK_Exporter, typename VTK_Importer>
  void ConnectPipelines(ITK_Exporter exporter, vtkSmartPointer<VTK_Importer> importer)
  {
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
  }

  /** Connection from VTK to ITK */
  template <typename VTK_Exporter, typename ITK_Importer>
  void ConnectPipelines(vtkSmartPointer<VTK_Exporter> exporter, ITK_Importer importer)
  {
    importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

    importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
    importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
    importer->SetSpacingCallback(exporter->GetSpacingCallback());
    importer->SetOriginCallback(exporter->GetOriginCallback());
    importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

    importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

    importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
    importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
    importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
    importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
    importer->SetCallbackUserData(exporter->GetCallbackUserData());
  }


  void UnmaskedStatisticsProgressUpdate();

  void MaskedStatisticsProgressUpdate();

  template <unsigned int VImageDimension>
  itk::Size<VImageDimension>
  CalculateConvolutionKernelSize(double spacing[VImageDimension], double radiusInMM);

  template <unsigned int VImageDimension>
  itk::SmartPointer< itk::Image<float, VImageDimension> >
  GenerateHotspotSearchConvolutionKernel(double spacing[VImageDimension], double radiusInMM);

  /** Uses members m_HotspotRadiusInMM */
  template <typename TPixel, unsigned int VImageDimension>
  void
  InternalUpdateConvolutionImage( itk::Image<TPixel, VImageDimension>* inputImage );

  template < typename TPixel, unsigned int VImageDimension>
  void
  FillHotspotMaskPixels( itk::Image<TPixel, VImageDimension>* maskImage,
                         itk::Point<double, VImageDimension> sphereCenter,
                         double sphereRadiusInMM);

  /** m_Image contains the input image (e.g. 2D, 3D, 3D+t)*/
  mitk::Image::ConstPointer m_Image;

  mitk::Image::ConstPointer m_ImageMask;

  mitk::PlanarFigure::Pointer m_PlanarFigure;

  HistogramVector m_ImageHistogramVector;
  HistogramVector m_MaskedImageHistogramVector;
  HistogramVector m_PlanarFigureHistogramVector;

  HistogramType::Pointer m_EmptyHistogram;
  HistogramContainer m_EmptyHistogramContainer;


  StatisticsVector m_ImageStatisticsVector;
  StatisticsVector m_MaskedImageStatisticsVector;
  StatisticsVector m_PlanarFigureStatisticsVector;
  StatisticsVector m_MaskedImageHotspotStatisticsVector;

  Statistics m_EmptyStatistics;
  StatisticsContainer m_EmptyStatisticsContainer;

  unsigned int m_MaskingMode;
  bool m_MaskingModeChanged;

  /** m_InternalImage contains a image volume at one time step (e.g. 2D, 3D)*/
  mitk::Image::ConstPointer m_InternalImage;
  MaskImage3DType::Pointer m_InternalImageMask3D;
  MaskImage2DType::Pointer m_InternalImageMask2D;

  TimeStampVectorType m_ImageStatisticsTimeStampVector;
  TimeStampVectorType m_MaskedImageStatisticsTimeStampVector;
  TimeStampVectorType m_PlanarFigureStatisticsTimeStampVector;

  BoolVectorType m_ImageStatisticsCalculationTriggerVector;
  BoolVectorType m_MaskedImageStatisticsCalculationTriggerVector;
  BoolVectorType m_PlanarFigureStatisticsCalculationTriggerVector;

  double m_IgnorePixelValue;
  bool m_DoIgnorePixelValue;
  bool m_IgnorePixelValueChanged;

  itk::Object::Pointer m_HotspotSearchConvolutionImage; // itk::Image<TPixel, VImageDimension>

  unsigned int m_PlanarFigureAxis;    // Normal axis for PlanarFigure
  unsigned int m_PlanarFigureSlice;   // Slice which contains PlanarFigure
  int m_PlanarFigureCoordinate0;      // First plane-axis for PlanarFigure
  int m_PlanarFigureCoordinate1;      // Second plane-axis for PlanarFigure

  double m_HotspotRadiusInMM;
  bool m_CalculateHotspot;
  bool m_HotspotRadiusInMMChanged;

};

} // namespace

#endif
