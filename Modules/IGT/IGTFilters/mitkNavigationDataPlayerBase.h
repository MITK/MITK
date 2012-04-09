/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataPlayerBase_H_HEADER_INCLUDED_
#define MITKNavigationDataPlayerBase_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>
#include "tinyxml.h"


namespace mitk{

  /**Documentation
  * \brief This class is a slightly changed reimplementation of the
  * NavigationDataPlayer which does not care about timestamps and just
  * outputs the navigationdatas in their sequential order
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataPlayerBase
    : public NavigationDataSource
  {
  public:
    mitkClassMacro(NavigationDataPlayerBase, NavigationDataSource);

    /**
    * \brief Used for pipeline update just to tell the pipeline that we always have to update
    */
    virtual void UpdateOutputInformation();

    /** @return Returns an error message if there was one (e.g. if the stream is invalid). 
     *          Returns an empty string if there was no error in the current stream.
     */
    itkGetStringMacro(ErrorMessage);

    /** @return Retruns if the current stream is valid or not. */
    itkGetMacro(StreamValid,bool);

   /**
    * \brief This method checks if player arrived at end of file.
    * 
    *\warning This method is not tested yet. It is not save to use!
    */
    bool IsAtEnd();

  protected:
    NavigationDataPlayerBase();
    virtual ~NavigationDataPlayerBase();
    virtual void GenerateData() = 0;


    /**
    * \brief Creates NavigationData from XML element and returns it 
    */
    mitk::NavigationData::Pointer ReadNavigationData(TiXmlElement* elem);

    bool m_StreamValid;                       ///< stores if the input stream is valid or not
    std::string m_ErrorMessage;               ///< stores the error message if the stream is invalid
  
  };
} // namespace mitk

#endif /* MITKNavigationDataSequentialPlayer_H_HEADER_INCLUDED_ */
