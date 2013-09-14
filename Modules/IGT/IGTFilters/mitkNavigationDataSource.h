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


#ifndef MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_

#include <itkProcessObject.h>
#include "mitkNavigationData.h"
#include "mitkPropertyList.h"

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>

namespace mitk {

  /**Documentation
  * \brief Navigation Data source
  *
  * Base class for all navigation filters that produce NavigationData objects as output.
  * This class defines the output-interface for NavigationDataFilters.
  * \warning: if Update() is called on any output object, all NavigationData filters will
  * generate new output data for all outputs, not just the one on which Update() was called.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataSource : public itk::ProcessObject
  {
  public:
    mitkClassMacro(NavigationDataSource, itk::ProcessObject);

    /** @return Returns a human readable name of this source. There will be a default name,
      *         or you can set the name with the method SetName() if you want to change it.
      */
    itkGetMacro(Name,std::string);

    /** @brief Sets the human readable name of this source. There is also a default name,
      *        but you can use this method if you need to define it on your own.
      */
    itkSetMacro(Name,std::string);

    /**
    *\brief return the output (output with id 0) of the filter
    */
    NavigationData* GetOutput(void);

    /**
    *\brief return the output with id idx of the filter
    */
    NavigationData* GetOutput(DataObjectPointerArraySizeType idx);

    /**
    *\brief return the output with name navDataName of the filter
    */
    NavigationData* GetOutput(const std::string& navDataName);

    /**
    *\brief return the index of the output with name navDataName, -1 if no output with that name was found
    *
    * \warning if a subclass has outputs that have different data type than mitk::NavigationData, they have to overwrite this method
    */
    DataObjectPointerArraySizeType GetOutputIndex(std::string navDataName);

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    * To unregister, call UnregisterMicroservice().
    */
    virtual void RegisterAsMicroservice();

    /**
    *\brief Registers this object as a Microservice, making it available to every module and/or plugin.
    */
    virtual void UnRegisterMicroservice();

    /**
    *\brief Returns the id that this device is registered with. The id will only be valid, if the
    * NavigationDataSource has been registered using RegisterAsMicroservice().
    */
    std::string GetMicroserviceID();

    /**
    *\brief These Constants are used in conjunction with Microservices
    */
    static const std::string US_INTERFACE_NAME;
    static const std::string US_PROPKEY_DEVICENAME;
    static const std::string US_PROPKEY_ID;
    static const std::string US_PROPKEY_ISACTIVE; //NOT IMPLEMENTED YET!

    /**
    *\brief Graft the specified DataObject onto this ProcessObject's output.
    *
    * See itk::ImageSource::GraftNthOutput for details
    */
    virtual void GraftNthOutput(unsigned int idx, itk::DataObject *graft);

    /**
    * \brief Graft the specified DataObject onto this ProcessObject's output.
    *
    * See itk::ImageSource::Graft Output for details
    */
    virtual void GraftOutput(itk::DataObject *graft);

    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx );

    /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appopriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name);

    /**
    * \brief Set all filter parameters as the PropertyList p
    *
    * This method allows to set all parameters of a filter with one
    * method call. For the names of the parameters, take a look at
    * the GetParameters method of the filter
    * This method has to be overwritten by each MITK-IGT filter.
    */
    virtual void SetParameters(const mitk::PropertyList*){};

    /**
    * \brief Get all filter parameters as a PropertyList
    *
    * This method allows to get all parameters of a filter with one
    * method call. The returned PropertyList must be assigned to a
    * SmartPointer immediately, or else it will get destroyed.
    * Every filter must overwrite this method to create a filter-specific
    * PropertyList. Note that property names must be unique over all
    * MITK-IGT filters. Therefore each filter should use its name as a prefix
    * for each property name.
    * Secondly, each filter should list the property names and data types
    * in the method documentation.
    */
    virtual mitk::PropertyList::ConstPointer GetParameters() const;

  protected:
    NavigationDataSource();
    virtual ~NavigationDataSource();

    std::string m_Name;


  private:
    us::ServiceRegistration<Self> m_ServiceRegistration;
  };
} // namespace mitk
// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::NavigationDataSource, "org.mitk.services.NavigationDataSource")
#endif /* MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_ */
