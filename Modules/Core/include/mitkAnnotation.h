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

#ifndef Annotation_H
#define Annotation_H

#include "mitkServiceInterface.h"
#include "usServiceRegistration.h"
#include <MitkCoreExports.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>

namespace mitk
{
  /** \brief Base class for all Annotation
   * This class is to be implemented in order to create Annotation which are managed by a AbstractAnnotationRenderer.
   * This class contains an internal Propertylist for configuring the appearance of the implemented Overlay. */
  class MITKCORE_EXPORT Annotation : public itk::Object
  {
  public:
    /** \brief Container for position and size on the display.*/
    struct Bounds
    {
      itk::Point<double, 2> Position;
      itk::Point<double, 2> Size;
    };

    /** \brief Base class for mapper specific rendering ressources.
     */
    class MITKCORE_EXPORT BaseLocalStorage
    {
    public:
      bool IsGenerateDataRequired(mitk::BaseRenderer *renderer, mitk::Annotation *Annotation);

      inline void UpdateGenerateDataTime() { m_LastGenerateDataTime.Modified(); }
      inline itk::TimeStamp &GetLastGenerateDataTime() { return m_LastGenerateDataTime; }
    protected:
      /** \brief timestamp of last update of stored data */
      itk::TimeStamp m_LastGenerateDataTime;
    };

    /**
    * @brief Set the property (instance of BaseProperty) with key @a propertyKey in the PropertyList
    * of the @a renderer (if nullptr, use BaseRenderer-independent PropertyList). This is set-by-value.
    *
    * @warning Change in semantics since Aug 25th 2006. Check your usage of this method if you do
    *          more with properties than just call <tt>SetProperty( "key", new SomeProperty("value") )</tt>.
    *
    * @sa GetProperty
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    void SetProperty(const std::string &propertyKey, const BaseProperty::Pointer &property);

    /**
    * @brief Replace the property (instance of BaseProperty) with key @a propertyKey in the PropertyList
    * of the @a renderer (if nullptr, use BaseRenderer-independent PropertyList). This is set-by-reference.
    *
    * If @a renderer is @a nullptr the property is set in the BaseRenderer-independent
    * PropertyList of this Annotation.
    * @sa GetProperty
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    void ReplaceProperty(const std::string &propertyKey, const BaseProperty::Pointer &property);

    /**
    * @brief Add the property (instance of BaseProperty) if it does
    * not exist (or always if \a overwrite is \a true)
    * with key @a propertyKey in the PropertyList
    * of the @a renderer (if nullptr, use BaseRenderer-independent
    * PropertyList). This is set-by-value.
    *
    * For \a overwrite == \a false the property is \em not changed
    * if it already exists. For \a overwrite == \a true the method
    * is identical to SetProperty.
    *
    * @sa SetProperty
    * @sa GetProperty
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    void AddProperty(const std::string &propertyKey, const BaseProperty::Pointer &property, bool overwrite = false);

    /**
    * @brief Add values from another PropertyList.
    *
    * Overwrites values in m_PropertyList only when possible (i.e. when types are compatible).
    * If you want to allow for object type changes (replacing a "visible":BoolProperty with "visible":IntProperty,
    * set the @param replace.
    *
    * @param replace true: if @param pList contains a property "visible" of type ColorProperty and our m_PropertyList
    * also has a "visible" property of a different type (e.g. BoolProperty), change the type, i.e. replace the objects
    * behind the pointer.
    *
    * @sa SetProperty
    * @sa ReplaceProperty
    * @sa m_PropertyList
    */
    void ConcatenatePropertyList(PropertyList *pList, bool replace = false);

    /**
    * @brief Get the property (instance of BaseProperty) with key @a propertyKey from the PropertyList
    * of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
    *
    * If @a renderer is @a nullptr or the @a propertyKey cannot be found
    * in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
    * PropertyList of this Annotation is queried.
    * @sa GetPropertyList
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    mitk::BaseProperty *GetProperty(const std::string &propertyKey) const;

    /**
    * @brief Get the property of type T with key @a propertyKey from the PropertyList
    * of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
    *
    * If @a renderer is @a nullptr or the @a propertyKey cannot be found
    * in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
    * PropertyList of this Annotation is queried.
    * @sa GetPropertyList
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    template <typename T>
    bool GetProperty(itk::SmartPointer<T> &property, const std::string &propertyKey) const
    {
      property = dynamic_cast<T *>(GetProperty(propertyKey));
      return property.IsNotNull();
    }

    /**
    * @brief Get the property of type T with key @a propertyKey from the PropertyList
    * of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
    *
    * If @a renderer is @a nullptr or the @a propertyKey cannot be found
    * in the PropertyList specific to @a renderer or is disabled there, the BaseRenderer-independent
    * PropertyList of this Annotation is queried.
    * @sa GetPropertyList
    * @sa m_PropertyList
    * @sa m_MapOfPropertyLists
    */
    template <typename T>
    bool GetProperty(T *&property, const std::string &propertyKey) const
    {
      property = dynamic_cast<T *>(GetProperty(propertyKey));
      return property != nullptr;
    }

    /**
    * @brief Convenience access method for GenericProperty<T> properties
    * (T being the type of the second parameter)
    * @return @a true property was found
    */
    template <typename T>
    bool GetPropertyValue(const std::string &propertyKey, T &value) const
    {
      GenericProperty<T> *gp = dynamic_cast<GenericProperty<T> *>(GetProperty(propertyKey));
      if (gp != nullptr)
      {
        value = gp->GetValue();
        return true;
      }
      return false;
    }

    /**
    * @brief Convenience access method for bool properties (instances of
    * BoolProperty)
    * @return @a true property was found
    */
    bool GetBoolProperty(const std::string &propertyKey, bool &boolValue) const;

    /**
    * @brief Convenience access method for int properties (instances of
    * IntProperty)
    * @return @a true property was found
    */
    bool GetIntProperty(const std::string &propertyKey, int &intValue) const;

    /**
    * @brief Convenience access method for float properties (instances of
    * FloatProperty)
    * @return @a true property was found
    */
    bool GetFloatProperty(const std::string &propertyKey, float &floatValue) const;

    /**
    * @brief Convenience access method for double properties (instances of
    * DoubleProperty)
    * @return @a true property was found
    */
    bool GetDoubleProperty(const std::string &propertyKey, double &doubleValue) const;

    /**
    * @brief Convenience access method for string properties (instances of
    * StringProperty)
    * @return @a true property was found
    */
    bool GetStringProperty(const std::string &propertyKey, std::string &string) const;

    /**
    * @brief Convenience method for setting int properties (instances of
    * IntProperty)
    */
    void SetIntProperty(const std::string &propertyKey, int intValue);

    /**
    * @brief Convenience method for setting int properties (instances of
    * IntProperty)
    */
    void SetBoolProperty(const std::string &propertyKey, bool boolValue);

    /**
    * @brief Convenience method for setting int properties (instances of
    * IntProperty)
    */
    void SetFloatProperty(const std::string &propertyKey, float floatValue);

    /**
    * @brief Convenience method for setting int properties (instances of
    * IntProperty)
    */
    void SetDoubleProperty(const std::string &propertyKey, double doubleValue);

    /**
    * @brief Convenience method for setting int properties (instances of
    * IntProperty)
    */
    void SetStringProperty(const std::string &propertyKey, const std::string &string);

    /**
    * @brief Convenience access method for boolean properties (instances
    * of BoolProperty). Return value is the value of the property. If the property is
    * not found, the value of @a defaultIsOn is returned.
    *
    * Thus, the return value has a different meaning than in the
    * GetBoolProperty method!
    * @sa GetBoolProperty
    */
    bool IsOn(const std::string &propertyKey, bool defaultIsOn = true) const
    {
      GetBoolProperty(propertyKey, defaultIsOn);
      return defaultIsOn;
    }

    /**
    * @brief Convenience access method for accessing the name of an object (instance of
    * StringProperty with property-key "name")
    * @return @a true property was found
    */
    bool GetName(std::string &nodeName, const std::string &propertyKey = "name") const;

    /**
    * @brief Extra convenience access method for accessing the name of an object (instance of
    * StringProperty with property-key "name").
    *
    * This method does not take the renderer specific
    * propertylists into account, because the name of an object should never be renderer specific.
    * @returns a std::string with the name of the object (content of "name" Property).
    * If there is no "name" Property, an empty string will be returned.
    */
    virtual std::string GetName() const;

    /**
    * @brief Extra convenience access method to set the name of an object.
    *
    * The name will be stored in the non-renderer-specific PropertyList in a StringProperty named "name".
    */
    virtual void SetName(const std::string &name);

    /**
    * @brief Convenience access method for color properties (instances of
    * ColorProperty)
    * @return @a true property was found
    */
    bool GetColor(float rgb[], const std::string &propertyKey = "color") const;
    /**
    * @brief Convenience method for setting color properties (instances of
    * ColorProperty)
    */
    void SetColor(const mitk::Color &color, const std::string &propertyKey = "color");
    /**
    * @brief Convenience method for setting color properties (instances of
    * ColorProperty)
    */
    void SetColor(float red, float green, float blue, const std::string &propertyKey = "color");
    /**
    * @brief Convenience method for setting color properties (instances of
    * ColorProperty)
    */
    void SetColor(const float rgb[], const std::string &propertyKey = "color");
    /**
    * @brief Convenience access method for opacity properties (instances of
    * FloatProperty)
    * @return @a true property was found
    */
    bool GetOpacity(float &opacity, const std::string &propertyKey = "opacity") const;
    /**
    * @brief Convenience method for setting opacity properties (instances of
    * FloatProperty)
    */
    void SetOpacity(float opacity, const std::string &propertyKey = "opacity");

    void SetText(std::string text);

    std::string GetText() const;

    void SetFontSize(int fontSize);

    int GetFontSize() const;

    /**
    * @brief Convenience access method for visibility properties (instances
    * of BoolProperty with property-key "visible")
    * @return @a true property was found
    * @sa IsVisible
    */
    bool GetVisibility(bool &visible, const std::string &propertyKey = "visible") const;

    /**
    * @brief Convenience access method for visibility properties (instances
    * of BoolProperty). Return value is the visibility. Default is
    * visible==true, i.e., true is returned even if the property (@a
    * propertyKey) is not found.
    *
    * Thus, the return value has a different meaning than in the
    * GetVisibility method!
    * @sa GetVisibility
    * @sa IsOn
    */
    bool IsVisible(const std::string &propertyKey = "visible", bool defaultIsOn = true) const;

    /**
    * @brief Convenience method for setting visibility properties (instances
    * of BoolProperty)
    * @param visible If set to true, the data will be rendered. If false, the render will skip this data.
    * @param renderer Specify a renderer if the visibility shall be specific to a renderer
    * @param propertykey Can be used to specify a user defined name of the visibility propery.
    */
    void SetVisibility(bool visible, const std::string &propertyKey = "visible");

    /** \brief Adds the Annotation to the specified renderer. Update Annotation should be called soon in order to apply
     * all
     * properties*/
    virtual void AddToBaseRenderer(BaseRenderer *renderer) = 0;

    /** \brief Adds the Annotation to the specified renderer. Update Annotation should be called soon in order to apply
     * all
     * properties*/
    virtual void AddToRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) = 0;

    /** \brief Removes the Annotation from the specified renderer. It is not visible anymore then.*/
    virtual void RemoveFromBaseRenderer(BaseRenderer *renderer) = 0;

    /** \brief Removes the Annotation from the specified renderer. It is not visible anymore then.*/
    virtual void RemoveFromRenderer(BaseRenderer *renderer, vtkRenderer *vtkrenderer) = 0;

    /** \brief Applies all properties and should be called before the rendering procedure.*/
    virtual void Update(BaseRenderer *renderer) = 0;

    /** \brief Returns position and size of the Annotation on the display.*/
    virtual Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const;

    /** \brief Sets position and size of the Annotation on the display.*/
    virtual void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds &);

    void SetForceInForeground(bool forceForeground);

    bool IsForceInForeground() const;

    PropertyList *GetPropertyList() const;

    /**
    *\brief Returns the id that this device is registered with. The id will only be valid, if the
    * Annotation has been registered using RegisterAsMicroservice().
    */
    std::string GetMicroserviceID();

    /**
    *\brief These Constants are used in conjunction with Microservices
    */
    static const std::string US_INTERFACE_NAME;
    static const std::string US_PROPKEY_AnnotationNAME;
    static const std::string US_PROPKEY_ID;
    static const std::string US_PROPKEY_MODIFIED;
    static const std::string US_PROPKEY_RENDERER_ID;
    static const std::string US_PROPKEY_AR_ID;

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    * To unregister, call UnregisterMicroservice().
    */
    virtual void RegisterAsMicroservice(us::ServiceProperties props);

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    */
    virtual void UnRegisterMicroservice();

    void AnnotationModified();

    mitkClassMacroItkParent(Annotation, itk::Object);

  protected:
    /** \brief explicit constructor which disallows implicit conversions */
    Annotation();

    /** \brief virtual destructor in order to derive from this class */
    ~Annotation() override;

    /**
    * @brief BaseRenderer-independent PropertyList
    *
    * Properties herein can be overwritten specifically for each BaseRenderer
    * by the BaseRenderer-specific properties defined in m_MapOfPropertyLists.
    */
    PropertyList::Pointer m_PropertyList;

    /**
    * @brief Timestamp of the last change of m_Data
    */
    itk::TimeStamp m_DataReferenceChangedTime;

    void SetUSProperty(const std::string &propertyKey, us::Any value);

  private:
    /** \brief render this Annotation on a foreground renderer */
    bool m_ForceInForeground;

    /** \brief copy constructor */
    Annotation(const Annotation &);

    /** \brief assignment operator */
    Annotation &operator=(const Annotation &);

  private:
    us::ServiceRegistration<Self> m_ServiceRegistration;

    unsigned long m_PropertyListModifiedObserverTag;
    void PropertyListModified(const itk::Object *, const itk::EventObject &);
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::Annotation, "org.mitk.services.Annotation")

#endif // Annotation_H
