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

#ifndef _MITKIMAGETOUNSTRUCTUREDGRIDFILTER_h__
#define _MITKIMAGETOUNSTRUCTUREDGRIDFILTER_h__

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridSource.h>
#include <mitkImage.h>


namespace mitk {
  /**
  * @brief Converts an Image into an UnstructuredGrid represented by Points.
  * The filter uses a Threshold to extract every pixel, with value higher than
  * the threshold, as point.
  * If no threshold is set, every pixel is extracted as a point.
  */

  class MITKALGORITHMSEXT_EXPORT ImageToUnstructuredGridFilter
      : public UnstructuredGridSource
  {
    public:

      mitkClassMacro(ImageToUnstructuredGridFilter, UnstructuredGridSource)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /** This method is called by Update(). */
      virtual void GenerateData() override;

      /** Initializes the output information */
      virtual void GenerateOutputInformation() override;

      /** Returns a const reference to the input image */
      const mitk::Image* GetInput(void);

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

      itkGetMacro(NumberOfExtractedPoints, int)

    protected:

      /** Constructor */
      ImageToUnstructuredGridFilter();

      /** Destructor */
      virtual ~ImageToUnstructuredGridFilter();

      /**
       * Access method for extracting the points from the input image
       */
      template<typename TPixel, unsigned int VImageDimension>
      void ExtractPoints(const itk::Image<TPixel, VImageDimension>* image);

  private:

      /**
       * Geometry of the input image, needed to tranform the image points
       * into world points
       */
      mitk::BaseGeometry* m_Geometry;

      /** The number of points extracted by the filter */
      int m_NumberOfExtractedPoints;

      /** Threshold for extracting the points */
      double m_Threshold;

      /** The output of the filter, which contains the extracted points */
      mitk::UnstructuredGrid::Pointer m_UnstructGrid;

  };

} // namespace mitk

#endif //_MITKIMAGETOUNSTRUCTUREDGRIDFILTER_h__


