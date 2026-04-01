/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkAbstractGlobalImageFeature_h
#define mitkAbstractGlobalImageFeature_h

#include <MitkCLCoreExports.h>

#include <mitkBaseData.h>
#include <mitkImage.h>

#include <mitkCommandLineParser.h>

#include <mitkIntensityQuantifier.h>

// STD Includes

// Eigen
#include <itkeigen/Eigen/Dense>

// MITK includes
#include <mitkConfigurationHolder.h>

namespace mitk
{

  /**
   * \brief Unique identifier for a computed image feature.
   *
   * FeatureID combines the feature name, its owning feature class, a settings identifier,
   * optional parameters, a legacy name (for backward compatibility), and a version string.
   * It is used as a key in feature result lists.
   *
   * \sa AbstractGlobalImageFeature
   */
  struct MITKCLCORE_EXPORT FeatureID
  {
    std::string name;         ///< Name of the individual feature.
    std::string featureClass; ///< Name of the feature class that computed this feature.
    std::string settingID;    ///< Identifier encoding the parameter configuration; must be unique per parameter set.
    std::string legacyName;   ///< Legacy feature name that encodes parameter settings directly in the string.
    std::string version = "1"; ///< Version of the feature definition.

    using ParametersType = std::map<std::string, us::Any>;
    ParametersType parameters; ///< Map of parameter name-value pairs used for computation.

    /**
     * \brief Less-than comparison for use in ordered containers.
     * \param[in] rh The right-hand FeatureID to compare against.
     * \return True if this FeatureID sorts before rh.
     */
    bool operator < (const FeatureID& rh) const;

    /**
     * \brief Equality comparison.
     * \param[in] rh The right-hand FeatureID to compare against.
     * \return True if both FeatureIDs are equal.
     */
    bool operator ==(const FeatureID& rh) const;
  };

  /**
   * \brief Create a FeatureID by cloning a template and setting its name.
   *
   * \param[in] templateID Reference FeatureID to clone.
   * \param[in] name Name to assign to the new FeatureID.
   * \return A new FeatureID with the given name and all other fields from templateID.
   */
  MITKCLCORE_EXPORT FeatureID CreateFeatureID(FeatureID templateID, std::string name);

  /**
  *
  *
  * ## Histogram Configuration ##
  * Most Feature Generation Classes that use histograms use the same parameters and
  * initialization logic. In general, all information can be passed either by the corresponding
  * Setter (which does not differentiate between global setting and feature specific setting) and
  * a parameter object which can be obtained from the command line arguments, for example.
  *
  * If the image values are used for the initialization of the histogram, it can be defined
  * whether the whole image is used or only the masked areas to find minima and maxima. This is
  * done by the option <b>SetIgnoreMask</b> or the corresponding options
  * <b>-%NAME::ignore-mask-for-histogram</b> and <b>-ignore-mask-for-histogram</b>. If these are
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
  * local parameters start with the filter name (Indicated by NAME) followed by two colons, for example
  * <b>vol::min</b> to set the minimum intensity for the volume filter. The global parameter is overwritten
  * by the local parameter, if it is specified. Otherwise, it is still valid. If this prevents the specification
  * of an histogram initialization method (for example, because the binsize is globally specified but the histogram
  * should be initialized using a fixed number of bins), the parameter <b>NAME::ignore-global-histogram</b> can be passed.
  * Then, all global histogram parameters are ignored and only local ones are used.
  *
  * The maximum intensity can be set by different command line parameters: global for all filters that use histograms
  * by <b>-minimum-intensity</b> and <b>-minimum</b>. Alternative it can be set only for this filter by
  * <b>-%NAME::minimum</b> and <b>-%NAME::min</b>.
  *
  * The minimum intensity can be set by different command line parameters: global for all filters that use histograms
  * by <b>-maximum-intensity</b> and <b>-maximum</b>. Alternative it can be set only for this filter by
  * <b>-%NAME::maximum</b> and <b>-%NAME::max</b>.
  *
  * The binsize can be set by different command line parameters: global for all filters that use histograms
  * by <b>-binsize</b>. Alternative it can be set only for this filter by
  * <b>-%NAME::binsize</b>.
  *
  * The number of bins can be set by different command line parameters: global for all filters that use histograms
  * by <b>-bins</b>. Alternative it can be set only for this filter by
  * <b>-%NAME::bins</b>.


  * ### Note to the developers ###
  * All features are supposed to work the same way if a histogram is used somewhere in
  * the code. For this, each derived class that makes use of a histogram should use
  * the Quantifier object. In order to use this object correctly, the AddArguments-Function should
  * contain the line <b>AddQuantifierArguments(parser);</b>, the CalculateFeaturesUsingParameters function
  * should contain the line <b>InitializeQuantifierFromParameters(feature, mask);</b> and the CalculateFeatures function
  * should contain the line <b>InitializeQuantifier(image, mask);</b>. These function
  * calls ensure that the necessary options are given to the configuration file, and that the initialization
  * of the quantifier is done correctly. This ensures a consistent behavior over all FeatureGeneration Classes.
  *
  */
class MITKCLCORE_EXPORT AbstractGlobalImageFeature : public BaseData
{
public:
  mitkClassMacro(AbstractGlobalImageFeature, BaseData);

  typedef std::vector< std::pair<FeatureID, double> > FeatureListType;
  using ParametersType = FeatureID::ParametersType;

  /**
  * \brief Calculates the feature of this abstract interface. Does not necessarily considers the parameter settings.
  */
  FeatureListType CalculateFeatures(const Image* image, const Image* mask);
  virtual FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) = 0;

  /**
  * \brief Calculates the given feature Slice-wise. Might not be available for an individual filter!
  */
  FeatureListType CalculateFeaturesSlicewise(const Image::Pointer & image, const Image::Pointer &mask, int sliceID);

  /**
  * \brief Calculates the feature of this abstract interface. Does not necessarily considers the parameter settings.
  */
  virtual void CalculateAndAppendFeaturesSliceWise(const Image::Pointer & image, const Image::Pointer &mask, int sliceID, FeatureListType &featureList, bool checkParameterActivation = true);

  /**
  * \brief Calculates the feature of this abstract interface. Does not necessarily considers the parameter settings.
  * @param image
  * @param mask
  * @param maskNoNaN
  * @param featureList
  * @param checkParameterActivation Indicates if the features should only be calculated and added if the FeatureClass is activated in the parameters.
  * True: only append if activated in the parameters. False: always and append it.
  */
  void CalculateAndAppendFeatures(const Image* image, const Image* mask, const Image* maskNoNaN, FeatureListType &featureList, bool checkParameterActivation = true);

  /** \brief Set the prefix prepended to command line option names. */
  itkSetMacro(Prefix, std::string);
  /** \brief Set the short name used in option keys and feature names. */
  itkSetMacro(ShortName, std::string);
  /** \brief Set the long (human-readable) name of this feature class. */
  itkSetMacro(LongName, std::string);
  /** \brief Set the feature class name used in FeatureID. */
  itkSetMacro(FeatureClassName, std::string);
  /** \brief Set the direction index for directional feature computation. */
  itkSetMacro(Direction, int);

  /**
   * \brief Set the parameter map and reconfigure the instance accordingly.
   *
   * Calls ConfigureQuantifierSettingsByParameters() and ConfigureSettingsByParameters()
   * to update histogram and feature-specific settings from the provided parameters.
   *
   * \param[in] param Map of parameter name-value pairs.
   */
  void SetParameters(ParametersType param)
  {
    m_Parameters = param;
    this->ConfigureQuantifierSettingsByParameters();
    this->ConfigureSettingsByParameters(param);
    this->Modified();
  };

  /** \brief Get the command line option prefix. */
  itkGetConstMacro(Prefix, std::string);
  /** \brief Get the short name of this feature class. */
  itkGetConstMacro(ShortName, std::string);
  /** \brief Get the long name of this feature class. */
  itkGetConstMacro(LongName, std::string);
  /** \brief Get the feature class name. */
  itkGetConstMacro(FeatureClassName, std::string);
  /** \brief Get the current parameter map. */
  itkGetConstMacro(Parameters, ParametersType);

  /** \brief Get the IntensityQuantifier used for histogram-based feature computation. */
  itkGetMacro(Quantifier, IntensityQuantifier::Pointer);

  /** \brief Get the direction index. */
  itkGetConstMacro(Direction, int);

  /** \brief Set the minimum intensity for histogram initialization. */
  itkSetMacro(MinimumIntensity, double);
  /** \brief Set whether to use the explicit minimum intensity. */
  itkSetMacro(UseMinimumIntensity, bool);
  /** \brief Set the maximum intensity for histogram initialization. */
  itkSetMacro(MaximumIntensity, double);
  /** \brief Set whether to use the explicit maximum intensity. */
  itkSetMacro(UseMaximumIntensity, bool);
  /** \brief Get the minimum intensity for histogram initialization. */
  itkGetConstMacro(MinimumIntensity, double);
  /** \brief Get whether the explicit minimum intensity is used. */
  itkGetConstMacro(UseMinimumIntensity, bool);
  /** \brief Get the maximum intensity for histogram initialization. */
  itkGetConstMacro(MaximumIntensity, double);
  /** \brief Get whether the explicit maximum intensity is used. */
  itkGetConstMacro(UseMaximumIntensity, bool);

  /** \brief Set the histogram bin size. */
  itkSetMacro(Binsize, double);
  /** \brief Set whether to use the explicit bin size for histogram initialization. */
  itkSetMacro(UseBinsize, bool);
  /** \brief Get the histogram bin size. */
  itkGetConstMacro(Binsize, double);
  /** \brief Get whether the explicit bin size is used. */
  itkGetConstMacro(UseBinsize, bool);

  /** \brief Set the morphological mask image used by some feature classes. */
  itkSetMacro(MorphMask, mitk::Image::Pointer);
  /** \brief Get the morphological mask image. */
  itkGetConstMacro(MorphMask, mitk::Image::Pointer);

  /** \brief Set the number of histogram bins. */
  itkSetMacro(Bins, int);
  /** \brief Set whether to use the explicit bin count for histogram initialization. */
  itkSetMacro(UseBins, bool);
  /** \brief Get whether the explicit bin count is used. */
  itkGetConstMacro(UseBins, bool);
  /** \brief Get the number of histogram bins. */
  itkGetConstMacro(Bins, int);

  /** \brief Set whether to ignore the mask when computing histogram ranges. */
  itkSetMacro(IgnoreMask, bool);
  /** \brief Get whether the mask is ignored for histogram range computation. */
  itkGetConstMacro(IgnoreMask, bool);

  /** \brief Set whether to encode parameter values in the feature name prefix. */
  itkSetMacro(EncodeParametersInFeaturePrefix, bool);
  /** \brief Get whether parameter values are encoded in the feature name prefix. */
  itkGetConstMacro(EncodeParametersInFeaturePrefix, bool);
  /** \brief Toggle encoding of parameter values in the feature name prefix. */
  itkBooleanMacro(EncodeParametersInFeaturePrefix);

  /**
   * \brief Build the full option prefix string for command line parameters.
   *
   * If a prefix is set, returns "prefix::shortName"; otherwise returns the short name alone.
   *
   * \return The option prefix string.
   */
  std::string GetOptionPrefix() const
  {
    if (!m_Prefix.empty())
      return m_Prefix + "::" + m_ShortName;
    return m_ShortName;
  }

  /** Can be called to add all relevant argument for configuring the feature instance to the passed parser instance.
  Must be implemented be derived classes. For adding the quantifier arguments use AddQuantifierArguments(...) as
  helper function.*/
  virtual void AddArguments(mitkCommandLineParser &parser) const = 0;

  /** Helper function that generates the legacy feature name without encoding of parameters; as it is used e.g.
   in the unit tests.*/
  static std::string GenerateLegacyFeatureNameWOEncoding(const FeatureID& id);

protected:
  std::vector<double> SplitDouble(std::string str, char delimiter);

  virtual FeatureListType DoCalculateFeatures(const Image* image, const Image* mask) = 0;

  void AddQuantifierArguments(mitkCommandLineParser& parser) const;

  /** Ensures that all quantifier relevant variables of the instance are set correctly given the information in m_Parameters.*/
  void ConfigureQuantifierSettingsByParameters();

  /** Ensures that the instance is configured according to the information given in the passed parameters.
  * This method will be called by SetParameters(...) after ConfigureQuantifierSettingsByParameters() was called.*/
  virtual void ConfigureSettingsByParameters(const ParametersType& parameters);

  /**Initializes the quantifier gigen the quantifier relevant variables and the passed arguments.*/
  void InitializeQuantifier(const Image* image, const Image* mask, unsigned int defaultBins = 256);

  /** Helper that encodes the quantifier parameters in a string (e.g. used for the legacy feature name)*/
  std::string QuantifierParameterString() const;

  /* Creates a template feature id.
   * it will set the featureClass, the settingID (assuming that it is the featureClass with the passed suffix
   * and all parameters that are global or have the option prefix of the instance.*/
  FeatureID CreateTemplateFeatureID(std::string settingsSuffix = "", FeatureID::ParametersType additionalParams = {});

  /** Helper that generates the legacy feature names for a passed FeatureID.
  * Format of the legacy feature name is: \<ClassName\>::[\<LegacyFeatureEncoding\>::]\<LegacyFeatureNamePart\>
  * Overwrite GenerateLegacyFeatureNamePart and GenerateLegacyFeatureEncoding to change behavior in
  * derived classes.
  */
  virtual std::string GenerateLegacyFeatureName(const FeatureID& id) const;
  virtual std::string GenerateLegacyFeatureNamePart(const FeatureID& id) const;
  virtual std::string GenerateLegacyFeatureEncoding(const FeatureID& id) const;

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
  ParametersType m_Parameters; // Parameter setting

  mitk::Image::Pointer m_MorphMask = nullptr;


  IntensityQuantifier::Pointer m_Quantifier;
  //Quantifier relevant variables
  double m_MinimumIntensity = 0;
  bool m_UseMinimumIntensity = false;
  double m_MaximumIntensity = 100;
  bool m_UseMaximumIntensity = false;
  bool m_EncodeParametersInFeaturePrefix = false;

  double m_Binsize = 1;
  bool m_UseBinsize = false;

  int m_Bins = 256;
  bool m_UseBins = true;
  int m_Direction = 0;

  bool m_IgnoreMask = false;
//#endif // Skip Doxygen

};
}

#endif
