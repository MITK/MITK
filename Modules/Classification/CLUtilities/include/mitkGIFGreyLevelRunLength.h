#ifndef mitkGIFGreyLevelRunLength_h
#define mitkGIFGreyLevelRunLength_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFGreyLevelRunLength : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFGreyLevelRunLength,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGreyLevelRunLength();

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

    itkGetConstMacro(UseCtRange, bool);
    itkSetMacro(UseCtRange, bool);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);


    struct ParameterStruct
    {
      bool  m_UseCtRange;
      double m_Range;
      unsigned int m_Direction;

      double MinimumIntensity;
      bool UseMinimumIntensity;
      double MaximumIntensity;
      bool UseMaximumIntensity;
      int Bins;
    };

  private:
    double m_Range;
    bool m_UseCtRange;
  };
}
#endif //mitkGIFGreyLevelRunLength_h
