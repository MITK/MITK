#ifndef mitkGIFGrayLevelRunLength_h
#define mitkGIFGrayLevelRunLength_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFGrayLevelRunLength : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFGrayLevelRunLength,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGrayLevelRunLength();

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
#endif //mitkGIFGrayLevelRunLength_h
