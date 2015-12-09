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
      virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

      /**
      * \brief Returns a list of the names of all features that are calculated from this class
      */
      virtual FeatureNameListType GetFeatureNames() override;

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);
    itkGetConstMacro(Direction, unsigned int);
    itkSetMacro(Direction, unsigned int);

    struct GIFCooccurenceMatrixConfiguration
    {
      double range;
      unsigned int direction;
    };

    private:
    double m_Range;
    unsigned int m_Direction;
  };

}
#endif //mitkGIFCooccurenceMatrix_h
