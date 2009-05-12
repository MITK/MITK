/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-04-02 15:46:56 +0200 (Do, 02 Apr 2009) $
Version:   $Revision: 16783 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNAVIGATIONDATATOMESSAGEFILTER_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATATOMESSAGEFILTER_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include "mitkMessage.h"

namespace mitk 
{
  /**Documentation
  * \brief NavigationDataToMessageilter emits multiple mitk::Message messages when the input NavigationData values change
  *
  * This filter has only one input. It copies the input navigation data values to its output and emits 
  * the following Messages if the input navigation data values changed since the last Update()
  * - PositionChangedMessage
  * - OrientationChangedMessage
  * - ErrorChangedMessage
  * - TimeStampChangedMessage
  * - DataValidChangedMessage
  *
  * Warning: adding more inputs with SetInput(idx, const mitk::NavigationData* nd) will raise an 
  * std::invalid_argument exception for idx > 0.
  *
  * \ingroup IGT
  */
  class MITK_IGT_EXPORT NavigationDataToMessageFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataToMessageFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);
    mitkNewMessage1Macro(PositionChanged, mitk::NavigationData::PositionType);       ///< Sends the message PositionChangedMessage whenever the position of the input changes
    mitkNewMessage1Macro(OrientationChanged, mitk::NavigationData::OrientationType); ///< Sends the message OrientationChangedMessage whenever the orientation of the input changes
    mitkNewMessage1Macro(ErrorChanged, mitk::NavigationData::CovarianceMatrixType);  ///< Sends the message ErrorChangedMessage whenever the error covariance matrix of the input changes
    mitkNewMessage1Macro(TimeStampChanged, mitk::NavigationData::TimeStampType);     ///< Sends the message TimeStampChangedMessage whenever the timestamp of the input changes
    mitkNewMessage1Macro(DataValidChanged, bool);                                    ///< Sends the message DataValidChangedMessage whenever the DataValid flag of the input changes

    /**Documentation
    * \brief sets the nth input of the filter. Warning: this filter only has input #0!
    *
    * WARNING: NavigationDataToMessageFilter manages only one input. Calling this method 
    * with an idx parameter other than 0 will raise an std::invalid_argument exception!
    */    
    virtual void SetInput(unsigned int idx, const NavigationData* nd);
    
    /**Documentation
    * \brief Sets the input of this filter
    *
    * Sets the input navigation data object for this filter. 
    */
    virtual void SetInput(const NavigationData* nd);
   
  protected:
    NavigationDataToMessageFilter();
    virtual ~NavigationDataToMessageFilter();

    /**Documentation
    * \brief filter execute method
    *
    * emits the Messages
    */
    virtual void GenerateData();
  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATATOMESSAGEFILTER_H_HEADER_INCLUDED_ */
