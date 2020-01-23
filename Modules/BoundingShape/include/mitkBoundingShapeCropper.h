/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BOUNDINGSHAPECROPPER_H
#define BOUNDINGSHAPECROPPER_H

#include "MitkBoundingShapeExports.h"
#include "mitkBoundingShapeCropper.h"
#include "mitkCommon.h"
#include "mitkGeometryData.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageToImageFilter.h"

#include "itkImage.h"

namespace mitk
{
  /** Documentation
  * @brief Crops or masks an Boundingbox defined by GeometryData out of an mitk Image
  *
  * Input Parameters are a mitk::GeometryData and an mitk::Image
  * Masking: Pixel on the outside of the bounding box will have a pixelvalue of m_OutsideValue
  * Cropping: Output image has the same size as the bounding box
  */
  //## @ingroup Process
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeCropper : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingShapeCropper, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
      * @brief Set geometry of the bounding object
      */
      void SetGeometry(const mitk::GeometryData *geometry);
    /**
    * @brief Get geometry of the bounding object
    */
    // const mitk::GeometryData* GetGeometryData() const;

    /**
    * @brief Sets and Gets the outside value for masking
    */
    itkSetMacro(OutsideValue, ScalarType);
    itkGetMacro(OutsideValue, ScalarType);
    /**
    * @brief Sets and Gets whether a masking or cropping needs to be performed
    */
    itkSetMacro(UseWholeInputRegion, bool);
    itkGetMacro(UseWholeInputRegion, bool);

    /**
    * @brief Sets and Gets the current timestep for images with 4 dimensons
    */
    itkSetMacro(CurrentTimeStep, ScalarType);
    itkGetMacro(CurrentTimeStep, ScalarType);
    /**
    *@brief Sets and Gets whether only one timestep is cropped / masked
    */
    itkSetMacro(UseCropTimeStepOnly, bool);
    itkGetMacro(UseCropTimeStepOnly, bool);

  protected:
    BoundingShapeCropper();
    ~BoundingShapeCropper() override;

    virtual const PixelType GetOutputPixelType();

    /**
    * @brief Reimplemented from ImageToImageFilter
    */
    void GenerateInputRequestedRegion() override;
    void GenerateOutputInformation() override;
    void GenerateData() override;

    /**
    * @brief Template Function for cropping and masking images with scalar pixel type
    */
    template <typename TPixel, unsigned int VImageDimension>
    void CutImage(itk::Image<TPixel, VImageDimension> *inputItkImage, int timeStep);

    /**
    *@brief Process the image and create the output
     **/
    virtual void ComputeData(mitk::Image *input3D, int boTimeStep);

    // virtual void ComputeData(mitk::LabelSetImage* image, int boTimeStep);

  private:
    /**
    *@brief GeometryData Type to capsulate all necessary components of the bounding object
     **/
    mitk::GeometryData::Pointer m_Geometry;

    /**
    * @brief scalar value for outside pixels (default: 0)
    */
    ScalarType m_OutsideValue;

    /**
    * @brief Use m_UseCropTimeStepOnly for only cropping a single time step(default: \a false)
    */
    bool m_UseCropTimeStepOnly;

    /**
    * @brief Current time step displayed
    */
    int m_CurrentTimeStep;
    /**
    * @brief Use m_UseWholeInputRegion for deciding whether a cropping or masking will be performed
    */
    bool m_UseWholeInputRegion;
    /**
    * @brief Select single input image in a timeseries
    */
    mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
    /**
    * @brief Select single output image in a timeseries
    */
    mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;
    /**
    * @brief Region of input needed for cutting
    */
    typedef itk::ImageRegion<5> RegionType;

    mitk::SlicedData::RegionType m_InputRequestedRegion;
    /**
    * @brief Time when Header was last initialized
    **/
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };
} // namespace mitk

#endif /* BOUNDINGSHAPECROPPER_H */
