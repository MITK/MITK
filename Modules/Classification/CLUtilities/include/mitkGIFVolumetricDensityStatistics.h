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

#ifndef mitkGIFVolumetricDensityStatistics_h
#define mitkGIFVolumetricDensityStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
    /**
    * \brief Calculates Volumetric Density Features
    *
    * These features characterize the compactness of the volume and shape by comparing the volumes
    * of different volume and shape estimation methods.
    *
    * This feature calculator is activated by the option <b>-volume-density</b> or <b>-volden</b>.
    *
    * The features are calculated based on a mask. It is assumed that the mask is
    * of the type of an unsigned short image. All voxels with the value equal or greater than 1 are treated as masked.
    *
    * The volume and surface are compared to the volume \f$ V \f$ and surface \f$ A \f$ that is calculated
    * directly from the mask. The following features are then defined:
    * - <b>Morphological Density::Volume density axis-aligned bounding box</b>: The axis-aligned bounding
    * box is defined as the minimum axis aligned box in 3D space that encloses all masked voxels.
    * It is calculated by using the maximum spacial extension of the mask. Based on the volume of  the
    * bounding box, \f$ V_{aabb} \f$, the feature is defined as:
    * \f[ \textup{Volume density axis-aligned bounding box}= \frac{V}{V_{aabb}} \f]
    * - <b>Morphological Density::Surface density axis-aligned bounding box</b>: As for the previous
    * feature, the axis-aligned bounding box is compared to the mask, this time using the surface of the
    * bounding box \f$ A_{aabb} \f$:
    * \f[ \textup{Surface density axis-aligned bounding box}= \frac{A}{A_{aabb}} \f]
    * - <b>Morphological Density::Volume density oriented minimum bounding box</b>: A three-dimensional
    * bounding box is defined using the box with the minimum volume. We do not use an estimation
    * for this feature, which makes the calculation of this feature slow. Based on the volume of the
    * bounding box, \f$ V_{ombb} \f$, the feature is defined as:
    * \f[ \textup{Volume density oriented minimum bounding box}= \frac{V}{V_{ombb}} \f]
    * - <b>Morphological Density::Surface density axis-aligned bounding box</b>: As for the previous
    * feature, theminimum oriented  bounding box is compared to the mask, this time using the surface of the
    * bounding box \f$ A_{ombb} \f$:
    * \f[ \textup{Surface density axis-aligned bounding box}= \frac{A}{A_{ombb}} \f]
    * - <b>Morphological Density::Volume density approx. enclosing ellipsoid</b>: Using a Principal Component Analysis (PCA)
    * of the spacial coordinates gives the three main axis of the mask. They correspond to the length of
    * a eclipse enclosing the mask. The length of the axis of the eclipse are given by the eigenvalues of the
    * decomposition: \f$ a = 2 \sqrt{\lambda_1} \f$, \f$ b = 2 \sqrt{\lambda_2} \f$, and \f$ c = 2 \sqrt{\lambda_3} \f$
    * with \f$\lambda_x\f$ being the sorted eigenvalues (higher number indicates larger values). The volume
    * of the enclosing eclipse can be estimated by \f$ V_{aee} = 4 \pi a b c \f$:
    * \f[ \textup{Volume density approx. enclosing ellipsoid}= \frac{V}{V_{aee}} \f]
    * - <b>Morphological Density::Surface density approx. enclosing ellipsoid</b>: As for the previous
    * feature, the surface of the enclosing ellipsoid is used. To simplify the calulation of it, an approximation (20 iterations)
    * for the surface is used (\f$ \alpha = \sqrt{1-\frac{b^2}{a^2}} \f$, \f$ \beta = \sqrt{1-\frac{c^2}{a^2}} \f$):
    * \f[ A_{aee} = 2 \pi a b \frac{\alpha^2 + \beta^2}{\alpha \beta} \sum_v^\infty \frac{(a \beta)^v}{1-a v^2} \f]
    * \f[ \textup{Surface density approx. enclosing ellipsoid}= \frac{A}{A_{aee}} \f]
    * - <b>Morphological Density::Volume density approx. minimum volume enclosing ellipsoid</b>:
    * The volume is compared to the volume of the minimum enclosing ellipsoid. While this ellipsoid can be
    * found by brute-force calculation, this is quite time-consuming. It is therefore estimated using
    * Khachiyan's Algorithm (Khachiyan, Rounding of Polytopes in the Real Number Model of Computation. Mathematics of Operations Research 1996)
    * The so-found ellipsoid is described by the lengths \f$a, b, c \f$ of its axis. The volume is then
    * defined as \f$ V_{mvee} = 4 \pi a b c \f$ and the feature given by:
    * \f[ \textup{Volume density approx. minimum volume enclosing ellipsoid}= \frac{V}{V_{mvee}} \f]
    * - <b>Morphological Density::Surface density approx. minimum volume enclosing ellipsoid</b>: As for the previous
    * feature, the surface of the minimum volume enclosing ellipsoid is used. To simplify the calulation of it,
    * an approximation with 20 iterations instead of infinite iterations is used for the calculation of the
    * the surface (\f$ \alpha = \sqrt{1-\frac{b^2}{a^2}} \f$, \f$ \beta = \sqrt{1-\frac{c^2}{a^2}} \f$):
    * \f[ A_{mvee} = 2 \pi a b \frac{\alpha^2 + \beta^2}{\alpha \beta} \sum_v^\infty \frac{(a \beta)^v}{1-a v^2} \f]
    * \f[ \textup{Surface density approx. minimum volume enclosing ellipsoid}= \frac{A}{A_{mvee}} \f]
    * - <b>Morphological Density::Volume density convex hull</b>: The volume of the density
    * hull is calculated using a convex mesh and then calculating the volume of this mesh \f$V_{convex} \f$.
    * The feature is then calculated using:
    * \f[ \textup{Volume density convex hull}= \frac{V}{V_{convex}} \f]
    * - <b>Morphological Density::Surface density convex hull</b>: The surface of the density
    * hull is calculated using a convex mesh and then calculating the surface  of this mesh \f$A_{convex} \f$.
    * The feature is then calculated using:
    * \f[ \textup{Volume density convex hull}= \frac{A}{A_{convex}} \f]
    * - <b>Morphological Density::Volume integrated intensity</b>: Integrated intensity is the
    * average intensity times the volume. It is often used in conjunction with PET-images, where
    * this feature is also called "total legion glycolysis". It is defined using the volume \f$V \f$, the
    * number of masked voxels \f$ N_v \f$ and the intensity of each voxel \f$ x_i \f$:
    * \f[ \textup{Volume integrated intensity}= V \frac{1}{N_v} \sum x_i \f]
    * - <b>Morphological Density::Volume Moran's I index</b>: Moran's I index is an measure for
    * the spacial autocorrelation. It is defined using the inverse spacial distance between two voxels \f$i, j \f$ \f$w_{ij} \f$,
    * the number of masked voxels \f$ N_v \f$, the intensity of each voxel \f$ x_i \f$,
    * and the mean intensity of all masked voxels \f$ \mu = \frac{1}{N_v} sum x_i \f$:
    * \f[ \textup{Volume Moran's I index}= \frac{N_v}{\sum_i \sum_j w_{ij}} \frac{\sum_i \sum_j (x_i - \mu) (x_j -\mu)}{\sum_i (x_i - \mu)^2 } \enspace \enspace {; i \neq j} \f]
    * - <b>Morphological Density::Volume Geary's C measure</b>: Geary's C meansure is similar to Moran's I index.
    * However, it is more sensitive to grey level differences and spacial autocorrelation:
    * the spacial autocorrelation. It is defined using the inverse spacial distance between two voxels \f$i, j \f$ \f$w_{ij} \f$,
    * the number of masked voxels \f$ N_v \f$, the intensity of each voxel \f$ x_i \f$,
    * and the mean intensity of all masked voxels \f$ \mu = \frac{1}{N_v} sum x_i \f$:
    * \f[ \textup{Volume Geary's C measure}= \frac{N_v - 1}{2 \sum_i \sum_j w_{ij}} \frac{ \sum_i \sum_j w_{ij} (x_i - x_j)^2 }{\sum_i (x_i - \mu)^2 } \enspace \enspace {; i \neq j} \f]
    */
  class MITKCLUTILITIES_EXPORT GIFVolumetricDensityStatistics : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFVolumetricDensityStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFVolumetricDensityStatistics();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames();

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

  private:
  };
}
#endif //mitkGIFVolumetricDensityStatistics_h
