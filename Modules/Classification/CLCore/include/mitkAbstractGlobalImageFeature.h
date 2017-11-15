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
  * \brief Calculates the feature of this abstact interface. Does not necessarily considers the parameter settings.
  */
  virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList) = 0;

  /**
  * \brief Returns a list of the names of all features that are calculated from this class
  */
  virtual FeatureNameListType GetFeatureNames() = 0;

  itkSetMacro(Prefix, std::string);
  itkSetMacro(ShortName, std::string);
  itkSetMacro(LongName, std::string);
  itkSetMacro(Direction, int);
  void SetParameter(ParameterTypes param) { m_Parameter=param; };

  itkGetConstMacro(Prefix, std::string);
  itkGetConstMacro(ShortName, std::string);
  itkGetConstMacro(LongName, std::string);
  itkGetConstMacro(Parameter, ParameterTypes);

  itkSetMacro(UseQuantifier, bool);
  itkGetConstMacro(UseQuantifier, bool);
  itkSetMacro(Quantifier, IntensityQuantifier::Pointer);
  itkGetMacro(Quantifier, IntensityQuantifier::Pointer);

  itkGetConstMacro(Direction, int);

  itkSetMacro(MinimumIntensity, float);
  itkSetMacro(UseMinimumIntensity, bool);
  itkSetMacro(MaximumIntensity, float);
  itkSetMacro(UseMaximumIntensity, bool);
  itkGetConstMacro(MinimumIntensity, float);
  itkGetConstMacro(UseMinimumIntensity, bool);
  itkGetConstMacro(MaximumIntensity, float);
  itkGetConstMacro(UseMaximumIntensity, bool);

  itkSetMacro(MorphMask, mitk::Image::Pointer);
  itkGetConstMacro(MorphMask, mitk::Image::Pointer);

  itkSetMacro(Bins, int);
  itkGetConstMacro(Bins, int);

  std::string GetOptionPrefix() const
  {
    if (m_Prefix.length() > 0)
      return m_Prefix + "::" + m_ShortName;
    return m_ShortName;

  }

  virtual void AddArguments(mitkCommandLineParser &parser) = 0;
  std::vector<double> SplitDouble(std::string str, char delimiter);

  void AddQuantifierArguments(mitkCommandLineParser &parser);
  void InitializeQuantifier(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList, unsigned int defaultBins = 256);

public:

//#ifndef DOXYGEN_SKIP

  virtual void SetRequestedRegionToLargestPossibleRegion() override {};
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  virtual bool VerifyRequestedRegion() override { return false; };
  virtual void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  virtual bool IsEmpty() const override
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
  ParameterTypes m_Parameter; // Parameter setting

  bool m_UseQuantifier = false;
  IntensityQuantifier::Pointer m_Quantifier;

  double m_MinimumIntensity = 0;
  bool m_UseMinimumIntensity = false;
  double m_MaximumIntensity = 100;
  bool m_UseMaximumIntensity = false;


  int m_Bins = 256;
  int m_Direction = 0;

  mitk::Image::Pointer m_MorphMask = nullptr;
//#endif // Skip Doxygen

};
}

#endif //mitkAbstractGlobalImageFeature_h
