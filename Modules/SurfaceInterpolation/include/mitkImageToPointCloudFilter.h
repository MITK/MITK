/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToPointCloudFilter_h
#define mitkImageToPointCloudFilter_h

#include <MitkSurfaceInterpolationExports.h>
#include <mitkImageToUnstructuredGridFilter.h>

#include <itkCastImageFilter.h>

namespace mitk
{
  /**
  * @brief The filter extracts the edge pixels of an image as points and stores
  * them in an UnstructuredGrid. Every pixel which grey value is between the
  * mean +- standard deviation * (2 or 3), will be extracted as point. The
  * DetectionMethod can be set to choose if the doubled or tripled standard
  * deviation is used.
  */
  class MITKSURFACEINTERPOLATION_EXPORT ImageToPointCloudFilter : public ImageToUnstructuredGridFilter
  {
  public:
    /**
     * @brief The method which calculates and extracts the edge pixels/points.
     * For the edge detection the laplacian filter is used and for extraction
     * the standard deviation multiplied with 2, 3 or 4 (depending on selected
     * method) is used.
     */
    enum DetectionMethod
    {
      LAPLACIAN_STD_DEV2 = 0,
      LAPLACIAN_STD_DEV3 = 1,
      LAPLACIAN_STD_DEV4 = 2
    };

    mitkClassMacro(ImageToPointCloudFilter, ImageToUnstructuredGridFilter);

    itkFactorylessNewMacro(Self);

      typedef itk::Image<double, 3> FloatImageType;

      /** Returns the selected DetectionMethod */
      itkGetMacro(Method, DetectionMethod);

      /** Sets the DetectionMethod for edge detection and extraction */
      itkSetMacro(Method, DetectionMethod);

        protected :

      /** This method is called by Update(). */
      void GenerateData() override;

    /** Initializes the output information */
    void GenerateOutputInformation() override;

    /** Constructor */
    ImageToPointCloudFilter();

    /** Destructor */
    ~ImageToPointCloudFilter() override;

  private:
    /** Uses the laplace filter to create an image and extracts a pixel as point
     *  if the grey value is between the mean +- standard deviation * (2 or 3) */
    template <typename TPixel, unsigned int VImageDimension>
    void StdDeviations(itk::Image<TPixel, VImageDimension> *image, int amount);

    /** The geometry of the input image for transforming the pixel coordinates to
     *  world coordinates */
    mitk::BaseGeometry *m_Geometry;

    /** The selected detection method */
    DetectionMethod m_Method;
  };
}
#endif
