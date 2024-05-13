/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROI_h
#define mitkROI_h

#include <mitkBaseData.h>
#include <MitkROIExports.h>

namespace mitk
{
  /** \brief A collection of region of interests (ROIs).
    *
    * \note This class is considered experimental and subject to substational change. We mean it.
    *
    * ROIs, essentially defined by the minimum and maximum index coordinates of an axis-aligned box, are
    * represented by the nested ROI::Element class. These index coordinates are relative to a common
    * TimeGeometry.
    *
    * All ROIs are required to have a unique ID by which they can be accessed.
    *
    * ROIs can optionally have properties (PropertyList), also called default properties. In case of
    * time-resolved ROIs, each time step can optionally have properties, too. These properties have
    * precedence over the default properties. In other words, the default properties may contain
    * fallback properties which are queried when a property is not defined at a certain time step.
    * This allows for an opt-in dynamic appearance of ROIs over time, for example by changing
    * color or opacity.
    *
    * ROIs are rendered both in 3-d and 2-d views as cubes, resp. cutting slices of these cubes.
    * They support the following ROI::Element properties:
    *
    *   - \c color (ColorProperty): Color of the cube
    *   - \c opacity (FloatProperty): Opacity of the cube
    *   - \c lineWidth (FloatProperty): %Line width of the cube edges
    *
    * ROIs display a customizable caption at their bottom-left corner. It is defined by the base data
    * property \c caption (StringProperty). By default it is set to <tt>"{name} ({ID})"</tt>. Braces
    * are used to define placeholders which are replaced by their corresponding ROI::Element properties.
    * <tt>{ID}</tt> is a special placeholder which will be replaced by the ID of the ROI::Element instead.
    * The caption is allowed to include line breaks. Rendering of captions can be customized through the
    * following data node properties:
    *
    *   - \c font.size (IntProperty) Font size in points
    *   - \c font.bold (BoolProperty) Bold font style
    *   - \c font.italic (BoolProperty) Italic font style
    *
    * See \ref MITKROIPage for details on the JSON-based MITK %ROI file format.
    */
  class MITKROI_EXPORT ROI : public BaseData
  {
  public:
    /** \brief Encapsulates a single (possibly time-resolved) %ROI.
      *
      * \sa ROI
      */
    class MITKROI_EXPORT Element : public IPropertyOwner
    {
    public:
      using PointsType = std::map<TimeStepType, Point3D>;
      using PropertyListsType = std::map<TimeStepType, PropertyList::Pointer>;

      Element();
      explicit Element(unsigned int id);
      ~Element() = default;

      /** \brief Get a const property.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        * An empty context name addresses the default properties.
        */
      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) const override;
      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = true) const;

      /** \brief Get all property keys.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        * An empty context name addresses the default properties.
        */
      std::vector<std::string> GetPropertyKeys(const std::string& contextName = "", bool includeDefaultContext = false) const override;
      std::vector<std::string> GetPropertyKeys(TimeStepType t, bool includeDefaultContext = false) const;

      /** \brief Get all property context names (stringified time steps).
        */
      std::vector<std::string> GetPropertyContextNames() const override;

      /** \brief Get a property.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        * An empty context name addresses the default properties.
        */
      BaseProperty* GetNonConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) override;
      BaseProperty* GetNonConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = true);

      /** \brief Set a property.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        * An empty context name addresses the default properties.
        */
      void SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;
      void SetProperty(const std::string& propertyKey, BaseProperty* property, TimeStepType t, bool fallBackOnDefaultContext = false);

      /** \brief Remove a property.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        * An empty context name addresses the default properties.
        */
      void RemoveProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;
      void RemoveProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = false);

      unsigned int GetID() const;
      void SetID(unsigned int id);

      /** \brief Check if the %ROI is defined for a certain time step.
        */
      bool HasTimeStep(TimeStepType t) const;

      /** \brief Check if the %ROI can be considered time-resolved.
        */
      bool HasTimeSteps() const;

      /** \brief Get all valid time steps that have a minimum point and a maximum point.
        */
      std::vector<TimeStepType> GetTimeSteps() const;

      Point3D GetMin(TimeStepType t = 0) const;
      void SetMin(const Point3D& min, TimeStepType t = 0);

      Point3D GetMax(TimeStepType t = 0) const;
      void SetMax(const Point3D& max, TimeStepType t = 0);

      PropertyList* GetDefaultProperties() const;
      void SetDefaultProperties(PropertyList* properties);

      /** \brief Get properties for a certain time step or \c nullptr if absent.
        */
      PropertyList* GetProperties(TimeStepType t = 0) const;

      void SetProperties(PropertyList* properties, TimeStepType t = 0);

    private:
      unsigned int m_ID;
      PointsType m_Min;
      PointsType m_Max;
      PropertyList::Pointer m_DefaultProperties;
      PropertyListsType m_Properties;
    };

    mitkClassMacro(ROI, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    using ElementsType = std::map<unsigned int, Element>;
    using Iterator = ElementsType::iterator;
    using ConstIterator = ElementsType::const_iterator;

    size_t GetNumberOfElements() const;

    /** \brief Add a ROI::Element to the collection.
      *
      * \note The ID of the ROI::Element must be set to a unique number in advance.
      */
    void AddElement(const Element& element);

    const Element& GetElement(unsigned int id) const;
    Element& GetElement(unsigned int id);

    ConstIterator begin() const;
    ConstIterator end() const;

    Iterator begin();
    Iterator end();

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject* data) override;

  protected:
    mitkCloneMacro(Self)

    ROI();
    ROI(const Self& other);
    ~ROI() override;

  private:
    ElementsType m_Elements;
  };

  MITKROI_EXPORT void to_json(nlohmann::json& j, const ROI::Element& roi);
  MITKROI_EXPORT void from_json(const nlohmann::json& j, ROI::Element& roi);
}

#endif
