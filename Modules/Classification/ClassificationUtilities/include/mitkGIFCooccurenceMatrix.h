#ifndef mitkGIFCooccurenceMatrix_h
#define mitkGIFCooccurenceMatrix_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkClassificationUtilitiesExports.h>

namespace mitk
{

  class MITKCLASSIFICATIONUTILITIES_EXPORT GIFCooccurenceMatrix : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFCooccurenceMatrix,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
      * \brief Calculates the Cooccurence-Matrix based features for this class.
      */
      virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

      /**
      * \brief Returns a list of the names of all features that are calculated from this class
      */
      virtual FeatureNameListType GetFeatureNames();
  };

}
#endif //mitkGIFCooccurenceMatrix_h