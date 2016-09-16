#ifndef mitkGIFCooccurenceMatrix2_h
#define mitkGIFCooccurenceMatrix2_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <eigen3\Eigen\src\Core\Array.h>

namespace mitk
{
  struct CoocurenceMatrixHolder
  {
  public:
    CoocurenceMatrixHolder(double min, double max, int number);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    Eigen::MatrixXd m_Matrix;

  };

  struct CoocurenceMatrixFeatures
  {
  public:
    double JointMaximum;
    double JointAverage;
  };


  class MITKCLUTILITIES_EXPORT GIFCooccurenceMatrix2 : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFCooccurenceMatrix2,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFCooccurenceMatrix2();

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

    struct GIFCooccurenceMatrix2Configuration
    {
      double range;
      unsigned int direction;
    };

    private:
    double m_Range;
    unsigned int m_Direction;
  };

}
#endif //mitkGIFCooccurenceMatrix2_h
