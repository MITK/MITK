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


#ifndef MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_
#define MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_

#include <mitkNavigationDataPlayerBase.h>
#include "tinyxml.h"


namespace mitk
{

  /**Documentation
  * \brief This class is a slightly changed reimplementation of the
  * NavigationDataPlayer which does not care about timestamps and just
  * outputs the navigationdatas in their sequential order
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataSequentialPlayer
    : public NavigationDataPlayerBase
  {
  public:
    mitkClassMacro(NavigationDataSequentialPlayer, NavigationDataPlayerBase);
    itkNewMacro(Self);

    /**
     * \brief sets the file name and path (if XMLString is set, this is neglected)
     * @throw mitk::IGTIOException Throws an exception if the given file cannot be loaded.
     */
    void SetFileName(const std::string& _FileName);

    /**
    * \brief returns the file name and path
    */
    itkGetStringMacro(FileName);

    /**
    * \brief sets a xml string (by this, the xml string is not read from file)
    * @throw mitk::IGTExcepton Throws an mitk::IGTExcepton if the string to set is not an XMLString
    */
    void SetXMLString(const std::string& _XMLString);

    /**
    * \brief returns the current xml string
    */
    itkGetStringMacro(XMLString);

    /**
     * @brief Set to true if the data player should repeat the outputs.
     */
    itkSetMacro(Repeat, bool);

    /**
     * @return Returns if the data player should repeat the outputs.
     */
    itkGetMacro(Repeat, bool);

    /**
     * @return Returns the number of navigation data snapshots available in the file
     */
    itkGetMacro(NumberOfSnapshots, unsigned int);

    /**
    * advance the output to the i-th snapshot
    * e.g. if you want to have the NavData of snapshot
    * 17 then you can call GoToSnapshot(17). index begins at 1!
    * you can then also go back to snapshot 1 with GoToSnapshot(1)
    *
    * @throw mitk::IGTException Throws an exception if cannot go back to particular snapshot.
    */
    void GoToSnapshot(int i);

    /**
    * \brief Used for pipeline update just to tell the pipeline
    * that we always have to update
    */
    virtual void UpdateOutputInformation();

  protected:
    NavigationDataSequentialPlayer();
    virtual ~NavigationDataSequentialPlayer();

    /**
     * @throw mitk::IGTException Throws an exception if data element is not found.
     */
    void ReinitXML();

    mitk::NavigationData::Pointer ReadVersion1();

    /**
     * @throw mitk::IGTException Throws an exception if cannot parse input file
     */
    virtual void GenerateData();

    std::string m_FileName;
    std::string m_XMLString;
    TiXmlDocument* m_Doc;
    TiXmlElement* m_DataElem;
    TiXmlElement* m_CurrentElem;
    bool m_Repeat;
    unsigned int m_NumberOfSnapshots;
    int m_LastGoTo;
  };
} // namespace mitk

#endif /* MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_ */
