#ifndef mitkGIFNeighbourhoodGreyLevelDifference_h
#define mitkGIFNeighbourhoodGreyLevelDifference_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFNeighbourhoodGreyLevelDifference : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFNeighbourhoodGreyLevelDifference,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFNeighbourhoodGreyLevelDifference();

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

    itkGetConstMacro(UseCtRange, bool);
    itkSetMacro(UseCtRange, bool);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);


    struct ParameterStruct
    {
      bool  m_UseCtRange;
      double m_Range;
      unsigned int m_Direction;
    };

  private:
    double m_Range;
    bool m_UseCtRange;
  };
}
#endif //mitkGIFNeighbourhoodGreyLevelDifference_h
