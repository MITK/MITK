#ifndef mitkGIFFirstOrderStatistics_h
#define mitkGIFFirstOrderStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkClassificationUtilitiesExports.h>

namespace mitk
{
  class MITKCLASSIFICATIONUTILITIES_EXPORT GIFFirstOrderStatistics : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFFirstOrderStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFFirstOrderStatistics();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames();

    itkGetConstMacro(Range,double);
    itkSetMacro(Range, double);

  private:
    double m_Range;
  };
}
#endif //mitkGIFFirstOrderStatistics_h