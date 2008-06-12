/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED
#define MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkBaseProperty.h"
#include "mitkVector.h"

#include <itkConfigure.h>

#include <string>


namespace mitk {

/**
 * @brief Property for annotations
 * @ingroup DataTree
 */
class MITK_CORE_EXPORT AnnotationProperty : public BaseProperty
{
public:
  mitkClassMacro(AnnotationProperty, BaseProperty);
  typedef std::string ValueType;
  
  itkNewMacro( AnnotationProperty );
  mitkNewMacro2Param( AnnotationProperty, 
    const char *, const Point3D & );
  mitkNewMacro2Param( AnnotationProperty, 
    const std::string &, const Point3D & );
  mitkNewMacro4Param( AnnotationProperty,
    const char *, ScalarType, ScalarType, ScalarType );
  mitkNewMacro4Param( AnnotationProperty,
    const std::string &, ScalarType, ScalarType, ScalarType );
  
  itkGetStringMacro( Label );
  itkSetStringMacro( Label );

  /// @todo: remove after ITK 2.8 compatibility has been dropped 
#if ITK_VERSION_MAJOR < 3     
  virtual void SetLabel(const std::string & _arg ) 
  { 
    this->SetValue( _arg.c_str() );
  } 
#endif

  const Point3D &GetPosition() const;
  void SetPosition( const Point3D &position );

  //##ModelId=3E3FF04F00E1
  virtual bool operator==(const BaseProperty& property ) const;
  virtual std::string GetValueAsString() const;

  //## 
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  //##
  virtual bool ReadXMLData( XMLReader& xmlReader );


protected:
  static const char *LABEL;
  static const char *POSITION;

  std::string m_Label;
  Point3D m_Position;
  
  AnnotationProperty();
  AnnotationProperty( const char *label, const Point3D &position );
  AnnotationProperty( const std::string &label, const Point3D &position );
  AnnotationProperty( const char *label, ScalarType x, ScalarType y, ScalarType z );
  AnnotationProperty( const std::string &label, ScalarType x, ScalarType y, ScalarType z );

};

} // namespace mitk

#endif /* MITKANNOTATIONPROPERTY_H_HEADER_INCLUDED */
