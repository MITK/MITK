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
      /** \brief Map from time steps to 3D points (used for min/max corners). */
      using PointsType = std::map<TimeStepType, Point3D>;

      /** \brief Map from time steps to property lists. */
      using PropertyListsType = std::map<TimeStepType, PropertyList::Pointer>;

      /** \brief Default constructor. Initializes the element with ID 0. */
      Element();

      /**
       * \brief Construct an element with the given unique ID.
       *
       * \param[in] id The unique identifier for this ROI element.
       */
      explicit Element(unsigned int id);

      /**
       * \brief Copy constructor. Deep-copies all properties.
       *
       * \param[in] other The element to copy from.
       */
      Element(const Element& other);

      /**
       * \brief Copy-and-swap assignment operator.
       *
       * \param[in] other The element to assign from (passed by value for copy-and-swap).
       * \return Reference to this element.
       */
      Element& operator=(Element other);

      ~Element() = default;

      /** \brief Get a const property by key and optional context.
        *
        * \param[in] propertyKey The key of the property to retrieve.
        * \param[in] contextName The context (stringified time step) to look up. An empty string
        *            addresses the default properties.
        * \param[in] fallBackOnDefaultContext If \c true and the property is not found in the
        *            specified context, fall back to the default properties.
        * \return The property, or \c nullptr if not found.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        */
      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) const override;

      /**
       * \brief Get a const property by key and time step.
       *
       * \param[in] propertyKey The key of the property to retrieve.
       * \param[in] t The time step to look up.
       * \param[in] fallBackOnDefaultContext If \c true and the property is not found at time
       *            step \p t, fall back to the default properties.
       * \return The property, or \c nullptr if not found.
       */
      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = true) const;

      /** \brief Get all property keys for a given context.
        *
        * \param[in] contextName The context (stringified time step) to query. An empty string
        *            addresses the default properties.
        * \param[in] includeDefaultContext If \c true, also include keys from the default properties.
        * \return A vector of property key strings.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        */
      std::vector<std::string> GetPropertyKeys(const std::string& contextName = "", bool includeDefaultContext = false) const override;

      /**
       * \brief Get all property keys for a given time step.
       *
       * \param[in] t The time step to query.
       * \param[in] includeDefaultContext If \c true, also include keys from the default properties
       *            that are not already present at time step \p t.
       * \return A vector of property key strings.
       */
      std::vector<std::string> GetPropertyKeys(TimeStepType t, bool includeDefaultContext = false) const;

      /** \brief Get all property context names (stringified time steps).
        */
      std::vector<std::string> GetPropertyContextNames() const override;

      /** \brief Get a mutable property by key and optional context.
        *
        * \param[in] propertyKey The key of the property to retrieve.
        * \param[in] contextName The context (stringified time step) to look up. An empty string
        *            addresses the default properties.
        * \param[in] fallBackOnDefaultContext If \c true and the property is not found in the
        *            specified context, fall back to the default properties.
        * \return Pointer to the property, or \c nullptr if not found.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        */
      BaseProperty* GetNonConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) override;

      /**
       * \brief Get a mutable property by key and time step.
       *
       * \param[in] propertyKey The key of the property to retrieve.
       * \param[in] t The time step to look up.
       * \param[in] fallBackOnDefaultContext If \c true and the property is not found at time
       *            step \p t, fall back to the default properties.
       * \return Pointer to the property, or \c nullptr if not found.
       */
      BaseProperty* GetNonConstProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = true);

      /** \brief Set a property for a given context.
        *
        * \param[in] propertyKey The key of the property to set.
        * \param[in] property The property value to assign.
        * \param[in] contextName The context (stringified time step) to set the property in.
        *            An empty string addresses the default properties.
        * \param[in] fallBackOnDefaultContext If \c true and the specified context does not exist,
        *            set the property in the default properties instead.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        */
      void SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;

      /**
       * \brief Set a property at a specific time step.
       *
       * \param[in] propertyKey The key of the property to set.
       * \param[in] property The property value to assign.
       * \param[in] t The time step at which to set the property.
       * \param[in] fallBackOnDefaultContext If \c true and the time step does not exist,
       *            set the property in the default properties. If \c false and the time step
       *            does not exist, an exception is thrown.
       * \throw mitk::Exception if \p fallBackOnDefaultContext is \c false and the time step
       *        does not exist.
       */
      void SetProperty(const std::string& propertyKey, BaseProperty* property, TimeStepType t, bool fallBackOnDefaultContext = false);

      /** \brief Remove a property from a given context.
        *
        * \param[in] propertyKey The key of the property to remove.
        * \param[in] contextName The context (stringified time step) to remove the property from.
        *            An empty string addresses the default properties.
        * \param[in] fallBackOnDefaultContext If \c true and the specified context does not exist,
        *            remove the property from the default properties instead.
        *
        * \note A time step can be specified as context. Use \c std::to_string() to convert a time step to a context name.
        */
      void RemoveProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;

      /**
       * \brief Remove a property at a specific time step.
       *
       * \param[in] propertyKey The key of the property to remove.
       * \param[in] t The time step from which to remove the property.
       * \param[in] fallBackOnDefaultContext If \c true and the time step does not exist,
       *            remove the property from the default properties. If \c false and the time
       *            step does not exist, an exception is thrown.
       * \throw mitk::Exception if \p fallBackOnDefaultContext is \c false and the time step
       *        does not exist.
       */
      void RemoveProperty(const std::string& propertyKey, TimeStepType t, bool fallBackOnDefaultContext = false);

      /**
       * \brief Get the unique ID of this ROI element.
       *
       * \return The unique identifier.
       */
      unsigned int GetID() const;

      /**
       * \brief Set the unique ID of this ROI element.
       *
       * \param[in] id The unique identifier to assign. Must be unique within the parent ROI collection.
       */
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

      /**
       * \brief Get the minimum corner point of the axis-aligned bounding box.
       *
       * \param[in] t The time step to query (default: 0).
       * \return The minimum corner point in index coordinates.
       * \throw std::out_of_range if the time step does not exist.
       */
      Point3D GetMin(TimeStepType t = 0) const;

      /**
       * \brief Set the minimum corner point of the axis-aligned bounding box.
       *
       * \param[in] min The minimum corner point in index coordinates.
       * \param[in] t The time step to set (default: 0).
       */
      void SetMin(const Point3D& min, TimeStepType t = 0);

      /**
       * \brief Get the maximum corner point of the axis-aligned bounding box.
       *
       * \param[in] t The time step to query (default: 0).
       * \return The maximum corner point in index coordinates.
       * \throw std::out_of_range if the time step does not exist.
       */
      Point3D GetMax(TimeStepType t = 0) const;

      /**
       * \brief Set the maximum corner point of the axis-aligned bounding box.
       *
       * \param[in] max The maximum corner point in index coordinates.
       * \param[in] t The time step to set (default: 0).
       */
      void SetMax(const Point3D& max, TimeStepType t = 0);

      /**
       * \brief Get the default (time-step-independent) property list.
       *
       * \return Pointer to the default property list. Never \c nullptr.
       */
      PropertyList* GetDefaultProperties() const;

      /**
       * \brief Set the default property list.
       *
       * \param[in] properties The property list to use as defaults. If \c nullptr, a new
       *            empty property list is created.
       */
      void SetDefaultProperties(PropertyList* properties);

      /** \brief Get properties for a certain time step or \c nullptr if absent.
        */
      PropertyList* GetProperties(TimeStepType t = 0) const;

      /**
       * \brief Set the property list for a specific time step.
       *
       * \param[in] properties The property list to assign.
       * \param[in] t The time step to associate the properties with (default: 0).
       */
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

    /** \brief Map type associating element IDs to their Element objects. */
    using ElementsType = std::map<unsigned int, Element>;

    /** \brief Mutable iterator over ROI elements. */
    using Iterator = ElementsType::iterator;

    /** \brief Const iterator over ROI elements. */
    using ConstIterator = ElementsType::const_iterator;

    /**
     * \brief Get the number of ROI elements in the collection.
     *
     * \return The number of elements.
     */
    size_t GetNumberOfElements() const;

    /** \brief Add a ROI::Element to the collection.
      *
      * \param[in] element The element to add. Its ID must be unique within this collection.
      * \pre The element's ID must not already exist in this collection.
      * \throw mitk::Exception if an element with the same ID already exists.
      */
    void AddElement(const Element& element);

    /**
     * \brief Get a const reference to an element by its ID.
     *
     * \param[in] id The unique identifier of the element.
     * \return Const reference to the element.
     * \throw std::out_of_range if no element with the given ID exists.
     */
    const Element& GetElement(unsigned int id) const;

    /**
     * \brief Get a mutable reference to an element by its ID.
     *
     * \param[in] id The unique identifier of the element.
     * \return Mutable reference to the element.
     * \throw std::out_of_range if no element with the given ID exists.
     */
    Element& GetElement(unsigned int id);

    /** \brief Get a const iterator to the first element. */
    ConstIterator begin() const;

    /** \brief Get a const iterator past the last element. */
    ConstIterator end() const;

    /** \brief Get a mutable iterator to the first element. */
    Iterator begin();

    /** \brief Get a mutable iterator past the last element. */
    Iterator end();

    /** \brief Set the requested region to the largest possible region (no-op for ROI). */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check whether the requested region is outside of the buffered region.
     *
     * \return Always returns \c false for ROI.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify the requested region.
     *
     * \return Always returns \c true for ROI.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object (no-op for ROI).
     *
     * \param[in] data The data object to copy the requested region from (ignored).
     */
    void SetRequestedRegion(const itk::DataObject* data) override;

  protected:
    mitkCloneMacro(Self)

    ROI();
    ROI(const Self& other);
    ~ROI() override;

  private:
    ElementsType m_Elements;
  };

  /**
   * \brief Serialize a ROI::Element to JSON (nlohmann::json ADL customization point).
   *
   * \param[out] j The JSON object to populate.
   * \param[in] roi The ROI element to serialize.
   *
   * \sa from_json
   */
  MITKROI_EXPORT void to_json(nlohmann::json& j, const ROI::Element& roi);

  /**
   * \brief Deserialize a ROI::Element from JSON (nlohmann::json ADL customization point).
   *
   * \param[in] j The JSON object to read from.
   * \param[out] roi The ROI element to populate.
   *
   * \sa to_json
   */
  MITKROI_EXPORT void from_json(const nlohmann::json& j, ROI::Element& roi);
}

#endif
