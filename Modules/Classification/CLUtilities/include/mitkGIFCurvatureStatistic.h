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

#ifndef mitkGIFCurvatureStatistic_h
#define mitkGIFCurvatureStatistic_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{

  /**
  * \brief Calculates features based on the co-occurence matrix.
  *
  * The Curvature is a measure for the bending of a surface and is therefore a measure for the description of the
  * surface of an segmentation.
  *
  * THe curvature is calculated for each point of the surface of the given object and then a combined measure is
  * produced. It measures the divergence of the orientation of an curve from the
  * tangent of the curve. There are multiple ways to calculate the Curvature:
  *
  * <b> Gaussian Curvature</b>: The discrete gaussian curvature (K) is computed as \f$K(\textup{Corner Point v}) = 2 * \pi - \sum_{\textup{Neighoubring Voxel Surfaces f of v}} (\textup{Angle}_f \textup{at} v) \f$.
  * <b> Mean Curvature</b>:The mean curvature (H) is computed as \f$H(\textup{Corner Point v}) = \textup{average over edges e neighbouring v of H(e)} \f$.
  * with \f$H(edge e) = length(e)*dihedral_angle(e)\f$
  * <b>Maximum</b> (\f$k_max\f$) and <b>Minimum</b> (\f$k_min\f$) Principal Curvatures
  * \f$k_max = H + sqrt(H^2 - K)\f$
  * \f$k_min = H - sqrt(H^2 - K)\f$
  * Excepting spherical and planar surfaces which have equal principal curvatures,
  * the curvature at a point on a surface varies with the direction one "sets off"
  * from the point. For all directions, the curvature will pass through two extrema:
  * a minimum (\f$k_min\f$) and a maximum (\f$k_max\f$) which occur at mutually orthogonal
  * directions to each other.
  *
  * This method does not take any parameters.
  *
  * This feature calculator is activated by the option <b>-curvature</b> or <b>-cur</b>.
  *
  * The features are calculated based on a mask, which is converted into a mesh.
  *
  * The following features are defined. All features are calculated for all four possible
  * curvation calculation methods (Gaussian, Mean, Minimum, Maximum). The principal way
  * of calculating these features is the same, the used curvation is indicated by <name> in the
  * feature name:
  *
  * - <b>Curvature Feature::Minimum <name> Curvature</b>:
  * The minimum curvature for the whole given mask
  * - <b>Curvature Feature::Maximum <name> Curvature</b>:
  * The maximum curvature for the whole given mask
  * - <b>Curvature Feature::Mean <name> Curvature</b>:
  * The mean curvature for the whole given mask
  * - <b>Curvature Feature::Standard Deviation <name> Curvature</b>:
  * The standard deviation curvature for the whole given mask
  * - <b>Curvature Feature::Skewness <name> Curvature</b>:
  * The skewness curvature for the whole given mask
  * - <b>Curvature Feature::Mean Positive <name> Curvature</b>:
  * The mean curvature of all positive curvatures from the whole given mask
  * - <b>Curvature Feature::Standard Deviation Positive <name> Curvature</b>:
  * The Standard Deviation curvature of all positive curvatures from the whole given mask
  * - <b>Curvature Feature::Skewness Positive <name> Curvature</b>:
  * The Skewness curvature of all positive curvatures from the whole given mask
  * - <b>Curvature Feature::Mean Negative <name> Curvature</b>:
  * The mean curvature of all Negative curvatures from the whole given mask
  * - <b>Curvature Feature::Standard Deviation Negative <name> Curvature</b>:
  * The Standard Deviation curvature of all Negative curvatures from the whole given mask
  * - <b>Curvature Feature::Skewness Negative <name> Curvature</b>:
  * The Skewness curvature of all Negative curvatures from the whole given mask
  */
  class MITKCLUTILITIES_EXPORT GIFCurvatureStatistic : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFCurvatureStatistic,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFCurvatureStatistic();

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
#endif //mitkGIFCurvatureStatistic_h
