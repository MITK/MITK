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

#ifndef mitkGIFVolumetricStatistics_h
#define mitkGIFVolumetricStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calulates simpel shape-related features.
  *
  * This class can be used to calculate simple, shape-related features describing
  * a given segmentation. There are no parameters that can be externaly set.
  *
  * This feature calculator is activated by the option "<b>-volume</b>" or "<b>-vol</b>"
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image and all voxels with an value larger or equal
  * to one are treated as masked. (Standard MITK mask)
  *
  * Some of the features are calculated twice using different methods. For voxel-
  * based approaches, the corresponding parameter is calcualted using the voxel,
  * for example the volume is then calculated by multiplying the volume of a
  * single volume with the number of voxels in the mask. In the second method, the
  * mesh based appraoch, a mesh is created prior to the feature calculation which
  * is then done using the features.
  *
  * Another difference between two features might be the evaluation of invalid
  * values within the image. There are two possibilities: By default, only those
  * voxels are used with an valid intensity value, i.e. where the value is not
  * infinite or NaN. The second possibility is not correcting for these voxels
  * and only looking at the mask. Features that use these method are marked as
  * "(uncorrected)"
  *
  * The resulting features are:
  * - <b>Volumetric Features:: Voxel Volume</b>: \f$ V_{single\_voxel} \f$ , the volume of an single volume, calculated as the
  * multiplication of the voxel spacing in all directions.
  * - <b>Volumetric Features:: Volume (voxel based)</b>: \f$ V_{voxel} \f$, the volume of the masked area. Calulated by
  * multiplying the numer of voxels with the Voxel Volume.
  * - <b>Volumetric Features:: Volume (mesh based)</b>: \f$ V_{shape} \f$, The volume based on the mesh-representation of
  * the mask.
  * - <b>Volumetric Features:: Surface (voxel based)</b>: \f$ A_{voxel} \f$, the surface of the given mask. It is calulated
  * by summing the surfaces between a masked and an unmasked voxel.
  * - <b>Volumetric Features:: Surface (mesh based)</b>: \f$ A_{mesh} \f$, the surface of the given mask calculated using
  * the mask representation
  * - <b>Volumetric Features:: Surface to volume ration (voxel based)</b>: The ratio between voxel based surface and voxel based
  * volume given as: \f[ F_{av\_voxel}=\frac{A_{voxel}}{V_{voxel}} \f]
  * - <b>Volumetric Features:: Surface to volume ration (mesh based)</b>: The ratio between voxel based surface and voxel based
  * volume given as: \f[ F_{av\_mesh}=\frac{A_{mesh}}{V_{mesh}} \f]
  * - <b>Volumetric Features:: Compactness 1 (voxel based)</b>:
  * - <b>Volumetric Features:: Compactness 1 (mesh based)</b>:
   The compatness is a measure how spheric a shape is given.
  * Compactness 1 is defined as:
  * \f[ F_{compactness\_1} = \frac{V}{\pi^{1/2} A^{3/2}}\f]
  * - <b>Volumetric Features:: Compactness 1 old (voxel based)</b>:
  * - <b>Volumetric Features:: Compactness 1 old (mesh based)</b>: Some implementations use a slightly different definition of
  * compactness 1. Although this is most likely an error and leads to an non-dimensionless feature,
  * this defition is still calculated as:
  * \f[ F_{compactness\_1\_old} = \frac{V}{\pi^{1/2} A^{2/3}}\f]
  * - <b>Volumetric Features:: Compactness 2 (voxel based)</b>:
  * - <b>Volumetric Features:: Compactness 2 (mesh based)</b>: The compatness is a measure how spheric a shape is given.
  * Compactness 2 is defined as:
  * \f[ F_{compactness\_1} = 36 \pi \frac{V^2}{A^3}\f]
  * - <b>Volumetric Features::Sphericity (voxel based)</b>:
  * - <b>Volumetric Features::Sphericity (mesh based)</b>: Sphericity is measure of how sphere-like a shape is:
  * \f[ F_{sphericity} = \frac{(36 \pi V^2)^{1/3}}{A} \f]
  * - <b>Volumetric Features::Asphericity (voxel based)</b>:
  * - <b>Volumetric Features::Asphericity (mesh based)</b>: Sphericity is measure of how sphere-like a shape is:
  * \f[ F_{asphericity} = \left(\frac{1}{36 \pi }\frac{(A^3}{V^2}\right)^{1/3}  - 1 \f]
  * - <b>Volumetric Features::Spherical disproportion (voxel based)</b>:
  * - <b>Volumetric Features::Spherical disproportion (mesh based)</b>: Sphericity is measure of how sphere-like a shape is:
  * \f[ F_{spherical\_disproportion} = \frac{A}{4\pi R^2}= \frac{A}{\left(36\pi V^2\right)^{1/3}} \f]
  * - <b>Volumetric Features:: Maximum 3D diameter</b>: This is the largest distance between the centers of two voxels that
  * are masked.
  * - <b>Volumetric Features::Bounding box volume</b>: The bounding box volume is the volume of the smallest axis-aligned box
  * that encapuslates all voxel centres.
  * - <b>Volumetric Features::Centre of mass shift</b>:
  * - <b>Volumetric Features::Centre of mass shift (uncorrected)</b>: This is the distance between two centres of mass,
  * namely the geometric centre and the weighted centre. The geometric centre is the mean position
  * of all masked voxels, and the weighted centre is the mean position if the position of each
  * voxel is weighted according to its intensity.
  * - <b>Volumetric Features::PCA Major Axis length</b>:
  * - <b>Volumetric Features::PCA Major Axis length (uncorrected)</b>: A Principal component analysis (PCA) of the masekd voxel
  * positions will give the main orientation and elongation of the masked area. The resulting
  * eigenvectors of the PCA are sorted so that \f$ \lambda_{major}\geq  \lambda_{minor} \geq \lambda_{least}\f$.
  * The major axis length is defined as:
  * \f[ F_{pca\_major} = 4 \sqrt{\lambda_{major}} \f]
  * - <b>Volumetric Features::PCA Minor axis length</b>:
  * - <b>Volumetric Features::PCA Minor axis length</b>: The Minor axis length is defined as:
  * \f[ F_{pca\_minor} = 4 \sqrt{\lambda_{minor}} \f]
  * - <b>Volumetric Features::PCA Least axis length</b>:
  * - <b>Volumetric Features::PCA Least axis length</b>: The Minor axis length is defined as:
  * \f[ F_{pca\_Least} = 4 \sqrt{\lambda_{Least}} \f]
  */
  class MITKCLUTILITIES_EXPORT GIFVolumetricStatistics : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFVolumetricStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFVolumetricStatistics();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    FeatureNameListType GetFeatureNames() override;

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

  private:
  };
}
#endif //mitkGIFVolumetricStatistics_h
