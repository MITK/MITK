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

#ifndef _MITK_IMAGESTATISTICSCALCULATOR_H
#define _MITK_IMAGESTATISTICSCALCULATOR_H

#include <itkObject.h>
#include "MitkImageStatisticsExports.h"
#include <itkImage.h>
#include <itkTimeStamp.h>
#include "mitkImage.h"
#include "mitkPlanarFigure.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <itkImageRegionIteratorWithIndex.h>


#include <vtkSmartPointer.h>

// just a helper to unclutter our code
// to be replaced with references to m_Member (when deprecated public members in Statistics are removed)
#define mitkSetGetConstMacro(name, type) \
  virtual type Get##name() const \
  { \
    return this->name; \
  } \
  \
  virtual void Set##name(const type _arg) \
  { \
    if ( this->name != _arg ) \
    { \
      this->name = _arg; \
    } \
  }

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
 * statistics for a region called "hotspot". The hotspot is a sphere of
 * user-defined size and its location is chosen in a way that the average
 * pixel value within the sphere is maximized.
 *
 * \warning Hotspot calculation does not work in case of 2D-images!
 *
 * Note: currently time-resolved and multi-channel pictures are not properly
 * supported.
 *
 * \section HotspotStatistics_caption Calculation of hotspot statistics
 *
 * Since calculation of hotspot location and statistics is not
 * straight-forward, the following paragraphs will describe it in more detail.
 *
 * <b>Note: Calculation of hotspot statistics is optional and set to off by default.
 * Multilabel-masks are supported.</b>
 *
 * \subsection HotspotStatistics_description Hotspot Definition
 *
 * The hotspot of an image is motivated from PET readings. It is defined
 * as a spherical region of fixed size which maximizes the average pixel value
 * within the region.  The following image illustrates the concept: the
 * colored areas are different image intensities and the hotspot is located
 * in the hottest region of the image.
 *
 * <b> Note:</b> Only hotspots are calculated for which the whole hotspot-sphere is
 *     inside the image by default. This behaviour can be changed by
 *     by calling SetHotspotMustBeCompletlyInsideImage().
 * \warning Note that SetHotspotMustBeCompletlyInsideImage(false) may overrate
 *          "hot" regions at image borders, because they have a stronger influence on the
 *          mean value! Think clearly about this fact and make sure this is what you
 *          want/need in your application, before calling
 *          SetHotspotMustBeCompletlyInsideImage(false)!
 *
 *
 * \image html hotspotexample.JPG
 *
 * \subsection HotspotStatistics_calculation Hotspot Calculation
 *
 * Since only the size of the hotspot is known initially, we need to calculate
 * two aspects (both implemented in CalculateHotspotStatistics() ):
 * - the hotspot location
 * - statistics of the pixels within the hotspot.
 *
 * Finding the hotspot location requires to calculate the average value at each
 * position. This is done by convolution of the image with a sperical kernel
 * image which reflects partial volumes (important in the case of low-resolution
 * PET images).
 *
 * Once the hotspot location is known, calculating the actual statistics is a
 * simple task which is implemented in CalculateHotspotStatistics() using a second
 * instance of the ImageStatisticsCalculator.
 *
 * <b>Step 1: Finding the hotspot by image convolution</b>
 *
 * As described above, we use image convolution with a rasterized sphere to
 * average the image at each position. To handle coarse resolutions, which would
 * normally force us to decide for partially contained voxels whether to count
 * them or not, we supersample the kernel image and use non-integer kernel values
 * (see GenerateHotspotSearchConvolutionKernel()), which reflect the volume part that is contained in the
 * sphere. For example, if three subvoxels are inside the sphere, the corresponding
 * kernel voxel gets a value of 0.75 (3 out of 4 subvoxels, see 2D example below).
 *
 * \image html convolutionkernelsupersampling.jpg
 *
 * Convolution itself is done by means of the itkFFTConvolutionImageFilter.
 * To find the hotspot location, we simply iterate the averaged image and find a
 * maximum location (see CalculateExtremaWorld()). In case of images with multiple
 * maxima the method returns value and corresponding index of the extrema that is
 * found by the iterator first.
 *
 * <b>Step 2: Computation of hotspot statistics</b>
 *
 * Once the hotspot location is found, statistics for the region are calculated
 * by simply iterating the input image and regarding all pixel centers inside the
 * hotspot-sphere for statistics.
 * \warning Index positions of maximum/minimum are not provided, because they are not necessarily unique
 * \todo If index positions of maximum/minimum are required, output needs to be changed to multiple positions / regions, etc.
 *
 * \subsection HotspotStatistics_tests Tests
 *
 * To check the correctness of the hotspot calculation, a special class
 * (\ref hotspottestdoc) has been created, which generates images with
 * known hotspot location and statistics. A number of unit tests use this class
 * to first generate an image of known properites and then verify that
 * ImageStatisticsCalculator is able to reproduce the known statistics.
 *
*/
class MitkImageStatistics_EXPORT ImageStatisticsCalculator : public itk::Object
{
public:

  /** \brief Enum for possible masking modi. */
  enum
  {
    MASKING_MODE_NONE = 0,
    MASKING_MODE_IMAGE = 1,
    MASKING_MODE_PLANARFIGURE = 2
  };

  typedef itk::Statistics::Histogram<double> HistogramType;
  typedef HistogramType::ConstIterator HistogramConstIteratorType;

  /** \brief Class for common statistics, includig hotspot properties. */
  class MitkImageStatistics_EXPORT Statistics
  {
  public:

    Statistics(bool withHotspotStatistics = true);
    Statistics(const Statistics& other);

    virtual ~Statistics();

    Statistics& operator=(Statistics const& stats);

    const Statistics& GetHotspotStatistics() const;  // real statistics
    Statistics& GetHotspotStatistics();  // real statistics
    bool HasHotspotStatistics() const;
    void SetHasHotspotStatistics(bool hasHotspotStatistics); // set a flag. if set, return empty hotspotstatistics object

    void Reset(unsigned int dimension = 2);

    mitkSetGetConstMacro(Label, unsigned int)
    mitkSetGetConstMacro(N, unsigned int)
    mitkSetGetConstMacro(Min, double)
    mitkSetGetConstMacro(Max, double)
    mitkSetGetConstMacro(Mean, double)
    mitkSetGetConstMacro(Median, double)

    double GetVariance() const;
    /** \brief Set variance
    *
    * This method checks whether the variance is negative:
    *   The reason that the variance may be negative is that the underlying itk::LabelStatisticsImageFilter uses a naïve algorithm
    *   for calculating the variance ( http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance ) which can lead to negative values
    *   due to rounding errors.
    *
    * If the variance is negative the value will be set to 0.0, else the given value will be set.
    */
    void SetVariance( const double );

    double GetSigma() const;
    /** \brief Set standard deviation (sigma)
    *
    * This method checks if the given standard deviation is a positive value. This is done because the underlying itk::LabelStatisticsImageFilter uses
    * a naïve algorithm to calculate the variance. This may lead to a negative variance and because the square root of the variance is taken it also
    * leads to NaN for sigma.
    *
    * If the given value is not reasonable the value will be set to 0.0, else the given value will be set.
    *
    * \see SetVariance()
    */
    void SetSigma( const double );

    mitkSetGetConstMacro(RMS, double)
    mitkSetGetConstMacro(MinIndex, vnl_vector<int>)
    mitkSetGetConstMacro(MaxIndex, vnl_vector<int>)
    mitkSetGetConstMacro(HotspotIndex, vnl_vector<int>)

  private:
   unsigned int Label;
   unsigned int N;
   double Min;
   double Max;
   double Mean;
   double Median;
   double Variance;
   double Sigma;
   double RMS;
   vnl_vector<int> MinIndex;
   vnl_vector<int> MaxIndex;

   Statistics* m_HotspotStatistics;

   bool m_HasHotspotStatistics;
    vnl_vector<int> HotspotIndex;     //< index of hotspotsphere origin
  };

  typedef std::vector< HistogramType::ConstPointer > HistogramContainer;
  typedef std::vector< Statistics > StatisticsContainer;

  mitkClassMacro( ImageStatisticsCalculator, itk::Object );
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Automatically calculate bin size to obtain 200 bins. */
  void SetUseDefaultBinSize(bool useDefault);

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

  /** \brief Set bin size for histogram resolution.*/
  void SetHistogramBinSize( unsigned int size);

  /** \brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramBinSize();

  /** \brief Sets the radius for the hotspot */
  void SetHotspotRadiusInMM (double hotspotRadiusInMM);

  /** \brief Returns the radius of the hotspot */
  double GetHotspotRadiusInMM();

  /** \brief Sets whether the hotspot should be calculated */
  void SetCalculateHotspot(bool calculateHotspot);

  /** \brief Returns true whether the hotspot should be calculated, otherwise false */
  bool IsHotspotCalculated();

  /** \brief Sets flag whether hotspot is completly inside the image. Please note that if set to false
      it can be possible that statistics are calculated for which the whole hotspot is not inside the image!

      \warning regarding positions at the image centers may produce unexpected hotspot locations, please see \ref HotspotStatistics_description
  */
  void SetHotspotMustBeCompletlyInsideImage(bool hotspotIsCompletlyInsideImage, bool warn = true);

  /** \brief Returns true if hotspot has to be completly inside the image. */
  bool GetHotspotMustBeCompletlyInsideImage() const;

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
  bool GetPrincipalAxis( const BaseGeometry *geometry, Vector3D vector,
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

  class ImageExtrema
  {
  public:
    bool Defined;
    double Max;
    double Min;
    vnl_vector<int> MaxIndex;
    vnl_vector<int> MinIndex;

    ImageExtrema()
    :Defined(false)
    ,Max(itk::NumericTraits<double>::min())
    ,Min(itk::NumericTraits<double>::max())
    {
    }
  };


  /** \brief Calculates minimum, maximum, mean value and their
  * corresponding indices in a given ROI. As input the function
  * needs an image and a mask. Returns an ImageExtrema object. */
  template <typename TPixel, unsigned int VImageDimension >
  ImageExtrema CalculateExtremaWorld(
    const itk::Image<TPixel, VImageDimension> *inputImage,
    itk::Image<unsigned short, VImageDimension> *maskImage,
    double neccessaryDistanceToImageBorderInMM,
    unsigned int label);


  /** \brief Calculates the hotspot statistics depending on
  * masking mode. Hotspot statistics are calculated for a
  * hotspot which is completly located inside the image by default. */
  template < typename TPixel, unsigned int VImageDimension>
  Statistics CalculateHotspotStatistics(
    const itk::Image<TPixel, VImageDimension> *inputImage,
    itk::Image<unsigned short, VImageDimension> *maskImage,
    double radiusInMM,
    bool& isHotspotDefined,
    unsigned int label);

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

  /** \brief Returns size of convolution kernel depending on spacing and radius. */
  template <unsigned int VImageDimension>
  itk::Size<VImageDimension>
  CalculateConvolutionKernelSize(double spacing[VImageDimension], double radiusInMM);

  /** \brief Generates image of kernel which is needed for convolution. */
  template <unsigned int VImageDimension>
  itk::SmartPointer< itk::Image<float, VImageDimension> >
  GenerateHotspotSearchConvolutionKernel(double spacing[VImageDimension], double radiusInMM);

  /** \brief Convolves image with spherical kernel image. Used for hotspot calculation.   */
  template <typename TPixel, unsigned int VImageDimension>
  itk::SmartPointer< itk::Image<TPixel, VImageDimension> >
  GenerateConvolutionImage( const itk::Image<TPixel, VImageDimension>* inputImage );

  /** \brief Fills pixels of the spherical hotspot mask. */
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

  unsigned int m_PlanarFigureAxis;    // Normal axis for PlanarFigure
  unsigned int m_PlanarFigureSlice;   // Slice which contains PlanarFigure
  int m_PlanarFigureCoordinate0;      // First plane-axis for PlanarFigure
  int m_PlanarFigureCoordinate1;      // Second plane-axis for PlanarFigure

  unsigned int m_HistogramBinSize;    ///Bin size for histogram resoluion.
  bool m_UseDefaultBinSize;
  double m_HotspotRadiusInMM;
  bool m_CalculateHotspot;
  bool m_HotspotRadiusInMMChanged;
  bool m_HotspotMustBeCompletelyInsideImage;

};

} // namespace

#endif
