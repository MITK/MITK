/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 9841 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef mitkPicHeaderPropertyhincluded
#define mitkPicHeaderPropertyhincluded

#include "mitkCommon.h"
#include "mitkBaseProperty.h"

#include "ipPic.h"

namespace mitk 
{

class PicHeaderProperty : public BaseProperty
{
  public:

    PicHeaderProperty();
    mitkClassMacro(PicHeaderProperty, BaseProperty);
    
    PicHeaderProperty(ipPicTSV_t*);
    
    virtual ~PicHeaderProperty();
     
    virtual bool Assignable(const BaseProperty& other) const;
    virtual BaseProperty& operator=(const BaseProperty& other);
   
    virtual bool operator==(const BaseProperty& property) const;
    
    ipPicTSV_t* GetValue() const;
    std::string GetValueAsString() const;

  protected:

    ipPicTSV_t* m_StoredTag;
};

} // namespace mitk

#endif 

