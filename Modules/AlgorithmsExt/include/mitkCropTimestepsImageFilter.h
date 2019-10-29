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

#ifndef mitkCropTimestepsImageFilter_h
#define mitkCropTimestepsImageFilter_h

#include "MitkAlgorithmsExtExports.h"

#include <mitkSubImageSelector.h>

namespace mitk
{
  /** \brief Crops timesteps at 2D+t and 3D+t images
    * \details The filter is able to crop timesteps in front and/or at the end.
    * Internally, a new image is created with the remaining volumes. The geometries and properties of
    * the input image are transferred to the output image.
   */
  class MITKALGORITHMSEXT_EXPORT CropTimestepsImageFilter : public SubImageSelector
  {
  public:
    mitkClassMacro(CropTimestepsImageFilter, SubImageSelector)
    itkFactorylessNewMacro(Self)

    /*!
    * \brief Sets the input image
    * \pre the image has 4 Dimensions
    * \pre the image has >1 timestep
    * \pre the image is valid (valid geometry and volume)
    */
    void SetInput(const InputImageType* image) override;
    void SetInput(unsigned int index, const InputImageType* image) override;

    /*!
    * \brief The last timestep to retain
    * \details Set to the maximum timestep of the input as default
    * \note if the last timestep is larger than the maximum timestep of the input,
    * it is corrected to the maximum timestep.
    * \exception if (LowerBoundaryTimestep > UpperBoundaryTimestep)
    */
    itkSetMacro(UpperBoundaryTimestep, unsigned int);
    itkGetConstMacro(UpperBoundaryTimestep, unsigned int);

    /*!
    * \brief The first timestep to retain
    * \details Set to 0 as default
    * \exception if (LowerBoundaryTimestep > UpperBoundaryTimestep)
    */
    itkSetMacro(LowerBoundaryTimestep, unsigned int);
    itkGetConstMacro(LowerBoundaryTimestep, unsigned int);

  private:
    using Superclass::SetInput;

    CropTimestepsImageFilter() = default;
    ~CropTimestepsImageFilter() override = default;

    void GenerateData() override;
    void VerifyInputInformation() override;
    void VerifyInputImage(const mitk::Image* inputImage) const;
    void GenerateOutputInformation() override;
    mitk::SlicedData::RegionType ComputeDesiredRegion() const;
    mitk::TimeGeometry::Pointer AdaptTimeGeometry(mitk::TimeGeometry::ConstPointer sourceGeometry, unsigned int startTimestep, unsigned int endTimestep) const;

    unsigned int m_UpperBoundaryTimestep = std::numeric_limits<unsigned int>::max();
    unsigned int m_LowerBoundaryTimestep = 0;

    mitk::SlicedData::RegionType m_DesiredRegion;
  };
}

#endif
