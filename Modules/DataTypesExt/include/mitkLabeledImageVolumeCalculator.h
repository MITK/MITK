/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabeledImageVolumeCalculator_h
#define mitkLabeledImageVolumeCalculator_h

#include <MitkDataTypesExtExports.h>
#include <itkObject.h>

#include <mitkImage.h>
#include <mitkImageTimeSelector.h>

#include <itkImage.h>

namespace mitk
{
  /**
   * \brief Calculates the volume and centroid for each label in a labeled image.
   *
   * Labels are expected to be of an unsigned integer type. The class iterates
   * over all voxels, groups them by label value, and computes each label's
   * volume (voxel count * voxel volume) and centroid (center of mass).
   *
   * \note Currently only the first time step of the image is processed.
   *
   * \sa mitk::Image
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT LabeledImageVolumeCalculator : public itk::Object
  {
  public:
    /** \brief Vector of per-label volume values. */
    typedef std::vector<double> VolumeVector;
    /** \brief Vector of per-label centroid points. */
    typedef std::vector<Point3D> PointVector;

    mitkClassMacroItkParent(LabeledImageVolumeCalculator, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the input image.
     * \param[in] _arg The labeled image to analyze.
     */
    itkSetConstObjectMacro(Image, mitk::Image);

    /**
     * \brief Perform the volume and centroid calculation.
     *
     * \pre An input image must have been set via SetImage().
     */
    virtual void Calculate();

    /**
     * \brief Get the volume for a specific label.
     *
     * \param[in] label The label index.
     * \return The volume in physical units, or 0.0 if the label does not exist.
     */
    double GetVolume(unsigned int label) const;

    /**
     * \brief Get the centroid (center of mass) for a specific label.
     *
     * \param[in] label The label index.
     * \return The centroid point in world coordinates, or a zero point if the label does not exist.
     */
    const Point3D &GetCentroid(unsigned int label) const;

    /**
     * \brief Get the vector of volumes for all labels.
     * \return Const reference to the volume vector.
     */
    const VolumeVector &GetVolumes() const;

    /**
     * \brief Get the vector of centroids for all labels.
     * \return Const reference to the centroid vector.
     */
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

#endif
