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

#ifndef MITK_BASEPROPERTYOBSERVER_H_INCLUDED
#define MITK_BASEPROPERTYOBSERVER_H_INCLUDED

#include <itkEventObject.h>
#include "MitkExtExports.h"
#include "mitkCommon.h"

namespace mitk {

/**
  \brief Convenience class to observe changes of a mitk::BaseProperty.

  This class registers itself as an ITK observer to a BaseProperty and gets
  informed of changes to the property. Whenever such a change occurrs, the virtual
  method PropertyChanged() or PropertyRemoved() is called. This way, derived 
  classes can implement behaviour for more specific properties (e.g. ColorProperty) 
  without the need to reimplement the Subject-Observer handling.

*/

class BaseProperty;

class MitkExt_EXPORT PropertyObserver 
{
  public:

    PropertyObserver();
    virtual ~PropertyObserver();
    
    virtual void PropertyChanged() = 0;
    virtual void PropertyRemoved() = 0;

  protected:

    void BeginModifyProperty();
    void EndModifyProperty();

    unsigned long m_ModifiedTag;
    unsigned long m_DeleteTag;

    bool m_SelfCall;
};
  
class MitkExt_EXPORT PropertyView : public PropertyObserver
{
  public:

    PropertyView( const mitk::BaseProperty* );
    virtual ~PropertyView();

    void OnModified(const itk::EventObject& e);
    void OnDelete(const itk::EventObject& e);

  protected:

    const mitk::BaseProperty* m_Property;
};

class MitkExt_EXPORT PropertyEditor : public PropertyObserver
{
  public:

    PropertyEditor( mitk::BaseProperty* );
    virtual ~PropertyEditor();

    void OnModified(const itk::EventObject& e);
    void OnDelete(const itk::EventObject& e);

  protected:
    
    mitk::BaseProperty* m_Property;
};
  
}

#endif


