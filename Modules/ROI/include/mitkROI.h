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
  class MITKROI_EXPORT ROI : public BaseData
  {
  public:
    struct MITKROI_EXPORT Element : IPropertyOwner
    {
      Element();
      ~Element() = default;

      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) const override;
      std::vector<std::string> GetPropertyKeys(const std::string& contextName = "", bool includeDefaultContext = false) const override;
      std::vector<std::string> GetPropertyContextNames() const override;

      BaseProperty* GetNonConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) override;
      void SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;
      void RemoveProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;

      unsigned int ID;
      Point3D Min;
      Point3D Max;
      PropertyList::Pointer Properties;
    };

    mitkClassMacro(ROI, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    using ElementsType = std::vector<Element>;
    using Iterator = ElementsType::iterator;
    using ConstIterator = ElementsType::const_iterator;

    size_t GetNumberOfElements() const;
    size_t AddElement(const Element& element);

    const Element* GetElement(size_t index) const;
    Element* GetElement(size_t index);

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
}

#endif
