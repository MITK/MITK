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

#ifndef MITKUSNAVIGATIONEXPERIMENTLOGGING_H
#define MITKUSNAVIGATIONEXPERIMENTLOGGING_H

#include <mitkCommon.h>
#include <itkObjectFactory.h>
#include <Poco/DOM/AutoPtr.h>
#include <ostream>

namespace itk {
template<class T> class SmartPointer;
class RealTimeClock;
}

namespace mitk {
class DataNode;
}

namespace Poco {
namespace XML {
class Document;
class Element;
}
}

namespace mitk {
/**
 * \brief Logs data node properties to an XML file.
 */
class USNavigationExperimentLogging : public itk::Object {
public:
  mitkClassMacroItkParent(USNavigationExperimentLogging, itk::Object)
  itkNewMacro(USNavigationExperimentLogging)

  /**
   * \brief Clears the XML tree.
   * Can be used to start logging a new experiment. Make sure that the output
   * file name was changed by calling SetFileName() before setting a new result
   * by calling SetResult() if you do not want your previous results to be
   * overwritten.
   *
   * The real time clock is restarted, too.
   */
  void Reset();

  /**
   * \brief Setter for the file path and name of the XML file.
   */
  void SetFileName(const std::string& fileName);

  /**
   * \brief The properties of the given node are appended to the XML tree.
   * The data node gets its own XML node under the root of the XML tree and the
   * XML node will get the name of the data node as its name. The properties of
   * the data node are stored as nodes under this XML node. The current tree is
   * written to the output file on every call of this method.
   *
   * If a key prefix is set by SetKeyPrefix, only properties which keys that are
   * starting with this prefix are stored in the XML tree. The prefix is cut from
   * the resulting key for the XML tree.
   */
  void SetResult(const itk::SmartPointer<mitk::DataNode> resultNode);

  /**
   * \brief Only properties with this prefix are written into the output file.
   */
  itkSetMacro(KeyPrefix, std::string)
  itkGetMacro(KeyPrefix, std::string)

protected:
  /**
   * \brief Constructs an empty XML tree and starts the real time clock.
   */
  USNavigationExperimentLogging();
  virtual ~USNavigationExperimentLogging();

  void AddCurrentTimeAttributes(Poco::AutoPtr<Poco::XML::Element>);

  /**
   * \brief Does the actual writing of the complete XML tree to the file system.
   */
  void WriteXmlToFile();

  std::string                           m_KeyPrefix;

  std::string                           m_FileName;
  unsigned int                          m_CurrentResultNumber;

  itk::SmartPointer<itk::RealTimeClock> m_RealTimeClock;
  double                                m_StartTime;

  Poco::AutoPtr<Poco::XML::Document>    m_OutputXML;
  Poco::AutoPtr<Poco::XML::Element>     m_OutputXMLRoot;
};
} // namespace mitk

#endif // MITKUSNAVIGATIONEXPERIMENTLOGGING_H
