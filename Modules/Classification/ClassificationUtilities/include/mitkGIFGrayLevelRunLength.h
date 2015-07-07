#ifndef mitkGIFGrayLevelRunLength_h
#define mitkGIFGrayLevelRunLength_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkClassificationUtilitiesExports.h>

namespace mitk
{

  class MITKCLASSIFICATIONUTILITIES_EXPORT GIFGrayLevelRunLength : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFGrayLevelRunLength,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGrayLevelRunLength();

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
#endif //mitkGIFGrayLevelRunLength_h