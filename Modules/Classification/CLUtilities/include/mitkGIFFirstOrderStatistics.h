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

#ifndef mitkGIFFirstOrderStatistics_h
#define mitkGIFFirstOrderStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFFirstOrderStatistics : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFFirstOrderStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFFirstOrderStatistics();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames() override;

    itkGetConstMacro(Range,double);
    itkSetMacro(Range, double);
    itkGetConstMacro(HistogramSize,int);
    itkSetMacro(HistogramSize, int);
    itkGetConstMacro(UseCtRange,bool);
    itkSetMacro(UseCtRange, bool);

    struct ParameterStruct {
      int m_HistogramSize;
      bool m_UseCtRange;
    };

  private:
    double m_Range;
    int m_HistogramSize;
    bool m_UseCtRange;
  };
}
#endif //mitkGIFFirstOrderStatistics_h
