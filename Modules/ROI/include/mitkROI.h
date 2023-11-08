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
    class MITKROI_EXPORT Element : public IPropertyOwner
    {
    public:
      using PointsType = std::map<TimeStepType, Point3D>;
      using PropertyListsType = std::map<TimeStepType, PropertyList::Pointer>;

      Element();
      explicit Element(unsigned int id);
      ~Element() = default;

      BaseProperty::ConstPointer GetConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) const override;
      std::vector<std::string> GetPropertyKeys(const std::string& contextName = "", bool includeDefaultContext = false) const override;
      std::vector<std::string> GetPropertyContextNames() const override;

      BaseProperty* GetNonConstProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = true) override;
      void SetProperty(const std::string& propertyKey, BaseProperty* property, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;
      void RemoveProperty(const std::string& propertyKey, const std::string& contextName = "", bool fallBackOnDefaultContext = false) override;

      unsigned int GetID() const;
      void SetID(unsigned int id);

      bool HasTimeStep(TimeStepType t) const;
      bool HasTimeSteps() const;
      std::vector<TimeStepType> GetTimeSteps() const;

      Point3D GetMin(TimeStepType t = 0) const;
      void SetMin(const Point3D& min, TimeStepType t = 0);

      Point3D GetMax(TimeStepType t = 0) const;
      void SetMax(const Point3D& max, TimeStepType t = 0);

      PropertyList* GetDefaultProperties() const;
      void SetDefaultProperties(PropertyList* properties);

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
