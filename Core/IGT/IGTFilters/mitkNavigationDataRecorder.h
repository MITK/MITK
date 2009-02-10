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


#ifndef _MITK_NavigationDataRecorder_H
#define _MITK_NavigationDataRecorder_H

#include <itkProcessObject.h>
#include "mitkNavigationData.h"
#include "tinyxml.h"

namespace mitk
{
/**
 * @brief Superclass of all classes generating point sets (instances of class
 * mitk::PointSet) as output. 
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. 
 * @ingroup Process
 */
class NavigationDataRecorder : public itk::ProcessObject
{
public:
    mitkClassMacro( NavigationDataRecorder, itk::ProcessObject );

    itkNewMacro( Self );

    itkSetStringMacro(FileName);

    virtual void AddNavigationData(const NavigationData* nd);

protected:

    /**Documentation
    * \brief filter execute method
    *
    */
    virtual void GenerateData();

    virtual void SaveToDisc();

    NavigationDataRecorder();
    
    virtual ~NavigationDataRecorder();

    std::string m_FileName;

    unsigned int m_NumberOfInputs;

    std::vector< TiXmlDocument > m_XMLFiles;

};

}
#endif // #define _MITK_POINT_SET_SOURCE_H

