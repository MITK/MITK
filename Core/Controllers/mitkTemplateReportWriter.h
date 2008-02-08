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


#ifndef  MITK_TEMPLATEREPORTWRITER_H_HEADER_INCLUDED

#define  MITK_TEMPLATEREPORTWRITER_H_HEADER_INCLUDED

#include <map>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk {

  /** 
  * This class provides some basic functionality to generate reports based on text-format
  * templates and keyword substitution
  */  
  class MITK_CORE_EXPORT TemplateReportWriter : public itk::Object
  {
  public:
    typedef std::map<std::string,std::string> KeyValueMapType;

    mitkClassMacro(TemplateReportWriter, itk::Object);
    itkNewMacro(Self);
    
    /** 
    * The filename to write the generated report to
    */
    itkSetStringMacro(OutputFileName);
    itkGetStringMacro(OutputFileName);
    /** 
    * The filename of the report template that shold be used 
    */
    itkSetStringMacro(TemplateFileName);
    itkGetStringMacro(TemplateFileName);

    /** 
    * returns a reference to the key->value replacement map 
    */
    KeyValueMapType& GetKeyValueMap() { return m_KeyValueMap; }
    
    /** 
    * Adds an entry to the key->value replacement map
    */
    void AddKey(std::string key, std::string value);

    /** 
    * Searches the template file for keywords and adds them to the key->value map (without values)
    */
    void ReadKeywordsFromTemplateFile();


    /** 
    * The write the report file keywords replaced by their value (as set before with the AddKey() method)
    */   
    void Generate();
  protected:
    TemplateReportWriter();

    std::string m_OutputFileName;
    std::string m_TemplateFileName;
    KeyValueMapType m_KeyValueMap;
  };

} //namespace mitk

#endif /*  MITK_TEMPLATEREPORTWRITER_H_HEADER_INCLUDED */
