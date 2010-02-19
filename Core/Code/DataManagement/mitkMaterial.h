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

#ifndef _MITK_MATERIAL_H_
#define _MITK_MATERIAL_H_

#include "mitkCommon.h"
#include <itkRGBPixel.h>
#include <itkObject.h>
#include <itkVectorContainer.h>
#include <vtkSystemIncludes.h>
#include <string>

namespace mitk
{

/**
 * Encapsulates 3D visualization properties which are forwarded to vtk for
 * color mapping. This includes color, specular coefficient and power, opacity
 * interpolation type (flat, gouraud, phong) and representation (points,
 * wireframe or surface).
 *
 * @see vtkProperty
*/
class MITK_CORE_EXPORT Material : public itk::Object
{
public:
    mitkClassMacro( Material, itk::Object );

    typedef itk::RGBPixel<vtkFloatingPointType> Color;

    enum InterpolationType
    {
        Flat, Gouraud, Phong
    };

    enum RepresentationType
    {
        Points, Wireframe, Surface
    };

    /**
     * Constructor. Materials are set to the following default values:
     * Color (0.5, 0.5, 0.5) color coefficient 1.0, specular color (1.0, 1.0, 1.0),
     * specular coefficient 1.0, specular power 10, opacity 1.0, interpolation
     * Gouraud, representation Surface.
     * @param node optinally a data tree node may be defined to which the properties
     *             are forwarded. Please note, that if this node doesn't have the
     *             needed properties associated, they will be added.
     */
    static Pointer New()
    {
      Pointer smartPtr = new Material(  );
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * Constructor. All values besides the given ones are set to defaults as
     * described in the default constructor
     * @param color the material color in RGB. Each RGB value should be in the
     *              range [0..1]
     * @param opacity the opacity of the material. 0.0 means fully transparent
     *              and 1.0 means solid.
     * @param node optinally a data tree node may be defined to which the properties
     *              are forwarded. Please note, that if this node doesn't have the
     *              needed properties associated, they will be added.
     */
    static Pointer New( Color color, vtkFloatingPointType opacity = 1.0f)
    {
      Pointer smartPtr = new Material(color, opacity );
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * Constructor. All values besides the given ones are set to defaults as
     * described in the default constructor
     * @param red the red component of the materials color (range [0..1])
     * @param green the green component of the materials color (range [0..1])
     * @param blue the blue component of the materials color (range [0..1])
     * @param opacity the opacity of the material. 0.0 means fully transparent
     *        and 1.0 means solid.
     * @param node optionally a data tree node may be defined to which the properties
     *        are forwarded. Please note, that if this node doesn't have the
     *        needed properties associated, they will be added.
     */
    static Pointer New( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity = 1.0f)
    {
      Pointer smartPtr = new Material(red, green, blue, opacity );
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * Constructor. All values besides the given ones are set to defaults as
     * described in the default constructor
     * @param red the red component of the materials color (range [0..1])
     * @param green the green component of the materials color (range [0..1])
     * @param blue the blue component of the materials color (range [0..1])
     * @param colorCoefficient a scaling factor for the color coefficient which
     *        will be multiplied with each color component (range [0..1]).
     * @param specularCoefficient controls in combination with the specular power
     *        how shiny the material will appear (range [0..1]).
     * @param specularPower controls in combination with the specular coefficient
     *        how shiny the material will appear (range [0..inf]).
     * @param opacity the opacity of the material. 0.0 means fully transparent
     *        and 1.0 means solid.
     * @param node optionally a data tree node may be defined to which the properties
     *        are forwarded. Please note, that if this node doesn't have the
     *        needed properties associated, they will be added.
     */
    static Pointer New( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType colorCoefficient,
        vtkFloatingPointType specularCoefficient, vtkFloatingPointType specularPower, vtkFloatingPointType opacity )
    {
      Pointer smartPtr = new Material(red, green, blue, colorCoefficient, specularCoefficient, specularPower, opacity );
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * Constructor. All values besides the given ones are set to defaults as
     * described in the default constructor
     *
     * @param color the material color in RGB. Each RGB value should be in the
     *        range [0..1]
     * @param colorCoefficient a scaling factor for the color coefficient which
     *        will be multiplied with each color component (range [0..1]).
     * @param specularCoefficient controls in combination with the specular power
     *        how shiny the material will appear (range [0..1]).
     * @param specularPower controls in combination with the specular coefficient
     *        how shiny the material will appear (range [0..inf]).
     * @param opacity the opacity of the material. 0.0 means fully transparent
     *        and 1.0 means solid.
     * @param node optionally a data tree node may be defined to which the properties
     *        are forwarded. Please note, that if this node doesn't have the
     *        needed properties associated, they will be added.
     */
    static Pointer New( Color color, vtkFloatingPointType colorCoefficient, vtkFloatingPointType specularCoefficient, vtkFloatingPointType specularPower, vtkFloatingPointType opacity )
    {
      Pointer smartPtr = new Material(color, colorCoefficient, specularCoefficient, specularPower, opacity );
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * Copy constructor
     */
    mitkNewMacro1Param(Material, const Material&);

    /**
     * Copy constructor, provided for convinience. The values are copied from property
     * and afterwards the values provided for red green blue and opacity are written into the object.
     */
    static Pointer New( const Material& property, vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity = 1.0, std::string name = "" )
    {
      Pointer smartPtr = new Material(property, red, green, blue, opacity, name );
      smartPtr->UnRegister();
      return smartPtr;
    }

    virtual bool Assignable(const Material& other) const;
    virtual Material& operator=(const Material& other);

     /* Sets the materials color in RGB space. The rgb components have to be
     * in the range [0..1]
     * @param color the new color of the material
     */
    virtual void SetColor( Color color );

    /**
     * Sets the materials color in RGB space. The rgb components have to be
     * in the range [0..1]
     * @param red the red component of the materials color (range [0..1])
     * @param green the green component of the materials color (range [0..1])
     * @param blue the blue component of the materials color (range [0..1])
     */
    virtual void SetColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue );

    /**
     * Sets a attenuation coefficient for the color. A value of 0 results in
     * a black object. VAlid range is [0..1]
     * @param coefficient the color attenuation coefficient
     */
    virtual void SetColorCoefficient( vtkFloatingPointType coefficient );

    /**
     * Sets the specular color
     * @param color the specular color in RGB. Each RGB value should be in the
     *        range [0..1]
     */
    virtual void SetSpecularColor( Color color );

    /**
     * Sets the specular color
     * @param red the red component of the specular color (range [0..1])
     * @param green the green component of the specular color (range [0..1])
     * @param blue the blue component of the specular color (range [0..1])
     */
    virtual void SetSpecularColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue );

    /**
     * Sets the specular coefficient which controls the shininess of the object
     * together with the specular power
     * @param specularCoefficient the new specular coefficient. Valid range
     *        is [0..1]
     */
    virtual void SetSpecularCoefficient( vtkFloatingPointType specularCoefficient );

    /**
     * Sets the specular power which controls the shininess of the object
     * together with the specular coefficient
     * @param specularCoefficient the new specular coefficient. Valid range
     *        is [0..inf]
     */
    virtual void SetSpecularPower( vtkFloatingPointType specularPower );

    /**
     * Sets the opacity of the material, which controls how transparent the
     * object appears. Valid range is [0..1], where 0 means fully transparent
     * and 1 means a solid surface.
     * @param opacity the new opacity of the material
     */
    virtual void SetOpacity( vtkFloatingPointType opacity );

    /**
     * Sets the surface interpolation method of the object rendered using the
     * given materials. Valid Interopation types are Flat, Gouraud and Phong.
     * See any computer graphics book for their meaning
     * @param interpolation the interpolation method used for rendering of
     *        surfaces.
     */
    virtual void SetInterpolation( InterpolationType interpolation );

    /**
     * Sets the surface representation method of the object rendered using the
     * given materials. Valid Interopation types are Points, Wireframe and
     * Surface.
     * @param representation the representation method used for rendering of
     *        surfaces.
     */
    virtual void SetRepresentation( RepresentationType representation );

    /**
     * Set/Get the width of a Line. The width is expressed in screen units. The default is 1.0.
     */
    virtual void SetLineWidth( float lineWidth );

    /**
     * @returns the color of the material
     */
    virtual Color GetColor() const;

    /**
     * @returns the color coefficient of the material. Range is [0..1]
     */
    virtual vtkFloatingPointType GetColorCoefficient() const;

    /**
     * @returns the specular color of the material in rgb values, which
     * range from 0 .. 1
     */
    virtual Color GetSpecularColor() const;

    /**
     * @returns the specular coefficient used for rendering. Range is [0..1]
     */
    virtual vtkFloatingPointType GetSpecularCoefficient() const;

    /**
     * @returns the specular power. Ranges from 0 to infinity
     */
    virtual vtkFloatingPointType GetSpecularPower() const;

    /**
     * @returns the opacity of the material. Ranges from 0 to 1
     */
    virtual vtkFloatingPointType GetOpacity() const;

    /**
     * @returns the interpolation method used for rendering.
     */
    virtual InterpolationType GetInterpolation() const;

    /**
     * @returns the representation type used for rendering.
     */
    virtual RepresentationType GetRepresentation() const;

    /**
     * @returns the interpolation method used for rendering using the predefined
     * vtk constants.
     */
    virtual int GetVtkInterpolation() const;

    /**
     * @returns the representation type used for rendering using the predefined
     * vtk constants.
     */
    virtual int GetVtkRepresentation() const;

    /**
     * @returns the line width used for wireframe rendering as a fraction of screen units
     */
    virtual float GetLineWidth() const;

    /**
     * Fills the current materials with the properties of the
     * given material.
     * @param property the materials which should be copied in the
     *        current materials
     * @param copyDataTreeNode If set to true, the data tree node and renderer
     *        associated with the material property are also copied. Otherwise
     *        these member variables will be left untouched
     */
    virtual void Initialize( const Material& property );

    /**
     * comparison operator which uses the member variables for
     * comparison
     */
    virtual bool operator==( const Material& property ) const;

    /**
     * Dumps the properties to the out stream out
     */
    void PrintSelf ( std::ostream &os ) const;

    /**
     * Sets an optional name which may be associated with the material property
     * Please note, that this name is NOT forwarded to the data tree node
     * as the node name
     */
    itkSetMacro( Name, std::string );

    /**
     * returns the name associated with the material property
     */
    itkGetConstMacro( Name, std::string );

protected:

  /**
   * Constructor. Materials are set to the following default values:
   * Color (0.5, 0.5, 0.5) color coefficient 1.0, specular color (1.0, 1.0, 1.0),
   * specular coefficient 1.0, specular power 10, opacity 1.0, interpolation
   * Gouraud, representation Surface.
   * @param node optinally a data tree node may be defined to which the properties
   *             are forwarded. Please note, that if this node doesn't have the
   *             needed properties associated, they will be added.
   */
  Material(  );

  /**
   * Constructor. All values besides the given ones are set to defaults as
   * described in the default constructor
   * @param color the material color in RGB. Each RGB value should be in the
   *              range [0..1]
   * @param opacity the opacity of the material. 0.0 means fully transparent
   *              and 1.0 means solid.
   * @param node optinally a data tree node may be defined to which the properties
   *              are forwarded. Please note, that if this node doesn't have the
   *              needed properties associated, they will be added.
   */
  Material( Color color, vtkFloatingPointType opacity = 1.0f );

  /**
   * Constructor. All values besides the given ones are set to defaults as
   * described in the default constructor
   * @param red the red component of the materials color (range [0..1])
   * @param green the green component of the materials color (range [0..1])
   * @param blue the blue component of the materials color (range [0..1])
   * @param opacity the opacity of the material. 0.0 means fully transparent
   *        and 1.0 means solid.
   * @param node optionally a data tree node may be defined to which the properties
   *        are forwarded. Please note, that if this node doesn't have the
   *        needed properties associated, they will be added.
   */
  Material( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity = 1.0f );

  /**
   * Constructor. All values besides the given ones are set to defaults as
   * described in the default constructor
   * @param red the red component of the materials color (range [0..1])
   * @param green the green component of the materials color (range [0..1])
   * @param blue the blue component of the materials color (range [0..1])
   * @param colorCoefficient a scaling factor for the color coefficient which
   *        will be multiplied with each color component (range [0..1]).
   * @param specularCoefficient controls in combination with the specular power
   *        how shiny the material will appear (range [0..1]).
   * @param specularPower controls in combination with the specular coefficient
   *        how shiny the material will appear (range [0..inf]).
   * @param opacity the opacity of the material. 0.0 means fully transparent
   *        and 1.0 means solid.
   * @param node optionally a data tree node may be defined to which the properties
   *        are forwarded. Please note, that if this node doesn't have the
   *        needed properties associated, they will be added.
   */
  Material( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType colorCoefficient,
    vtkFloatingPointType specularCoefficient, vtkFloatingPointType specularPower, vtkFloatingPointType opacity );

  /**
   * Constructor. All values besides the given ones are set to defaults as
   * described in the default constructor
   *
   * @param color the material color in RGB. Each RGB value should be in the
   *        range [0..1]
   * @param colorCoefficient a scaling factor for the color coefficient which
   *        will be multiplied with each color component (range [0..1]).
   * @param specularCoefficient controls in combination with the specular power
   *        how shiny the material will appear (range [0..1]).
   * @param specularPower controls in combination with the specular coefficient
   *        how shiny the material will appear (range [0..inf]).
   * @param opacity the opacity of the material. 0.0 means fully transparent
   *        and 1.0 means solid.
   * @param node optionally a data tree node may be defined to which the properties
   *        are forwarded. Please note, that if this node doesn't have the
   *        needed properties associated, they will be added.
   */
  Material( Color color, vtkFloatingPointType colorCoefficient, vtkFloatingPointType specularCoefficient, vtkFloatingPointType specularPower, vtkFloatingPointType opacity );

  /**
   * Copy constructor
   */
  Material( const Material& property );

  /**
   * Copy constructor, provided for convinience. The values are copied from property
   * and afterwards the values provided for red green blue and opacity are written into the object.
   */
  Material( const Material& property, vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue, vtkFloatingPointType opacity = 1.0, std::string name = "");


    virtual void InitializeStandardValues();

    virtual void Update();

    std::string m_Name;

    Color m_Color;

    Color m_SpecularColor;

    vtkFloatingPointType m_ColorCoefficient;

    vtkFloatingPointType m_SpecularCoefficient;

    vtkFloatingPointType m_SpecularPower;

    vtkFloatingPointType m_Opacity;

    float m_LineWidth;

    InterpolationType m_Interpolation;

    RepresentationType m_Representation;
};

typedef itk::VectorContainer< unsigned int, Material::Pointer > MaterialVectorContainer;

}


#endif
