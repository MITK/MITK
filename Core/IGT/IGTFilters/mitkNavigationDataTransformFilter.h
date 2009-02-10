/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include <vtkTransform.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataTransformFilter applies an user-defined transformation on navigation data objects
  *
  * @ingroup Navigation
  */
  class NavigationDataTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataTransformFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    void SetTransform(vtkTransform* transform)
    { 
      m_Transform = transform; 
      this->Modified();
    }
    
    vtkTransform* GetTransform()
    {return m_Transform;}
   

  protected:
    NavigationDataTransformFilter();
    virtual ~NavigationDataTransformFilter();

    /**Documentation
    * \brief filter execute method
    *
    * transforms data   
    */
    virtual void GenerateData();

    vtkTransform* m_Transform; ///< transform which will be applied on navigation data(s)
  };
} // namespace mitk

#endif /* MITKNavigationDataTransformFilter_H_HEADER_INCLUDED_ */
