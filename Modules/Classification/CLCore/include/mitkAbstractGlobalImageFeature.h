/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef mitkAbstractGlobalImageFeature_h
#define mitkAbstractGlobalImageFeature_h

#include <MitkCLCoreExports.h>

#include <mitkBaseData.h>
#include <mitkImage.h>

#include <mitkCommandLineParser.h>

#include <mitkIntensityQuantifier.h>

// STD Includes

// Eigen
#include <Eigen/Dense>

// MITK includes
#include <mitkConfigurationHolder.h>

namespace mitk
{
  /**
  *
  *
  * ## Histogram Configuration ##
  * Most Feature Generation Classes that use histograms use the same parameters and
  * initialization logic. In general, all information can be passed either by the corresponding
  * Setter (which does not differenciate between global setting and feature specific setting) and
  * a parameter object which can be obtained from the command line arguments, for example.
  *
  * If the image values are used for the initializiation of the histogram, it can be defined
  * whether the whole image is used or only the masked areas to find minima and maxima. This is
  * done by the option <b>SetIgnoreMask</b> or the corrsponding options
  * <b>-NAME::ignore-mask-for-histogram</b> and <b>-ignore-mask-for-histogram</b>. If these are
  * true, the whole image is used for the calculation.
  *
  * Depending on the passed arguments, different initialization methods are used. The initialization
  * is in the following order:
  * - If <b>Minimum Intensity</b>, <b>Maximum Intensity</b>, and <b>Binsize</b>: The histogram is
  * initialized between the minimum and maximum intensity. the number of bins is determined by the
  * binsize. If the distance between minimum and maximum is not a multiple of the binsize, the maximum
  * is increase so that it is.
  * - <b>Minimum Intensity</b>, <b>Bins</b>, and <b>Binsize</b>: The histogram is initialized with the
  * given binsize, and the intensity range from the minimum to \f$maximum = minimum + binsize*bins\f$.
  * - <b>Minimum Intensity</b>, <b>Maximum Intensity</b>, and <b>Bins</b>: The histogram is initialized
  * between the given minimum and maximum intensity. The binsize is calculated so that the number
  * of bins is equal to the given number of bins.
  * - <b>Binsize</b>, and <b>Minimum Intensity</b>: The maximum  is set to the  maximum that
  * occur in the given image. Depending if the mask is considered or not, either only masked voxels or
  * the whole image is used for the calculation. The initialization is then equal as if the minimum
  * and maximum would have been given right from the beginning.
  * - <b>Binsize</b>, and <b>Maximum Intensity</b>: The minimum intensity is set to the minimum that
  * occur in the given image. Depending if the mask is considered or not, either only masked voxels or
  * the whole image is used for the calculation. The initialization is then equal as if the minimum
  * and maximum would have been given right from the beginning.
  * - <b>Binsize</b>: The maximum and the minimum intensity is set to the minimum and maximum that
  * occur in the given image. Depending if the mask is considered or not, either only masked voxels or
  * the whole image is used for the calculation. The initialization is then equal as if the minimum
  * and maximum would have been given right from the beginning.
  * - <b>Bins</b>, and <b>Minimum Intensity</b>: The maximum is calculated from the image. Depending
  * if the mask is considered or not, either only masked voxels or the whole image is used for the calculation. The histogram is
  * then initialized as if these values would have been given as minimum and maximum intensity.
  * - <b>Bins</b>, and <b>Maximum Intensity</b>: The minimum is calculated from the image. Depending
  * if the mask is considered or not, either only masked voxels or the whole image is used for the calculation. The histogram is
  * then initialized as if these values would have been given as minimum and maximum intensity.
  * - <b>Bins</b>: The minimum and the maximum is calculated from the image. Depending
  * if the mask is considered or not, either only masked voxels or * the whole image is used for the calculation. The histogram is
  * then initialized as if these values would have been given as minimum and maximum intensity.
  * - <b>No Parameter given</b>:The minimum and maximum intensity from the whole image or masked image is calculated and
  * the histogram then initialized to this with a standard number of bins (Is set by each filter on its own.)
  *
  * ### Remark about command line parameter####
  * There are generally two options to set a parameter via the command line. A global one that works for
  * all filters that use histograms and a local one that set this parameter specific for this filter. The
  * local parameters start with the filter name (Indiciated by NAME) followed by two colons, for example
  * <b>vol::min</b> to set the minimum intensity for the volume filter. The global parameter is overwritten
  * by the local parameter, if it is specified. Otherwise, it is still valid. If this prevents the specification
  * of an histogram initialization method (for example, because the binsize is globally specified but the histogram
  * should be initialized using a fixed numbe of bins), the parameter <b>NAME::ignore-global-histogram</b> can be passed.
  * Then, all global histogram parameters are ignored and only local ones are used.
  *
  * The maximum intensity can be set by different command line parameters: global for all filters that use histograms
  * by <b>-minimum-intensity</b> and <b>-minimum</b>. Alternative it can be set only for this filter by
  * <b>-NAME::minimum</b> and <b>-NAME::min</b>.
  *
  * The minimum intensity can be set by different command line parameters: global for all filters that use histograms
  * by <b>-maximum-intensity</b> and <b>-maximum</b>. Alternative it can be set only for this filter by
  * <b>\NAME::maximum</b> and <b>NAME::max</b>.
  *
  * The binsize can be set by different command line parameters: global for all filters that use histograms
  * by <b>-binsize</b>. Alternative it can be set only for this filter by
  * <b>\NAME::binsize</b>.
  *
  * The number of bins can be set by different command line parameters: global for all filters that use histograms
  * by <b>-bins</b>. Alternative it can be set only for this filter by
  * <b>\NAME::bins</b>.


  * ### Note to the developers ###
  * All features are supposed to work the same way if a histogram is used somewhere in
  * the code. For this, each derived class that makes use of a histogram should use
  * the Quantifier object. In order to use this object correctly, the AddArguments-Function should
  * contain the line <b>AddQuantifierArguments(parser);</b>, the CalculateFeaturesUsingParameters function
  * should contain the line <b>InitializeQuantifierFromParameters(feature, mask);</b> and the CalculateFeatures function
  * sould contain the line <b>InitializeQuantifier(image, mask);</b>. These function
  * calls ensure that the necessary options are given to the configuration file, and that the initialization
  * of the quantifier is done correctly. This ensures an consistend behavior over all FeatureGeneration Classes.
  *
  */
class MITKCLCORE_EXPORT AbstractGlobalImageFeature : public BaseData
{
public:
  mitkClassMacro(AbstractGlobalImageFeature, BaseData)

  typedef std::vector< std::pair<std::string, double> > FeatureListType;
  typedef std::vector< std::string>                     FeatureNameListType;
  typedef std::map<std::string, us::Any>                ParameterTypes;

  /**
  * \brief Calculates the feature of this abstact interface. Does not necessarily considers the parameter settings.
  */
  virtual FeatureListType CalculateFeatures(const Image::Pointer & feature, const Image::Pointer &mask) = 0;

  /**
  * \brief Calculates the given feature Slice-wise. Might not be availble for an individual filter!
  */
  FeatureListType CalculateFeaturesSlicewise(const Image::Pointer & feature, const Image::Pointer &mask, int sliceID);

  /**
  * \brief Calculates the feature of this abstact interface. Does not necessarily considers the parameter settings.
  */
  virtual void CalculateFeaturesSliceWiseUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, int sliceID, FeatureListType &featureList);

  /**
  * \brief Calculates the feature of this abstact interface. Does not necessarily considers the parameter settings.
  */
  virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList) = 0;

  /**
  * \brief Returns a list of the names of all features that are calculated from this class
  */
  virtual FeatureNameListType GetFeatureNames() = 0;

  /**
  * \brief Adds an additional Separator to the name of the feature, which encodes the used parameters
  */
  virtual std::string GetCurrentFeatureEncoding();

  /**
  * \brief Returns a string that encodes the feature class name.
  *
  * This Feature returns a string that should be put before every other feature
  * value. It has the format [<Prefix>::]<Feature Class Name>::[<Parameter Encoding>::].
  * The options <Prefix> and <Parameter Encoding> are only added, if the corresponding
  * options are set.
  */
  std::string FeatureDescriptionPrefix();

  itkSetMacro(Prefix, std::string);
  itkSetMacro(ShortName, std::string);
  itkSetMacro(LongName, std::string);
  itkSetMacro(FeatureClassName, std::string);
  itkSetMacro(Direction, int);
  void SetParameter(ParameterTypes param) { m_Parameter=param; };

  itkGetConstMacro(Prefix, std::string);
  itkGetConstMacro(ShortName, std::string);
  itkGetConstMacro(LongName, std::string);
  itkGetConstMacro(FeatureClassName, std::string);
  itkGetConstMacro(Parameter, ParameterTypes);

  itkSetMacro(UseQuantifier, bool);
  itkGetConstMacro(UseQuantifier, bool);
  itkSetMacro(Quantifier, IntensityQuantifier::Pointer);
  itkGetMacro(Quantifier, IntensityQuantifier::Pointer);

  itkGetConstMacro(Direction, int);

  itkSetMacro(MinimumIntensity, double);
  itkSetMacro(UseMinimumIntensity, bool);
  itkSetMacro(MaximumIntensity, double);
  itkSetMacro(UseMaximumIntensity, bool);
  itkGetConstMacro(MinimumIntensity, double);
  itkGetConstMacro(UseMinimumIntensity, bool);
  itkGetConstMacro(MaximumIntensity, double);
  itkGetConstMacro(UseMaximumIntensity, bool);


  itkSetMacro(Binsize, double);
  itkSetMacro(UseBinsize, bool);
  itkGetConstMacro(Binsize, double);
  itkGetConstMacro(UseBinsize, bool);

  itkSetMacro(MorphMask, mitk::Image::Pointer);
  itkGetConstMacro(MorphMask, mitk::Image::Pointer);

  itkSetMacro(Bins, int);
  itkSetMacro(UseBins, bool);
  itkGetConstMacro(UseBins, bool);
  itkGetConstMacro(Bins, int);

  itkSetMacro(IgnoreMask, bool);
  itkGetConstMacro(IgnoreMask, bool);

  itkSetMacro(EncodeParameters, bool);
  itkGetConstMacro(EncodeParameters, bool);

  std::string GetOptionPrefix() const
  {
    if (m_Prefix.length() > 0)
      return m_Prefix + "::" + m_ShortName;
    return m_ShortName;

  }

  virtual void AddArguments(mitkCommandLineParser &parser) = 0;
  std::vector<double> SplitDouble(std::string str, char delimiter);

  void AddQuantifierArguments(mitkCommandLineParser &parser);
  void InitializeQuantifierFromParameters(const Image::Pointer & feature, const Image::Pointer &mask,unsigned int defaultBins = 256);
  void InitializeQuantifier(const Image::Pointer & feature, const Image::Pointer &mask, unsigned int defaultBins = 256);
  std::string QuantifierParameterString();

public:

//#ifndef DOXYGEN_SKIP

  void SetRequestedRegionToLargestPossibleRegion() override {};
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  bool VerifyRequestedRegion() override { return false; };
  void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  bool IsEmpty() const override
  {
    if(IsInitialized() == false)
      return true;
    const TimeGeometry* timeGeometry = const_cast<AbstractGlobalImageFeature*>(this)->GetUpdatedTimeGeometry();
    if(timeGeometry == nullptr)
      return true;
    return false;
  }


private:
  std::string m_Prefix; // Prefix before all input parameters
  std::string m_ShortName; // Name of all variables
  std::string m_LongName; // Long version of the name (For turning on)
  std::string m_FeatureClassName;
  ParameterTypes m_Parameter; // Parameter setting

  bool m_UseQuantifier = false;
  IntensityQuantifier::Pointer m_Quantifier;

  double m_MinimumIntensity = 0;
  bool m_UseMinimumIntensity = false;
  double m_MaximumIntensity = 100;
  bool m_UseMaximumIntensity = false;
  bool m_EncodeParameters = false;

  double m_Binsize = 1;
  bool m_UseBinsize = false;

  int m_Bins = 256;
  bool m_UseBins = true;
  int m_Direction = 0;

  bool m_IgnoreMask = false;
  bool m_CalculateWithParameter = false;

  mitk::Image::Pointer m_MorphMask = nullptr;
//#endif // Skip Doxygen

};
}

#endif //mitkAbstractGlobalImageFeature_h
