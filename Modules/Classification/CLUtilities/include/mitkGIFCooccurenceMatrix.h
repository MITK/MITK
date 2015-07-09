#ifndef mitkGIFCooccurenceMatrix_h
#define mitkGIFCooccurenceMatrix_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{

  class MITKCLUTILITIES_EXPORT GIFCooccurenceMatrix : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFCooccurenceMatrix,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFCooccurenceMatrix();

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
#endif //mitkGIFCooccurenceMatrix_h
