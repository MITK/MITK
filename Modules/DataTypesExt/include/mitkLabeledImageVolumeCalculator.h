/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_LABELEDIMAGEVOLUMECALCULATOR_H
#define _MITK_LABELEDIMAGEVOLUMECALCULATOR_H

#include "MitkDataTypesExtExports.h"
#include <itkObject.h>

#include "mitkImage.h"
#include "mitkImageTimeSelector.h"

#include <itkImage.h>

namespace mitk
{
  /**
   * \brief Class for calculating the volume (or area) for each label in a
   * labeled image.
   *
   * Labels are expected to be of an unsigned integer type.
   *
   * TODO: Extend class for time resolved images
   */
  class MITKDATATYPESEXT_EXPORT LabeledImageVolumeCalculator : public itk::Object
  {
  public:
    typedef std::vector<double> VolumeVector;
    typedef std::vector<Point3D> PointVector;

    mitkClassMacroItkParent(LabeledImageVolumeCalculator, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetConstObjectMacro(Image, mitk::Image);

    virtual void Calculate();

    double GetVolume(unsigned int label) const;

    const Point3D &GetCentroid(unsigned int label) const;

    const VolumeVector &GetVolumes() const;

    const PointVector &GetCentroids() const;

  protected:
    LabeledImageVolumeCalculator();

    ~LabeledImageVolumeCalculator() override;

    template <typename TPixel, unsigned int VImageDimension>
    void _InternalCalculateVolumes(itk::Image<TPixel, VImageDimension> *image,
                                   LabeledImageVolumeCalculator *volumeCalculator,
                                   BaseGeometry *geometry);

    ImageTimeSelector::Pointer m_InputTimeSelector;

    Image::ConstPointer m_Image;

    VolumeVector m_VolumeVector;
    PointVector m_CentroidVector;

    Point3D m_DummyPoint;
  };
}

#endif // #define _MITK_LABELEDIMAGEVOLUMECALCULATOR_H
