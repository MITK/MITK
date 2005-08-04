/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED
#define MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED

#include "mitkBaseProperty.h"
#include "mitkTransferFunction.h"
#include <mitkXMLWriter.h>
#include <mitkXMLReader.h>


namespace mitk {

  class TransferFunctionProperty : public BaseProperty
  {
    public:
      mitkClassMacro(TransferFunctionProperty, BaseProperty);

      /**
       *
       */
      TransferFunctionProperty()
        : BaseProperty()
      {};

      /**
       *
       */
      TransferFunctionProperty( mitk::TransferFunction::Pointer value )
        : BaseProperty(), m_Value( value )
      {};

      itkSetMacro(Value, mitk::TransferFunction::Pointer );
      itkGetConstMacro(Value, mitk::TransferFunction::Pointer );

      /**
       *
       */
      virtual bool operator==(const BaseProperty& property) const {

        const Self *other = dynamic_cast<const Self*>(&property);

        if(other==NULL) 
          return false;
        else
        return (other->m_Value==m_Value);
      }

      /**
       *
       */
      std::string GetValueAsString() const {
        std::stringstream myStr;
        myStr << GetValue() ;
        return myStr.str(); 
      }

      /**
       *
       */
      virtual bool WriteXMLData( XMLWriter& xmlWriter )
      {
        xmlWriter.WriteProperty( XMLReader::MIN, m_Value->GetMin() );
        xmlWriter.WriteProperty( XMLReader::MAX, m_Value->GetMax() );
        xmlWriter.WriteProperty( XMLReader::VALID, m_Value->GetValid() );    
        return true;
      }

      /**
       *
       */
      virtual bool ReadXMLData( XMLReader& xmlReader )
      {
        int min, max;
        bool valid;

        xmlReader.GetAttribute( XMLReader::MIN, min );
        xmlReader.GetAttribute( XMLReader::MAX, max );
        xmlReader.GetAttribute( XMLReader::VALID, valid );

        m_Value = new mitk::TransferFunction( min, max );
        return true;
      }

      protected:
        mitk::TransferFunction::Pointer m_Value;  
  };


  //  typedef GenericProperty<mitk::TransferFunction::Pointer> TransferFunctionProperty;
} // namespace mitk  

#endif /* MITKTRANFERFUNCTIONPROPERTY_H_HEADER_INCLUDED */ 
