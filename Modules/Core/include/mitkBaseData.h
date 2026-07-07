/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseData_h
#define mitkBaseData_h

#include <itkDataObject.h>

#include <mitkBaseDataSource.h>
#include <mitkIdentifiable.h>
#include <mitkIPropertyOwner.h>
#include <mitkOperationActor.h>
#include <mitkPropertyList.h>
#include <mitkTimeGeometry.h>
#include <MitkCoreExports.h>

namespace mitk
{
  // class BaseProcess;

  /**
   * \brief Base class of all data objects.
   *
   * BaseData is the common base class for all MITK data objects such as images,
   * surfaces, contours, and point sets. It inherits from itk::DataObject and
   * can therefore be included in ITK-style processing pipelines.
   *
   * It also inherits from OperationActor, enabling it to receive and execute
   * Operation objects (e.g., for undo/redo support), from Identifiable, giving
   * each data object a unique identifier (UID), and from IPropertyOwner,
   * providing a key-value property list for arbitrary metadata storage.
   *
   * Every BaseData carries a TimeGeometry that describes the spatial and temporal
   * extent of the data. Sub-classes are expected to initialize this geometry
   * appropriately and to implement the pure virtual pipeline region methods.
   *
   * \remark Some derived classes may support the persistence of the Identifiable UID,
   * but this is not a guaranteed feature and also depends on the format the data
   * is stored in, as not all formats support storing of meta information. Please check
   * the documentation of the IFileReader and IFileWriter classes to see if UID
   * persistence is supported. MITK SceneIO supports UID persistence for all
   * BaseData-derived classes.
   *
   * \sa mitk::Image, mitk::Surface, mitk::PointSet, mitk::SlicedData
   * \sa mitk::TimeGeometry, mitk::BaseGeometry
   * \sa mitk::PropertyList, mitk::IPropertyOwner
   * \ingroup Data
   */
  class MITKCORE_EXPORT BaseData
    : public itk::DataObject, public OperationActor, public Identifiable, public IPropertyOwner
  {
  public:
    mitkClassMacroItkParent(BaseData, itk::DataObject);

    // IPropertyProvider

    /**
     * \brief Get a const property by its key.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] contextName The context name for the property lookup. An empty string
     *            denotes the default context.
     * \param[in] fallBackOnDefaultContext If true and the property is not found in the
     *            specified context, the default context is searched as well.
     * \return A const smart pointer to the property, or nullptr if not found.
     * \sa IPropertyProvider
     */
    BaseProperty::ConstPointer GetConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) const override;

    /**
     * \brief Get all property keys stored in the property list.
     *
     * \param[in] contextName The property context to query. An empty string denotes the
     *            default context.
     * \param[in] includeDefaultContext If true, keys from the default context are included
     *            even when a non-default context is queried.
     * \return A vector of property key strings.
     * \sa IPropertyProvider
     */
    std::vector<std::string> GetPropertyKeys(const std::string &contextName = "", bool includeDefaultContext = false) const override;

    /**
     * \brief Get the names of all property contexts.
     *
     * BaseData has no sub-contexts by default, so this returns an empty vector.
     *
     * \return An empty vector (no non-default contexts supported in the base class).
     * \sa IPropertyProvider
     */
    std::vector<std::string> GetPropertyContextNames() const override;

    // IPropertyOwner

    /**
     * \brief Get a non-const property by its key.
     *
     * \param[in] propertyKey The key identifying the property.
     * \param[in] contextName The context name for the property lookup.
     * \param[in] fallBackOnDefaultContext If true, the default context is used as fallback.
     * \return A raw pointer to the property, or nullptr if not found.
     * \sa IPropertyOwner
     */
    BaseProperty * GetNonConstProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = true) override;

    /**
     * \brief Set a property in the property list.
     *
     * \param[in] propertyKey The key identifying the property. Must not be empty.
     * \param[in] property The property to set.
     * \param[in] contextName The context name. An empty string denotes the default context.
     * \param[in] fallBackOnDefaultContext If true, the property is stored in the default
     *            context when the specified context is unknown.
     * \throw mitk::Exception if the property key is empty or if a non-default context
     *        is specified without fallback.
     * \sa IPropertyOwner
     */
    void SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Remove a property from the property list.
     *
     * \param[in] propertyKey The key of the property to remove. Must not be empty.
     * \param[in] contextName The context name. An empty string denotes the default context.
     * \param[in] fallBackOnDefaultContext If true, the property is removed from the default
     *            context when the specified context is unknown.
     * \throw mitk::Exception if the property key is empty or if a non-default context
     *        is specified without fallback.
     * \sa IPropertyOwner
     */
    void RemoveProperty(const std::string &propertyKey, const std::string &contextName = "", bool fallBackOnDefaultContext = false) override;

    /**
     * \brief Return the TimeGeometry of the data as const pointer.
     *
     * \warning No update will be called. Use GetUpdatedTimeGeometry() if you cannot
     * be sure that the geometry is up-to-date.
     *
     * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
     *
     * \return Const pointer to the TimeGeometry, or nullptr if not set.
     * \sa GetUpdatedTimeGeometry, GetGeometry
     */
    const mitk::TimeGeometry *GetTimeGeometry() const
    {
      return m_TimeGeometry.GetPointer();
    }

    /**
     * \brief Return the TimeGeometry of the data as non-const pointer.
     *
     * \warning No update will be called. Use GetUpdatedTimeGeometry() if you cannot
     * be sure that the geometry is up-to-date.
     *
     * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
     *
     * \return Non-const pointer to the TimeGeometry.
     * \sa GetUpdatedTimeGeometry, GetGeometry
     */
    mitk::TimeGeometry *GetTimeGeometry() { return m_TimeGeometry.GetPointer(); }

    /**
     * \brief Return the TimeGeometry of the data after ensuring it is up-to-date.
     *
     * Unlike GetTimeGeometry(), this method sets the requested region to the
     * largest possible region and calls UpdateOutputInformation() before
     * returning the TimeGeometry, ensuring the geometry is current.
     *
     * \return Const pointer to the up-to-date TimeGeometry.
     * \sa GetTimeGeometry
     */
    const mitk::TimeGeometry *GetUpdatedTimeGeometry();

    /**
     * \brief Expand the TimeGeometry to a number of time steps.
     *
     * The method expands the TimeGeometry to the given number of time steps,
     * filling newly created elements with empty geometries. Sub-classes should
     * override this method to handle the elongation of their data vectors, too.
     *
     * \note Shrinking is neither possible nor intended. If \a timeSteps is less
     * than or equal to the current number of time steps, no action is taken.
     *
     * \param[in] timeSteps The desired number of time steps.
     * \sa TimeGeometry::Expand
     */
    virtual void Expand(unsigned int timeSteps);

    /**
     * \brief Return the BaseGeometry of the data at time step \a t after ensuring it is up-to-date.
     *
     * Unlike GetGeometry(), this method sets the requested region to the
     * largest possible region and calls UpdateOutputInformation() before
     * returning the geometry, ensuring the geometry is current.
     *
     * \param[in] t The time step for which to retrieve the geometry (default: 0).
     * \return Const pointer to the up-to-date BaseGeometry at the given time step,
     *         or nullptr if no geometry exists.
     * \sa GetGeometry, GetUpdatedTimeGeometry
     * \todo Appropriate setting of the update extent is missing.
     */
    const mitk::BaseGeometry *GetUpdatedGeometry(int t = 0);

    /**
     * \brief Return the BaseGeometry of the data at time step \a t as a non-const pointer.
     *
     * \warning No update will be called. Use GetUpdatedGeometry() if you cannot
     * be sure that the geometry is up-to-date.
     *
     * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
     *
     * \param[in] t The time step for which to retrieve the geometry (default: 0).
     * \return Pointer to the BaseGeometry, or nullptr if the TimeGeometry is not set.
     * \sa GetUpdatedGeometry, GetTimeGeometry
     */
    mitk::BaseGeometry *GetGeometry(int t = 0) const
    {
      if (m_TimeGeometry.IsNull())
        return nullptr;
      return m_TimeGeometry->GetGeometryForTimeStep(t);
    }

    /**
     * \brief Update the information for this BaseData so that it can be used as
     * an output of a BaseProcess.
     *
     * This method is used in the pipeline mechanism to propagate information and
     * initialize the metadata associated with a BaseData. Any implementation
     * of this method in a derived class is assumed to call its source's
     * BaseProcess::UpdateOutputInformation(), which determines modified times,
     * LargestPossibleRegions, and any extra metadata like spacing, origin, etc.
     * The default implementation calls the source's UpdateOutputInformation()
     * and then updates the TimeGeometry bounding box.
     *
     * \note Implementations in derived classes must ensure that the geometry is
     * updated by calling GetTimeGeometry()->UpdateInformation() \em after
     * calling its source's BaseProcess::UpdateOutputInformation().
     */
    void UpdateOutputInformation() override;

    /**
     * \brief Set the RequestedRegion to the LargestPossibleRegion.
     *
     * This forces a filter to produce all of the output in one execution
     * (i.e., not streaming) on the next call to Update().
     */
    void SetRequestedRegionToLargestPossibleRegion() override = 0;

    /**
     * \brief Determine whether the RequestedRegion is outside of the BufferedRegion.
     *
     * This method returns true if the RequestedRegion is outside the
     * BufferedRegion (true if at least one pixel is outside). This is used
     * by the pipeline mechanism to determine whether a filter needs to
     * re-execute in order to satisfy the current request. If the current
     * RequestedRegion is already inside the BufferedRegion from the previous
     * execution (and the current filter is up to date), then a given filter
     * does not need to re-execute.
     *
     * \return True if the requested region is outside the buffered region.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override = 0;

    /**
     * \brief Verify that the RequestedRegion is within the LargestPossibleRegion.
     *
     * If the RequestedRegion is not within the LargestPossibleRegion,
     * then the filter cannot possibly satisfy the request. This method
     * returns true if the request can be satisfied (even if it will be
     * necessary to process the entire LargestPossibleRegion) and returns
     * false otherwise. Used by PropagateRequestedRegion(), which throws
     * an InvalidRequestedRegionError exception if the requested region
     * is not within the LargestPossibleRegion.
     *
     * \return True if the requested region is within the largest possible region.
     */
    bool VerifyRequestedRegion() override = 0;

    /**
     * \brief Copy information from the specified data set.
     *
     * This method is part of the pipeline execution model. By default, a
     * BaseProcess will copy metadata from the first input to all of its
     * outputs. See ProcessObject::GenerateOutputInformation(). Each subclass
     * of DataObject is responsible for copying whatever metadata it needs
     * from another DataObject. The default implementation copies the
     * TimeGeometry and the PropertyList. If a subclass overrides this method,
     * it should always call its superclass version.
     *
     * \param[in] data The data object from which to copy information. Must be
     *            castable to BaseData.
     * \throw itk::ExceptionObject if \a data cannot be cast to BaseData.
     */
    void CopyInformation(const itk::DataObject *data) override;

    /**
     * \brief Check whether the data has been initialized.
     *
     * A data object is considered initialized when at least the geometry and
     * other header data have been set.
     *
     * \warning Set to \a true by default for compatibility reasons. Set
     * m_Initialized=false in constructors of sub-classes that support
     * distinction between initialized and uninitialized state.
     *
     * \return True if the data object has been initialized.
     */
    virtual bool IsInitialized() const;

    /**
     * \brief Reset the data object by calling ClearData() and InitializeEmpty().
     *
     * \warning Only use in subclasses that have reimplemented ClearData() and
     * InitializeEmpty(). Calling Clear() directly on BaseData will reset the
     * object to a non-initialized, invalid state.
     *
     * \sa ClearData, InitializeEmpty
     */
    virtual void Clear();

    /**
     * \brief Check whether the object contains data at the specified time step.
     *
     * For example, a set of points may be empty at a given time step.
     *
     * \warning Returns IsInitialized()==false by default for compatibility
     * reasons. Override in sub-classes that support distinction between
     * empty/non-empty state.
     *
     * \param[in] t The time step to check.
     * \return True if the data is empty at time step \a t.
     */
    virtual bool IsEmptyTimeStep(unsigned int t) const;

    /**
     * \brief Check whether the object contains data at any time step.
     *
     * Iterates over all time steps and returns true only if every time step
     * is empty (or the object is not initialized).
     *
     * \warning Returns IsInitialized()==false by default for compatibility
     * reasons. Override in sub-classes that support distinction between
     * empty/non-empty state.
     *
     * \return True if the data object is empty at all time steps.
     * \sa IsEmptyTimeStep
     */
    virtual bool IsEmpty() const;

    /**
     * \brief Set the requested region from this data object to match the requested
     * region of the data object passed in as a parameter.
     *
     * This method is implemented in the concrete subclasses of BaseData.
     *
     * \param[in] data The data object whose requested region will be matched.
     */
    void SetRequestedRegion(const itk::DataObject *data) override = 0;

    /**
     * \brief Execute an operation on this data object.
     *
     * Override this method if the data can be called by an Interactor
     * (StateMachine). Empty by default. Subclasses should implement all
     * necessary operations here, extracting needed information from the
     * \a operation parameter.
     *
     * \param[in] operation The operation to execute.
     * \sa OperationActor
     */
    void ExecuteOperation(Operation *operation) override;

    /**
     * \brief Set the BaseGeometry of the data, which will be referenced (not copied!).
     *
     * Assumes the data object has only 1 time step (i.e., is a 3D object) and creates
     * a new ProportionalTimeGeometry wrapping the given BaseGeometry. If a TimeGeometry
     * has already been set, it will be replaced.
     *
     * \warning This method will normally be called internally by sub-classes of BaseData
     * during initialization.
     *
     * \param[in] aGeometry3D The geometry to reference. If nullptr, an empty TimeGeometry
     *            is created.
     * \sa SetClonedGeometry, SetTimeGeometry
     */
    virtual void SetGeometry(BaseGeometry *aGeometry3D);

    /**
     * \brief Set the TimeGeometry of the data, which will be referenced (not copied!).
     *
     * \warning This method will normally be called internally by sub-classes of BaseData
     * during initialization.
     *
     * \param[in] geometry The TimeGeometry to reference.
     * \sa SetClonedTimeGeometry
     */
    virtual void SetTimeGeometry(TimeGeometry *geometry);

    /**
     * \brief Set a clone of the provided geometry as the BaseGeometry of the data.
     *
     * Assumes the data object has only 1 time step (i.e., is a 3D object) and creates
     * a new TimeGeometry. If a TimeGeometry has already been set, it will be replaced.
     *
     * \param[in] aGeometry3D The geometry to clone.
     * \sa SetGeometry
     */
    virtual void SetClonedGeometry(const BaseGeometry *aGeometry3D);

    /**
     * \brief Set a clone of the provided TimeGeometry as the TimeGeometry of the data.
     *
     * \param[in] geometry The TimeGeometry to clone.
     * \sa SetTimeGeometry
     */
    virtual void SetClonedTimeGeometry(const TimeGeometry *geometry);

    /**
     * \brief Set a clone of the provided geometry as the BaseGeometry of a given time step.
     *
     * \param[in] aGeometry3D The geometry to clone.
     * \param[in] time The time step at which to set the cloned geometry.
     * \pre The TimeGeometry must already be set.
     * \sa SetGeometry
     */
    virtual void SetClonedGeometry(const BaseGeometry *aGeometry3D, unsigned int time);

    /**
     * \brief Get the data's property list.
     *
     * \return Smart pointer to the PropertyList.
     * \sa SetPropertyList, GetProperty, SetProperty
     */
    mitk::PropertyList::Pointer GetPropertyList() const;

    /**
     * \brief Set the data's property list, replacing the existing one.
     *
     * \param[in] propertyList The new property list.
     * \sa GetPropertyList
     */
    void SetPropertyList(PropertyList *propertyList);

    /**
     * \brief Get a property by its key from the PropertyList.
     *
     * \param[in] propertyKey The key of the property to look up.
     * \return Smart pointer to the property, or nullptr if no property with the
     *         given key exists.
     * \sa SetProperty, GetPropertyList
     */
    mitk::BaseProperty::Pointer GetProperty(const char *propertyKey) const;

    /**
     * \brief Set a property in the PropertyList.
     *
     * \param[in] propertyKey The key under which to store the property.
     * \param[in] property The property value to store.
     * \sa GetProperty, GetPropertyList
     */
    void SetProperty(const char *propertyKey, BaseProperty *property);

    /**
     * \brief Convenience method for setting the origin of the BaseGeometry
     * instances of all time steps.
     *
     * \warning Sub-geometries contained in the BaseGeometry will \em not be
     * changed. For example, if the BaseGeometry is a SlicedGeometry3D, the
     * origin will \em not be propagated to the contained slices. The sub-class
     * SlicedData handles this for the case that the SlicedGeometry3D is
     * evenly spaced.
     *
     * \param[in] origin The new origin in world coordinates.
     * \sa SlicedData::SetOrigin
     */
    virtual void SetOrigin(const Point3D &origin);

    /**
     * \brief Get the process object that generated this data object.
     *
     * If there is no process object, then the data object has been
     * disconnected from the pipeline, or the data object was created manually.
     *
     * \return A smart pointer to the source BaseDataSource, or nullptr if no
     *         source is set.
     *
     * \note A SmartPointer is returned (not a WeakPointer) because it is
     * assumed the caller wants to hold a long-term reference to the source.
     */
    itk::SmartPointer<mitk::BaseDataSource> GetSource() const;

    /**
     * \brief Get the number of time steps from the TimeGeometry.
     *
     * As the base data does not have a data vector of its own, the number
     * of time steps is defined by the TimeGeometry. Sub-classes may provide
     * a more efficient implementation based on their data vector length.
     *
     * \return The number of time steps.
     */
    unsigned int GetTimeSteps() const { return m_TimeGeometry->CountTimeSteps(); }

    /**
     * \brief Get the modification time of this data object or its geometry,
     * whichever was modified most recently.
     *
     * \return The most recent modification time.
     */
    itk::ModifiedTimeType GetMTime() const override;

    /**
     * \brief Graft data and information from another data object.
     *
     * \throw itk::ExceptionObject Always throws in the base class. Subclasses
     *        must provide their own implementation.
     * \sa itk::ProcessObject::Graft
     */
    void Graft(const DataObject *) override;

  protected:
    BaseData();
    BaseData(const BaseData &other);
    ~BaseData() override;

    //##Documentation
    //## \brief Initialize the TimeGeometry for a number of time steps.
    //## The TimeGeometry is initialized empty and evenly timed.
    //## In many cases it will be necessary to overwrite this in sub-classes.
    virtual void InitializeTimeGeometry(unsigned int timeSteps = 1);

    //##Documentation
    //## @brief reset to non-initialized state, release memory
    virtual void ClearData();

    //##Documentation
    //## @brief Pure virtual; Must be used in subclasses to get a data object to a
    //## valid state. Should at least create one empty object and call
    //## Superclass::InitializeTimeGeometry() to ensure an existing valid geometry
    virtual void InitializeEmpty() {}
    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    bool m_LastRequestedRegionWasOutsideOfTheBufferedRegion;

    mutable unsigned int m_SourceOutputIndexDuplicate;

    bool m_Initialized;

  private:
    //##Documentation
    //## @brief PropertyList, f.e. to hold pic-tags, tracking-data,..
    //##
    PropertyList::Pointer m_PropertyList;

    TimeGeometry::Pointer m_TimeGeometry;
  };

} // namespace mitk

#endif
