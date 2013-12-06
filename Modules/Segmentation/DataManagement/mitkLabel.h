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


#ifndef __mitkLabel_H_
#define __mitkLabel_H_

#include "SegmentationExports.h"
#include <mitkColorProperty.h>
#include <mitkVector.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk
{

//##
//##Documentation
//## @brief A data structure describing a label.
//## @ingroup Data
//##
class Segmentation_EXPORT Label : public itk::Object
{
public:

    mitkClassMacro( Label, itk::Object );

    itkNewMacro( Self );

    itkSetMacro(Locked, bool);
    itkGetConstMacro(Locked, bool);
    itkBooleanMacro(Locked);

    itkSetMacro(Visible, bool);
    itkGetConstMacro(Visible, bool);
    itkBooleanMacro(Visible);

    itkSetMacro(Exterior, bool);
    itkGetConstMacro(Exterior, bool);
    itkBooleanMacro(Exterior);

    itkSetMacro(Filled, bool);
    itkGetConstMacro(Filled, bool);
    itkBooleanMacro(Filled);

    itkSetMacro(Selected, bool );
    itkGetConstMacro( Selected, bool );
    itkBooleanMacro(Selected);

    itkSetMacro(Opacity, float);
    itkGetConstMacro(Opacity, float);

    itkSetMacro(Volume, float);
    itkGetConstMacro(Volume, float);

    itkSetStringMacro(Name);
    itkGetStringMacro(Name);

    itkSetMacro(LatinName, std::string);
    itkGetConstMacro(LatinName, std::string);

    itkSetMacro(LastModified, std::string);
    itkGetConstMacro(LastModified, std::string);

    void SetCenterOfMassIndex(const mitk::Point3D& center);
    const mitk::Point3D& GetCenterOfMassIndex();

    void SetCenterOfMassCoordinates(const mitk::Point3D& center);
    const mitk::Point3D& GetCenterOfMassCoordinates();

    void SetColor(const mitk::Color&);
    const mitk::Color& GetColor() const
    { return m_Color; };

    itkSetMacro(Index, int);
    itkGetConstMacro(Index, int);

    itkSetMacro(Layer, int);
    itkGetConstMacro(Layer, int);

    Label();
    virtual ~Label();

protected:

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    Label(const Label& other);

    bool m_Locked;
    bool m_Visible;
    bool m_Filled;
    bool m_Exterior;
    float m_Opacity;
    bool m_Selected;
    std::string m_Name;
    std::string m_LatinName;
    float m_Volume;
    std::string m_LastModified;
    int m_Index;
    int m_Layer;
    mitk::Color m_Color;
    mitk::Point3D m_CenterOfMassIndex;
    mitk::Point3D m_CenterOfMassCoordinates;
};

} // namespace mitk

#endif // __mitkLabel_H_
