/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToUnstructuredGridFilter_h
#define mitkImageToUnstructuredGridFilter_h

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkImage.h>
#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridSource.h>

namespace mitk
{
  /**
   * \brief Converts an Image into an UnstructuredGrid represented by points.
   *
   * The filter extracts points from a 3D image. A threshold can be set so that
   * only pixels with values higher than the threshold are extracted as points.
   * If no threshold is set (default: -1.0), every pixel is extracted.
   * The resulting points are transformed from image index space to world coordinates
   * using the image geometry.
   *
   * \sa UnstructuredGrid
   * \sa UnstructuredGridSource
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT ImageToUnstructuredGridFilter : public UnstructuredGridSource
  {
  public:
    mitkClassMacro(ImageToUnstructuredGridFilter, UnstructuredGridSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief Performs the extraction of points from the input image. Called by Update(). */
      void GenerateData() override;

    /** \brief Initializes the output information based on the input image. */
    void GenerateOutputInformation() override;

    /**
     * \brief Returns a const pointer to the input image.
     * \return Const pointer to the input mitk::Image, or nullptr if not set.
     */
    const mitk::Image *GetInput(void) const;

    /**
     * \brief Returns a non-const pointer to the input image.
     * \return Pointer to the input mitk::Image, or nullptr if not set.
     */
    mitk::Image *GetInput(void);

    /**
     * \brief Set the input image from which points will be extracted.
     * \param[in] image The 3D image to use as input.
     * \pre The image must be a valid 3D mitk::Image.
     */
    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::Image *image);

    /**
     * \brief Set the threshold for extracting points.
     *
     * Only pixels whose value exceeds this threshold will be extracted as points.
     * Set to -1.0 (default) to extract all pixels.
     *
     * \param[in] threshold The threshold value.
     */
    void SetThreshold(double threshold);

    /**
     * \brief Returns the current threshold value.
     * \return The threshold used for point extraction.
     */
    double GetThreshold();

    /**
     * \brief Returns the number of points extracted in the last filter execution.
     */
    itkGetMacro(NumberOfExtractedPoints, int);

      protected :

      /** Constructor */
      ImageToUnstructuredGridFilter();

    /** Destructor */
    ~ImageToUnstructuredGridFilter() override;

    /**
     * Access method for extracting the points from the input image
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ExtractPoints(const itk::Image<TPixel, VImageDimension> *image);

    /** The number of points extracted by the filter */
    int m_NumberOfExtractedPoints;
  private:

    /**
     * Geometry of the input image, needed to transform the image points
     * into world points
     */
    mitk::BaseGeometry *m_Geometry;

    /** Threshold for extracting the points */
    double m_Threshold;

    /** The output of the filter, which contains the extracted points */
    mitk::UnstructuredGrid::Pointer m_UnstructGrid;
  };

} // namespace mitk

#endif
