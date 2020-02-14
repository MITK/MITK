/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGIFCooccurenceMatrix_h
#define mitkGIFCooccurenceMatrix_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{

  class MITKCLUTILITIES_EXPORT GIFCooccurenceMatrix : public AbstractGlobalImageFeature
  {
    /**
    * \brief Calculates features based on the co-occurence matrix.
    *
    * This filter calculates features based on the Co-Occurence Matrix.
    *
    * \warning{ This is a legacy class only. If possible, avoid to use it. Use
    * GIFCooccurenceMatrix2 instead.}
    */
    public:
      mitkClassMacro(GIFCooccurenceMatrix,AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

      GIFCooccurenceMatrix();

      /**
      * \brief Calculates the Cooccurence-Matrix based features for this class.
      */
      FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

      /**
      * \brief Returns a list of the names of all features that are calculated from this class
      */
      FeatureNameListType GetFeatureNames() override;

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);

    void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList) override;
    void AddArguments(mitkCommandLineParser &parser) override;


    struct GIFCooccurenceMatrixConfiguration
    {
      double range;
      unsigned int direction;

      double MinimumIntensity;
      bool UseMinimumIntensity;
      double MaximumIntensity;
      bool UseMaximumIntensity;
      int Bins;
    };

    private:
    double m_Range;
  };

}
#endif //mitkGIFCooccurenceMatrix_h
