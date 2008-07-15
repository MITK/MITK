/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKRIGIDREGISTRATIONPRESET_H_HEADER
#define MITKRIGIDREGISTRATIONPRESET_H_HEADER

#include <vtkXMLParser.h>
#include <mitkXMLWriter.h>
#include <map>
#include <string>
#include <itkArray.h>
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT RigidRegistrationPreset : public vtkXMLParser
  {
  public:
    RigidRegistrationPreset();
    ~RigidRegistrationPreset();

    bool LoadPreset();
    bool LoadPreset(std::string fileName);
    itk::Array<double> getTransformValues(std::string name);
    itk::Array<double> getMetricValues(std::string name);
    itk::Array<double> getOptimizerValues(std::string name);
    itk::Array<double> getInterpolatorValues(std::string name);
    std::map<std::string, itk::Array<double> >& getTransformValuesPresets();
    std::map<std::string, itk::Array<double> >& getMetricValuesPresets();
    std::map<std::string, itk::Array<double> >& getOptimizerValuesPresets();
    std::map<std::string, itk::Array<double> >& getInterpolatorValuesPresets();
    void newPresets(std::map<std::string, itk::Array<double> > newTransformValues, std::map<std::string, itk::Array<double> > newMetricValues,
                                            std::map<std::string, itk::Array<double> > newOptimizerValues, std::map<std::string, itk::Array<double> > newInterpolatorValues);//, std::map<std::string, itk::Array<float> newMetricValues, std::map<std::string, itk::Array<float>> newOptimizerValues, std::map<std::string, itk::Array<float>> newInterpolatorValues);
    

    private:

      //##Documentation
      //## @brief method used in XLM-Reading; gets called when a start-tag is read
      void StartElement (const char *elementName, const char **atts);
    
      void saveXML(mitk::XMLWriter& xmlWriter);
      void save();
      void saveTransformValues(mitk::XMLWriter& xmlWriter, std::string item);
      void saveMetricValues(mitk::XMLWriter& xmlWriter, std::string item);
      void saveOptimizerValues(mitk::XMLWriter& xmlWriter, std::string item);
      void saveInterpolatorValues(mitk::XMLWriter& xmlWriter, std::string item);

      itk::Array<double> loadTransformValues(itk::Array<double> transformValues, double transform, const char **atts);
      itk::Array<double> loadMetricValues(itk::Array<double> metricValues, double metric, const char **atts);
      itk::Array<double> loadOptimizerValues(itk::Array<double> optimizerValues, double optimizer, const char **atts);
      itk::Array<double> loadInterpolatorValues(itk::Array<double> interpolatorValues/*, double interpolator, const char **atts*/);

      //##Documentation
      //## @brief reads an XML-String-Attribute
      std::string ReadXMLStringAttribut( std::string name, const char** atts);

      std::string m_Name;
      std::map<std::string, itk::Array<double> > m_TransformValues;
      std::map<std::string, itk::Array<double> > m_MetricValues;
      std::map<std::string, itk::Array<double> > m_OptimizerValues;
      std::map<std::string, itk::Array<double> > m_InterpolatorValues;

      std::string m_XmlFileName;
  };
}
#endif
