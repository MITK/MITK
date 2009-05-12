/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef  MITK_REPORTGENERATOR_H_HEADER_INCLUDED

#define  MITK_REPORTGENERATOR_H_HEADER_INCLUDED

#include <map>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk {

/** 
 * This class provides some basic functionality to generate reports based on text-format
 * templates and keyword substitution
 */  
class MITK_CORE_EXPORT ReportGenerator : public itk::Object
{
public:
  typedef std::map<std::string,std::string> KeyValueMapType;
  
  mitkClassMacro(ReportGenerator, itk::Object);
  itkNewMacro(Self);
  itkGetMacro(OutputFileName,std::string);
  /** 
   * The filename to write the generated report to
   */
  itkSetMacro(OutputFileName,std::string);
  itkGetMacro(TemplateFileName,std::string);
  /** 
   * The filename of the report template that shold be used 
   */
   itkSetMacro(TemplateFileName,std::string);
  KeyValueMapType& GetKeyValueMap() { return m_KeyValueMap; }
  void AddKey(std::string key, std::string value);
  void Generate();
protected:
  ReportGenerator() {}
  
  std::string m_OutputFileName;
  std::string m_TemplateFileName;
  KeyValueMapType m_KeyValueMap;
};

} //namespace mitk

#endif /*  MITK_REPORTGENERATOR_H_HEADER_INCLUDED */
