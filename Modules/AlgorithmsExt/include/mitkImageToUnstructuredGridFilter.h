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
  * @brief Converts an Image into an UnstructuredGrid represented by Points.
  * The filter uses a Threshold to extract every pixel, with value higher than
  * the threshold, as point.
  * If no threshold is set, every pixel is extracted as a point.
  */

  class MITKALGORITHMSEXT_EXPORT ImageToUnstructuredGridFilter : public UnstructuredGridSource
  {
  public:
    mitkClassMacro(ImageToUnstructuredGridFilter, UnstructuredGridSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** This method is called by Update(). */
      void GenerateData() override;

    /** Initializes the output information */
    void GenerateOutputInformation() override;

    /** Returns a const reference to the input image */
    const mitk::Image *GetInput(void) const;

    mitk::Image *GetInput(void);

    /** Set the source image. As input every mitk 3D image can be used. */
    using itk::ProcessObject::SetInput;
    virtual void SetInput(const mitk::Image *image);

    /**
     * Set the threshold for extracting points. Every pixel, which value
     * is higher than this value, will be a point.
    */
    void SetThreshold(double threshold);

    /** Returns the threshold */
    double GetThreshold();

    /** Returns the number of extracted points after edge detection */
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
     * Geometry of the input image, needed to tranform the image points
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
