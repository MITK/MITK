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


#ifndef MITKNavigationDataReaderXML_H_HEADER_INCLUDED_
#define MITKNavigationDataReaderXML_H_HEADER_INCLUDED_

#include <mitkAbstractFileReader.h>
#include <mitkNavigationDataSet.h>
// includes for exceptions
#include <mitkIGTException.h>
#include <mitkIGTIOException.h>

class TiXmlElement;
class TiXmlNode;

namespace mitk {

  class NavigationDataReaderXML : public AbstractFileReader
  {
  public:
    NavigationDataReaderXML();
    virtual ~NavigationDataReaderXML();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData>> Read() override;

  protected:

    NavigationDataReaderXML(const NavigationDataReaderXML& other);
    virtual mitk::NavigationDataReaderXML* Clone() const override;

    NavigationDataSet::Pointer ReadNavigationDataSet();

    /**
     * \brief This method reads one line of the XML document and returns the data as a NavigationData object
     * If there is a new file version another method must be added which reads this data.
     * @throw mitk::IGTException Throws an exceptions if file is damaged.
     */
    mitk::NavigationData::Pointer ReadVersion1();
    mitk::NavigationData::Pointer ReadNavigationData(TiXmlElement* elem);

    std::string m_FileName;

    TiXmlElement* m_parentElement;
    TiXmlNode* m_currentNode;

    int m_FileVersion; ///< indicates which XML encoding is used
    int m_NumberOfOutputs; ///< stores the number of outputs known from the XML document

    // -- deprecated | begin
    //std::istream* m_Stream; ///< stores a pointer to the input stream
    bool m_StreamEnd; ///< stores if the input stream arrived at end
    bool m_StreamValid;                       ///< stores if the input stream is valid or not
    std::string m_ErrorMessage;               ///< stores the error message if the stream is invalid

    /**
     * \brief Creates a stream out of the filename given by the variable m_FileName.
     * The stream is then set to m_Stream.
     *
     * @throw mitk::IGTIOException Throws an exception if file does not exist
     * @throw mitk::IGTException Throws an exception if the stream is NULL
     */
    //void CreateStreamFromFilename();

    /**
     * \brief Returns the file version out of the XML document.
     * @throw mitk::IGTException Throws an mitk::IGTException an exception if stream is NULL or not good.
     * @throw mitk::IGTIOException Throws an mitk::IGTIOException if the stream has an incompatible XML format.
     */
    unsigned int GetFileVersion(std::istream* stream);

    /**
     * \brief Returns the number of tracked tools out of the XML document.
     * @throw Throws an exception if stream is NULL.
     * @throw Throws an exception if the input stream has an XML incompatible format.
     */
    unsigned int GetNumberOfNavigationDatas(std::istream* stream);

    /**
     * @brief This is a helping method which gives an error message and throws an exception with the given message.
     *        It can be used if a stream is found to be invalid.
     *
     * @throw mitk::IGTIOException Always throws an exception.
     */
    void StreamInvalid(std::string message);  ///< help method which sets the stream invalid and displays an error
    // -- deprecated | end
  private:
    NavigationDataSet::Pointer Read(std::istream* stream);
    NavigationDataSet::Pointer Read(std::string fileName);
  };

} // namespace mitk

#endif // MITKNavigationDataReaderXML_H_HEADER_INCLUDED_
