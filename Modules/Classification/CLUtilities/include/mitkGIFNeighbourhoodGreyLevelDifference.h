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
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames() override;

    itkGetConstMacro(Range,double);
    itkSetMacro(Range, double);

    itkGetConstMacro(UseCtRange, bool);
    itkSetMacro(UseCtRange, bool);

    itkGetConstMacro(Direction, unsigned int);
    itkSetMacro(Direction, unsigned int);

    struct ParameterStruct
    {
      bool  m_UseCtRange;
      double m_Range;
      unsigned int m_Direction;
    };

  private:
    double m_Range;
    bool m_UseCtRange;
    unsigned int m_Direction;
  };
}
#endif //mitkGIFNeighbourhoodGreyLevelDifference_h
