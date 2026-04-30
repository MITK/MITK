/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaterial_h
#define mitkMaterial_h

#include <MitkCoreExports.h>
#include <itkObject.h>
#include <itkRGBPixel.h>
#include <itkVectorContainer.h>
#include <mitkCommon.h>
#include <string>
#include <vtkSystemIncludes.h>

namespace mitk
{
  /**
   * \brief Encapsulates 3D visualization properties which are forwarded to VTK for color mapping.
   *
   * This includes color, specular coefficient and power, opacity,
   * interpolation type (flat, gouraud, phong) and representation (points,
   * wireframe or surface).
   *
   * \sa vtkProperty
   */
  class MITKCORE_EXPORT Material : public itk::Object
  {
  public:
    mitkClassMacroItkParent(Material, itk::Object);

    typedef itk::RGBPixel<double> Color;

    enum InterpolationType
    {
      Flat,
      Gouraud,
      Phong
    };

    enum RepresentationType
    {
      Points,
      Wireframe,
      Surface
    };

    /**
     * \brief Create a new Material with default values.
     *
     * Materials are set to the following default values:
     * Color (0.5, 0.5, 0.5), color coefficient 1.0, specular color (1.0, 1.0, 1.0),
     * specular coefficient 1.0, specular power 10, opacity 1.0, interpolation
     * Gouraud, representation Surface.
     *
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New()
    {
      Pointer smartPtr = new Material();
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * \brief Create a new Material with the given color and opacity.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] color the material color in RGB. Each RGB value should be in the
     *            range [0..1].
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New(Color color, double opacity = 1.0f)
    {
      Pointer smartPtr = new Material(color, opacity);
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * \brief Create a new Material with the given RGB color components and opacity.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New(double red, double green, double blue, double opacity = 1.0f)
    {
      Pointer smartPtr = new Material(red, green, blue, opacity);
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * \brief Create a new Material with full control over color and lighting properties.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     * \param[in] colorCoefficient a scaling factor for the color coefficient which
     *            will be multiplied with each color component (range [0..1]).
     * \param[in] specularCoefficient controls in combination with the specular power
     *            how shiny the material will appear (range [0..1]).
     * \param[in] specularPower controls in combination with the specular coefficient
     *            how shiny the material will appear (range [0..inf]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New(double red,
                       double green,
                       double blue,
                       double colorCoefficient,
                       double specularCoefficient,
                       double specularPower,
                       double opacity)
    {
      Pointer smartPtr = new Material(red, green, blue, colorCoefficient, specularCoefficient, specularPower, opacity);
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * \brief Create a new Material with the given color and lighting properties.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] color the material color in RGB. Each RGB value should be in the
     *            range [0..1].
     * \param[in] colorCoefficient a scaling factor for the color coefficient which
     *            will be multiplied with each color component (range [0..1]).
     * \param[in] specularCoefficient controls in combination with the specular power
     *            how shiny the material will appear (range [0..1]).
     * \param[in] specularPower controls in combination with the specular coefficient
     *            how shiny the material will appear (range [0..inf]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New(
      Color color, double colorCoefficient, double specularCoefficient, double specularPower, double opacity)
    {
      Pointer smartPtr = new Material(color, colorCoefficient, specularCoefficient, specularPower, opacity);
      smartPtr->UnRegister();
      return smartPtr;
    }

    /** \brief Copy constructor. */
    mitkNewMacro1Param(Material, const Material &);

    /**
     * \brief Copy constructor with color and opacity override.
     *
     * The values are copied from the given property and afterwards the values
     * provided for red, green, blue, and opacity are written into the object.
     *
     * \param[in] property the source Material to copy from.
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     * \param[in] name an optional name to associate with this material.
     * \return Smart pointer to the newly created Material.
     */
    static Pointer New(
      const Material &property, double red, double green, double blue, double opacity = 1.0, std::string name = "")
    {
      Pointer smartPtr = new Material(property, red, green, blue, opacity, name);
      smartPtr->UnRegister();
      return smartPtr;
    }

    /**
     * \brief Check whether the given material can be assigned to this one.
     *
     * \param[in] other the Material to check for assignability.
     * \return True if the other material is of compatible type.
     */
    virtual bool Assignable(const Material &other) const;

    /**
     * \brief Assignment operator. Copies all properties from the other material.
     *
     * \param[in] other the Material to copy from.
     * \return Reference to this material.
     */
    virtual Material &operator=(const Material &other);

    /**
     * \brief Set the material color in RGB space.
     *
     * The RGB components have to be in the range [0..1].
     *
     * \param[in] color the new color of the material.
     */
    virtual void SetColor(Color color);

    /**
     * \brief Set the material color in RGB space.
     *
     * The RGB components have to be in the range [0..1].
     *
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     */
    virtual void SetColor(double red, double green, double blue);

    /**
     * \brief Set an attenuation coefficient for the color.
     *
     * A value of 0 results in a black object. Valid range is [0..1].
     *
     * \param[in] coefficient the color attenuation coefficient.
     */
    virtual void SetColorCoefficient(double coefficient);

    /**
     * \brief Set the specular color.
     *
     * \param[in] color the specular color in RGB. Each RGB value should be in the
     *            range [0..1].
     */
    virtual void SetSpecularColor(Color color);

    /**
     * \brief Set the specular color using RGB components.
     *
     * \param[in] red the red component of the specular color (range [0..1]).
     * \param[in] green the green component of the specular color (range [0..1]).
     * \param[in] blue the blue component of the specular color (range [0..1]).
     */
    virtual void SetSpecularColor(double red, double green, double blue);

    /**
     * \brief Set the specular coefficient which controls the shininess of the object
     * together with the specular power.
     *
     * \param[in] specularCoefficient the new specular coefficient. Valid range
     *            is [0..1].
     */
    virtual void SetSpecularCoefficient(double specularCoefficient);

    /**
     * \brief Set the specular power which controls the shininess of the object
     * together with the specular coefficient.
     *
     * \param[in] specularPower the new specular power. Valid range
     *            is [0..inf].
     */
    virtual void SetSpecularPower(double specularPower);

    /**
     * \brief Set the opacity of the material.
     *
     * Controls how transparent the object appears. Valid range is [0..1],
     * where 0 means fully transparent and 1 means a solid surface.
     *
     * \param[in] opacity the new opacity of the material.
     */
    virtual void SetOpacity(double opacity);

    /**
     * \brief Set the surface interpolation method of the object.
     *
     * Valid interpolation types are Flat, Gouraud and Phong.
     *
     * \param[in] interpolation the interpolation method used for rendering of
     *            surfaces.
     */
    virtual void SetInterpolation(InterpolationType interpolation);

    /**
     * \brief Set the surface representation method of the object.
     *
     * Valid representation types are Points, Wireframe and Surface.
     *
     * \param[in] representation the representation method used for rendering of
     *            surfaces.
     */
    virtual void SetRepresentation(RepresentationType representation);

    /**
     * \brief Set the width of a Line.
     *
     * The width is expressed in screen units. The default is 1.0.
     *
     * \param[in] lineWidth the new line width in screen units.
     */
    virtual void SetLineWidth(float lineWidth);

    /**
     * \brief Get the color of the material.
     * \return The material color as an RGB pixel.
     */
    virtual Color GetColor() const;

    /**
     * \brief Get the color coefficient of the material.
     * \return The color coefficient in the range [0..1].
     */
    virtual double GetColorCoefficient() const;

    /**
     * \brief Get the specular color of the material.
     * \return The specular color in RGB values, each in the range [0..1].
     */
    virtual Color GetSpecularColor() const;

    /**
     * \brief Get the specular coefficient used for rendering.
     * \return The specular coefficient in the range [0..1].
     */
    virtual double GetSpecularCoefficient() const;

    /**
     * \brief Get the specular power.
     * \return The specular power, ranging from 0 to infinity.
     */
    virtual double GetSpecularPower() const;

    /**
     * \brief Get the opacity of the material.
     * \return The opacity in the range [0..1].
     */
    virtual double GetOpacity() const;

    /**
     * \brief Get the interpolation method used for rendering.
     * \return The interpolation type (Flat, Gouraud, or Phong).
     */
    virtual InterpolationType GetInterpolation() const;

    /**
     * \brief Get the representation type used for rendering.
     * \return The representation type (Points, Wireframe, or Surface).
     */
    virtual RepresentationType GetRepresentation() const;

    /**
     * \brief Get the interpolation method as a VTK constant.
     * \return The VTK interpolation constant (VTK_FLAT, VTK_GOURAUD, or VTK_PHONG).
     */
    virtual int GetVtkInterpolation() const;

    /**
     * \brief Get the representation type as a VTK constant.
     * \return The VTK representation constant (VTK_POINTS, VTK_WIREFRAME, or VTK_SURFACE).
     */
    virtual int GetVtkRepresentation() const;

    /**
     * \brief Get the line width used for wireframe rendering.
     * \return The line width in screen units.
     */
    virtual float GetLineWidth() const;

    /**
     * \brief Fill the current material with the properties of the given material.
     *
     * \param[in] property the Material whose properties should be copied
     *            into this material.
     */
    virtual void Initialize(const Material &property);

    /**
     * \brief Comparison operator which uses the member variables for comparison.
     *
     * \param[in] property the Material to compare with.
     * \return True if all properties are equal.
     */
    virtual bool operator==(const Material &property) const;

    /**
     * \brief Print the material properties to the given output stream.
     *
     * \param[in] os the output stream.
     */
    void PrintSelf(std::ostream &os, itk::Indent) const override;

    /**
     * \brief Set an optional name which may be associated with the material property.
     *
     * Please note that this name is NOT forwarded to the data tree node
     * as the node name.
     */
    itkSetMacro(Name, std::string);

    /**
     * \brief Get the name associated with the material property.
     */
    itkGetConstMacro(Name, std::string);

  protected:
    /**
     * \brief Default constructor.
     *
     * Materials are set to the following default values:
     * Color (0.5, 0.5, 0.5), color coefficient 1.0, specular color (1.0, 1.0, 1.0),
     * specular coefficient 1.0, specular power 10, opacity 1.0, interpolation
     * Gouraud, representation Surface.
     */
    Material();

    /**
     * \brief Constructor with color and opacity.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] color the material color in RGB. Each RGB value should be in the
     *            range [0..1].
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     */
    Material(Color color, double opacity = 1.0f);

    /**
     * \brief Constructor with RGB components and opacity.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     */
    Material(double red, double green, double blue, double opacity = 1.0f);

    /**
     * \brief Constructor with full control over color and lighting properties.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] red the red component of the material color (range [0..1]).
     * \param[in] green the green component of the material color (range [0..1]).
     * \param[in] blue the blue component of the material color (range [0..1]).
     * \param[in] colorCoefficient a scaling factor for the color coefficient which
     *            will be multiplied with each color component (range [0..1]).
     * \param[in] specularCoefficient controls in combination with the specular power
     *            how shiny the material will appear (range [0..1]).
     * \param[in] specularPower controls in combination with the specular coefficient
     *            how shiny the material will appear (range [0..inf]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     */
    Material(double red,
             double green,
             double blue,
             double colorCoefficient,
             double specularCoefficient,
             double specularPower,
             double opacity);

    /**
     * \brief Constructor with color and lighting properties.
     *
     * All values besides the given ones are set to defaults as
     * described in the default constructor.
     *
     * \param[in] color the material color in RGB. Each RGB value should be in the
     *            range [0..1].
     * \param[in] colorCoefficient a scaling factor for the color coefficient which
     *            will be multiplied with each color component (range [0..1]).
     * \param[in] specularCoefficient controls in combination with the specular power
     *            how shiny the material will appear (range [0..1]).
     * \param[in] specularPower controls in combination with the specular coefficient
     *            how shiny the material will appear (range [0..inf]).
     * \param[in] opacity the opacity of the material. 0.0 means fully transparent
     *            and 1.0 means solid.
     */
    Material(Color color, double colorCoefficient, double specularCoefficient, double specularPower, double opacity);

    /** \brief Copy constructor. */
    Material(const Material &property);

    /**
     * \brief Copy constructor with color and opacity override.
     *
     * The values are copied from the given property and afterwards the values
     * provided for red, green, blue, and opacity are written into the object.
     */
    Material(
      const Material &property, double red, double green, double blue, double opacity = 1.0, std::string name = "");

    /** \brief Initialize all member variables to standard default values. */
    virtual void InitializeStandardValues();

    /** \brief Re-apply all current property values, triggering Modified(). */
    virtual void Update();

    std::string m_Name;

    Color m_Color;

    Color m_SpecularColor;

    double m_ColorCoefficient;

    double m_SpecularCoefficient;

    double m_SpecularPower;

    double m_Opacity;

    float m_LineWidth;

    InterpolationType m_Interpolation;

    RepresentationType m_Representation;
  };

  typedef itk::VectorContainer<unsigned int, Material::Pointer> MaterialVectorContainer;
}

#endif
