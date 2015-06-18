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

#ifndef MITKRIGIDREGISTRATIONPRESET_H_HEADER
#define MITKRIGIDREGISTRATIONPRESET_H_HEADER

#include <vtkXMLParser.h>
#include "MitkRigidRegistrationExports.h"

#include <map>
#include <string>
#include <list>

#include <itkArray.h>
#include "mitkCommon.h"

namespace mitk {

  /**
   * \brief Class to load and save parameter presets for rigid registration.
   *
   * \ingroup RigidRegistration
   *
   * This class stores parameter presets for rigid registration applications. To that belong parameters for transformations, metrics,
   * optimizer and interpolators. The presets will be stored in a xml file. Existing presets can be opened with LoadPreset() and saved with
   * newPresets(...).
   *
   *
   * \author Daniel Stein
   */
  class MITKRIGIDREGISTRATION_EXPORT RigidRegistrationPreset : public vtkXMLParser
  {
  public:
    RigidRegistrationPreset();
    ~RigidRegistrationPreset();

    /**
    * \brief Tries to find mitkRigidRegistrationPresets.xml in /mitk/Config and loads all presets stored in this file.
    */
    bool LoadPreset();
    /**
    * \brief Tries to open preset xml file \e fileName and loads all presets stored in this file.
    */
    bool LoadPreset(std::string fileName);
    /**
    * \brief Returns an array including all all transform values belonging to preset \e name.
    */
    itk::Array<double> getTransformValues(std::string name);
    /**
    * \brief Returns an array including all all metric values belonging to preset \e name.
    */
    itk::Array<double> getMetricValues(std::string name);
    /**
    * \brief Returns an array including all all optimizer values belonging to preset \e name.
    */
    itk::Array<double> getOptimizerValues(std::string name);
    /**
    * \brief Returns an array including all interpolator values belonging to preset \e name.
    */
    itk::Array<double> getInterpolatorValues(std::string name);

    std::list< std::string>& getAvailablePresets();

    /**
    * \brief Returns a map with all preset names and their according transform values as an array.
    */
    std::map<std::string, itk::Array<double> >& getTransformValuesPresets();
    /**
    * \brief Returns a map with all preset names and their according metric values as an array.
    */
    std::map<std::string, itk::Array<double> >& getMetricValuesPresets();
    /**
    * \brief Returns a map with all preset names and their according optimizer values as an array.
    */
    std::map<std::string, itk::Array<double> >& getOptimizerValuesPresets();
    /**
    * \brief Returns a map with all preset names and their according interpolator values as an array.
    */
    std::map<std::string, itk::Array<double> >& getInterpolatorValuesPresets();
    /**
    * \brief Saves new presets in the previous opened xml file or in the new fileName location.
    *
    * Every old entry will be removed from the xml file and replaced by the ones stored in the parameter maps. Make sure you have the
    * old presets as well as the new ones within the maps.
    */
    bool newPresets(std::map<std::string, itk::Array<double> > newTransformValues,
                    std::map<std::string, itk::Array<double> > newMetricValues,
                    std::map<std::string, itk::Array<double> > newOptimizerValues,
                    std::map<std::string, itk::Array<double> > newInterpolatorValues,
                    std::string fileName = "");

    private:


      /**
      * \brief method used in XLM-Reading; gets called when a start-tag is read
      */
      void StartElement (const char *elementName, const char **atts) override;

      /**
      * \brief Saves all preset attributes to xml file.
      */
      //bool saveXML(mitk::XMLWriter& xmlWriter);
      /**
      * \brief Saves all preset attributes to xml file.
      */
      bool save();
      /**
      * \brief Saves the transform values for one preset to the xml file.
      */
      //void saveTransformValues(mitk::XMLWriter& xmlWriter, std::string item);
      /**
      * \brief Saves the metric values for one preset to the xml file.
      */
      //void saveMetricValues(mitk::XMLWriter& xmlWriter, std::string item);
      /**
      * \brief Saves the optimizer values for one preset to the xml file.
      */
      //void saveOptimizerValues(mitk::XMLWriter& xmlWriter, std::string item);
      /**
      * \brief Saves the interpolator values for one preset to the xml file.
      */
      //void saveInterpolatorValues(mitk::XMLWriter& xmlWriter, std::string item);

      /**
      * \brief Loads the transform values for one preset from the xml file.
      */
      itk::Array<double> loadTransformValues(itk::Array<double> transformValues, double transform, const char **atts);
      /**
      * \brief Loads the metric values for one preset from the xml file.
      */
      itk::Array<double> loadMetricValues(itk::Array<double> metricValues, double metric, const char **atts);
      /**
      * \brief Loads the optimizer values for one preset from the xml file.
      */
      itk::Array<double> loadOptimizerValues(itk::Array<double> optimizerValues, double optimizer, const char **atts);
      /**
      * \brief Loads the interpolator values for one preset from the xml file.
      */
      itk::Array<double> loadInterpolatorValues(itk::Array<double> interpolatorValues/*, double interpolator, const char **atts*/);

      /**
      * \brief Reads an XML-String-Attribute
      */
      std::string ReadXMLStringAttribut( std::string name, const char** atts);

      std::string m_Name;
      std::map<std::string, itk::Array<double> > m_TransformValues;
      std::map<std::string, itk::Array<double> > m_MetricValues;
      std::map<std::string, itk::Array<double> > m_OptimizerValues;
      std::map<std::string, itk::Array<double> > m_InterpolatorValues;
      std::list<std::string> m_LoadedPresets;

      std::string m_XmlFileName;
  };
}
#endif
