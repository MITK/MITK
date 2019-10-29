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

#include "mitkSourceImageRelationRule.h"

#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "mitkStringProperty.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkPropertyNameHelper.h"
#include "mitkTemporoSpatialStringProperty.h"
#include "mitkPropertyNameHelper.h"

#include <regex>

class mitkSourceImageRelationRuleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSourceImageRelationRuleTestSuite);

  MITK_TEST(IsSourceCandidate);
  MITK_TEST(IsDestinationCandidate);
  MITK_TEST(IsSource);
  MITK_TEST(HasRelation);
  MITK_TEST(GetExistingRelations);
  MITK_TEST(GetRelationUIDs);
  MITK_TEST(GetSourceCandidateIndicator);
  MITK_TEST(GetDestinationCandidateIndicator);
  MITK_TEST(GetConnectedSourcesDetector);
  MITK_TEST(GetSourcesDetector);
  MITK_TEST(GetDestinationsDetector);
  MITK_TEST(GetDestinationDetector);
  MITK_TEST(Connect);
  MITK_TEST(Disconnect);
  MITK_TEST(Connect_abstract);
  MITK_TEST(Disconnect_abstract);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::SourceImageRelationRule::Pointer rule;
  mitk::SourceImageRelationRule::Pointer abstractRule;

  mitk::Image::Pointer unRelated;
  mitk::DataNode::Pointer unRelated_Node;

  mitk::Image::Pointer source_implicit_1;
  mitk::DataNode::Pointer source_implicit_1_Node;
  mitk::Image::Pointer source_data_1;
  mitk::DataNode::Pointer source_data_1_Node;
  mitk::Image::Pointer source_idOnly_1;
  mitk::DataNode::Pointer source_idOnly_1_Node;
  mitk::Image::Pointer source_1;
  mitk::DataNode::Pointer source_1_Node;

  mitk::Image::Pointer source_otherRule;
  mitk::DataNode::Pointer source_otherRule_Node;

  mitk::Image::Pointer source_otherPurpose;
  mitk::DataNode::Pointer source_otherPurpose_Node; //relevant for abstract rule checks. Abstract rule should see it concrete rule not.

  mitk::DataNode::Pointer dest_1_Node;
  mitk::Image::Pointer dest_1;
  mitk::DataNode::Pointer dest_2_Node;
  mitk::Image::Pointer dest_2;

  bool hasRelationProperties(mitk::IPropertyProvider *provider, std::string instance = "") const
  {
    auto keyPath = mitk::PropertyRelationRuleBase::GetRootKeyPath();
    if (!instance.empty())
    {
      keyPath.AddElement(instance);
    }

    auto prefix = mitk::PropertyKeyPathToPropertyName(keyPath);
    auto keys = provider->GetPropertyKeys();

    for (const auto &key : keys)
    {
      if (key.find(prefix) == 0)
      {
        return true;
      }
    }

    return false;
  }

  std::vector<std::string> GetReferenceSequenceIndices(const mitk::IPropertyProvider * source,
    const mitk::IPropertyProvider * destination) const
  {
    std::vector<std::string> result;

    auto destInstanceUIDProp = destination->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0018));

    if (destInstanceUIDProp.IsNull())
    {
      return result;
    }

    mitk::PropertyKeyPath referencedInstanceUIDs;
    referencedInstanceUIDs.AddElement("DICOM").AddElement("0008").AddAnySelection("2112").AddElement("0008").AddElement("1155");

    auto sourceRegExStr = PropertyKeyPathToPropertyRegEx(referencedInstanceUIDs);;
    auto regEx = std::regex(sourceRegExStr);

    std::vector<std::string> keys;
    //workaround until T24729 is done. Please remove if T24728 is done
    keys = source->GetPropertyKeys();
    //end workaround for T24729

    for (const auto &key : keys)
    {
      if (std::regex_match(key, regEx))
      {
        auto refUIDProp = source->GetConstProperty(key);
        if (*refUIDProp == *destInstanceUIDProp)
        {
          mitk::PropertyKeyPath finding = mitk::PropertyNameToPropertyKeyPath(key);
          result.push_back(std::to_string(finding.GetNode(2).selection));
        }
      }
    }

    return result;
  };

  void SetDICOMReferenceInfo(mitk::IPropertyOwner* owner, const std::string& instanceUID, const std::string& classUID, const std::string& purpose, unsigned int sequElement)
  {
    mitk::PropertyKeyPath refInstanceUIDPath;
    refInstanceUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0008").AddElement("1155");
    owner->SetProperty(PropertyKeyPathToPropertyName(refInstanceUIDPath), mitk::TemporoSpatialStringProperty::New(instanceUID));

    mitk::PropertyKeyPath refClassUIDPath;
    refClassUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0008").AddElement("1150");
    owner->SetProperty(PropertyKeyPathToPropertyName(refClassUIDPath), mitk::TemporoSpatialStringProperty::New(classUID));

    mitk::PropertyKeyPath purposePath;
    purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");
    owner->SetProperty(PropertyKeyPathToPropertyName(purposePath), mitk::TemporoSpatialStringProperty::New(purpose));
  }

  bool IsCorrectDICOMReference(const mitk::IPropertyOwner* owner, const std::string& instanceUID, const std::string& classUID, const std::string& purpose, unsigned int sequElement) const
  {
    mitk::PropertyKeyPath refInstanceUIDPath;
    refInstanceUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0008").AddElement("1155");
    auto prop = owner->GetConstProperty(PropertyKeyPathToPropertyName(refInstanceUIDPath));
    if (prop->GetValueAsString() != instanceUID)
    {
      return false;
    }

    mitk::PropertyKeyPath refClassUIDPath;
    refClassUIDPath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0008").AddElement("1150");
    prop = owner->GetConstProperty(PropertyKeyPathToPropertyName(refClassUIDPath));
    if (prop->GetValueAsString() != classUID)
    {
      return false;
    }

    mitk::PropertyKeyPath purposePath;
    purposePath.AddElement("DICOM").AddElement("0008").AddSelection("2112", sequElement).AddElement("0040").AddSelection("a170", 0).AddElement("0008").AddElement("0104");
    prop = owner->GetConstProperty(PropertyKeyPathToPropertyName(purposePath));
    if (prop->GetValueAsString() != purpose)
    {
      return false;
    }

    return true;
  }


public:
  void setUp() override
  {
    auto instanceUIDPropName = mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0018);
    auto classUIDPropName = mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0016);

    rule = mitk::SourceImageRelationRule::New("Test");

    abstractRule = mitk::SourceImageRelationRule::New();

    unRelated = mitk::Image::New();
    unRelated->SetProperty(instanceUIDPropName, mitk::TemporoSpatialStringProperty::New("unRelated"));
    unRelated->SetProperty(classUIDPropName, mitk::TemporoSpatialStringProperty::New("image"));
    unRelated_Node = mitk::DataNode::New();
    unRelated_Node->SetData(unRelated);

    dest_1_Node = mitk::DataNode::New();
    dest_1_Node->SetName("dest_1");
    dest_1 = mitk::Image::New();
    dest_1->SetProperty(instanceUIDPropName, mitk::TemporoSpatialStringProperty::New("dest_1"));
    dest_1->SetProperty(classUIDPropName, mitk::TemporoSpatialStringProperty::New("image"));
    dest_1_Node->SetData(dest_1);



    dest_2_Node = mitk::DataNode::New();
    dest_2_Node->SetName("dest_2");
    dest_2 = mitk::Image::New();
    dest_2->SetProperty(instanceUIDPropName, mitk::TemporoSpatialStringProperty::New("dest_2"));
    dest_2->SetProperty(classUIDPropName, mitk::TemporoSpatialStringProperty::New("image"));
    dest_2_Node->SetData(dest_2);

    source_implicit_1 = mitk::Image::New();
    SetDICOMReferenceInfo(source_implicit_1, "dest_1", "image", "Test", 0);
    source_implicit_1_Node = mitk::DataNode::New();
    source_implicit_1_Node->SetData(source_implicit_1);

    source_idOnly_1 = mitk::Image::New();
    std::string name = "MITK.Relations.1.relationUID";
    source_idOnly_1->SetProperty(name.c_str(), mitk::StringProperty::New("uid1"));
    name = "MITK.Relations.1.destinationUID";
    source_idOnly_1->SetProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_idOnly_1->SetProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    source_idOnly_1_Node = mitk::DataNode::New();
    source_idOnly_1_Node->SetData(source_idOnly_1);

    source_data_1 = mitk::Image::New();
    SetDICOMReferenceInfo(source_data_1, "dest_1", "image", "Test", 0);
    SetDICOMReferenceInfo(source_data_1, "dest_2", "image", "otherpurpose", 1);
    name = "MITK.Relations.1.relationUID";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New("uid2"));
    name = "MITK.Relations.1.ruleID";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.1.SourceImageSequenceItem";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New("0"));
    name = "MITK.Relations.2.relationUID";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New("uid10"));
    name = "MITK.Relations.2.SourceImageSequenceItem";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New("1"));
    name = "MITK.Relations.2.ruleID";
    source_data_1->SetProperty(name.c_str(), mitk::StringProperty::New("SourceImageRelation otherpurpose"));
    source_data_1_Node = mitk::DataNode::New();
    source_data_1_Node->SetData(source_data_1);

    source_1 = mitk::Image::New();
    SetDICOMReferenceInfo(source_1, "dest_1", "image", "Test", 0);
    SetDICOMReferenceInfo(source_1, "dest_2", "image", "otherpurpose", 1);
    name = "MITK.Relations.1.relationUID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New("uid3"));
    name = "MITK.Relations.1.destinationUID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New(rule->GetRuleID()));
    name = "MITK.Relations.1.SourceImageSequenceItem";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New("0"));
    name = "MITK.Relations.2.relationUID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New("uid8"));
    name = "MITK.Relations.2.destinationUID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New(dest_2->GetUID()));
    name = "MITK.Relations.2.ruleID";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New("SourceImageRelation otherpurpose"));
    name = "MITK.Relations.2.SourceImageSequenceItem";
    source_1->SetProperty(name.c_str(), mitk::StringProperty::New("1"));
    source_1_Node = mitk::DataNode::New();
    source_1_Node->SetData(source_1);

    source_otherRule = mitk::Image::New();
    name = "MITK.Relations.1.relationUID";
    source_otherRule->SetProperty(name.c_str(), mitk::StringProperty::New("uid7"));
    name = "MITK.Relations.1.destinationUID";
    source_otherRule->SetProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_otherRule->SetProperty(name.c_str(), mitk::StringProperty::New("otherRuleID"));
    source_otherRule_Node = mitk::DataNode::New();
    source_otherRule_Node->SetData(source_otherRule);

    source_otherPurpose = mitk::Image::New();
    name = "MITK.Relations.1.relationUID";
    source_otherPurpose->SetProperty(name.c_str(), mitk::StringProperty::New("uid9"));
    name = "MITK.Relations.1.destinationUID";
    source_otherPurpose->SetProperty(name.c_str(), mitk::StringProperty::New(dest_1->GetUID()));
    name = "MITK.Relations.1.ruleID";
    source_otherPurpose->SetProperty(name.c_str(), mitk::StringProperty::New("SourceImageRelation otherpurpose"));
    source_otherPurpose_Node = mitk::DataNode::New();
    source_otherPurpose_Node->SetData(source_otherPurpose);
  }

  void tearDown() override {}

  void IsSourceCandidate()
  {
    CPPUNIT_ASSERT(rule->IsSourceCandidate(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!rule->IsSourceCandidate(nullptr));
  }

  void IsDestinationCandidate()
  {
    CPPUNIT_ASSERT(rule->IsDestinationCandidate(this->dest_1_Node));
    CPPUNIT_ASSERT(rule->IsDestinationCandidate(this->dest_1));
    CPPUNIT_ASSERT(!rule->IsDestinationCandidate(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!rule->IsDestinationCandidate(nullptr));
  }

  void IsSource()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE(
      "Violated precondition (nullptr) does not throw.", rule->IsSource(nullptr), itk::ExceptionObject);

    CPPUNIT_ASSERT(!rule->IsSource(unRelated));
    CPPUNIT_ASSERT(!rule->IsSource(source_implicit_1));
    CPPUNIT_ASSERT(rule->IsSource(source_data_1));
    CPPUNIT_ASSERT(rule->IsSource(source_idOnly_1));
    CPPUNIT_ASSERT(rule->IsSource(source_1));

    CPPUNIT_ASSERT(!rule->IsSource(source_otherRule));
    CPPUNIT_ASSERT(!rule->IsSource(source_otherPurpose));

    CPPUNIT_ASSERT(!rule->IsSource(source_implicit_1_Node));
    CPPUNIT_ASSERT(rule->IsSource(source_data_1_Node));
    CPPUNIT_ASSERT(rule->IsSource(source_idOnly_1_Node));
    CPPUNIT_ASSERT(rule->IsSource(source_1_Node));

    CPPUNIT_ASSERT(!rule->IsSource(source_otherRule_Node));
    CPPUNIT_ASSERT(!rule->IsSource(source_otherPurpose_Node));


    CPPUNIT_ASSERT(!abstractRule->IsSource(unRelated));
    CPPUNIT_ASSERT(!abstractRule->IsSource(source_implicit_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_data_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_idOnly_1));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_1));

    CPPUNIT_ASSERT(!abstractRule->IsSource(source_otherRule));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_otherPurpose));

    CPPUNIT_ASSERT(!abstractRule->IsSource(unRelated_Node));
    CPPUNIT_ASSERT(!abstractRule->IsSource(source_implicit_1_Node));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_data_1_Node));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_idOnly_1_Node));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_1_Node));

    CPPUNIT_ASSERT(!abstractRule->IsSource(source_otherRule_Node));
    CPPUNIT_ASSERT(abstractRule->IsSource(source_otherPurpose_Node));
  }

  void HasRelation()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->HasRelation(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->HasRelation(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(unRelated, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(source_otherRule, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(rule->HasRelation(source_otherPurpose, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);

    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(rule->HasRelation(source_idOnly_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);


    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(unRelated, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_otherRule, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_otherPurpose, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_implicit_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_data_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_idOnly_1, dest_1) ==
      mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
  }

  void GetExistingRelations()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetExistingRelations(nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->GetExistingRelations(unRelated).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_otherRule).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_otherPurpose).empty());
    CPPUNIT_ASSERT(rule->GetExistingRelations(source_implicit_1).empty());

    auto uids = rule->GetExistingRelations(source_idOnly_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid1");

    uids = rule->GetExistingRelations(source_data_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid2");

    uids = rule->GetExistingRelations(source_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid3");


    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(unRelated).empty());
    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(source_otherRule).empty());
    CPPUNIT_ASSERT(abstractRule->GetExistingRelations(source_implicit_1).empty());

    uids = abstractRule->GetExistingRelations(source_idOnly_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid1");

    uids = abstractRule->GetExistingRelations(source_data_1);
    CPPUNIT_ASSERT(uids.size() == 2);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid2") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid10") != uids.end());

    uids = abstractRule->GetExistingRelations(source_1);
    CPPUNIT_ASSERT(uids.size() == 2);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid3") != uids.end());
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid8") != uids.end());

    uids = abstractRule->GetExistingRelations(source_otherPurpose);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid9");
  }

  void GetRelationUIDs()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetRelationUIDs(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetRelationUIDs(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_1, unRelated).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_1, dest_2).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(unRelated, dest_1).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_otherRule, dest_1).empty());
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_otherPurpose, dest_1).empty());

    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_idOnly_1, dest_1).front() == "uid1");
    CPPUNIT_ASSERT(rule->GetRelationUIDs(source_data_1, dest_1).front() == "uid2");
    auto uids = rule->GetRelationUIDs(source_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(uids.front() == "uid3");

    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_1, unRelated).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(unRelated, dest_1).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_otherRule, dest_1).empty());
    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_otherPurpose, dest_1).front() == "uid9");

    CPPUNIT_ASSERT(abstractRule->GetRelationUIDs(source_idOnly_1, dest_1).front() == "uid1");
    uids = abstractRule->GetRelationUIDs(source_data_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid2") != uids.end());
    uids = abstractRule->GetRelationUIDs(source_1, dest_1);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid3") != uids.end());
    uids = abstractRule->GetRelationUIDs(source_1, dest_2);
    CPPUNIT_ASSERT(uids.size() == 1);
    CPPUNIT_ASSERT(std::find(uids.begin(), uids.end(), "uid8") != uids.end());
  }

  void GetSourceCandidateIndicator()
  {
    auto predicate = rule->GetSourceCandidateIndicator();

    CPPUNIT_ASSERT(predicate->CheckNode(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
  }

  void GetDestinationCandidateIndicator()
  {
    auto predicate = rule->GetDestinationCandidateIndicator();

    CPPUNIT_ASSERT(predicate->CheckNode(this->dest_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(mitk::DataNode::New()));
    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
  }

  void GetConnectedSourcesDetector()
  {
    auto predicate = rule->GetConnectedSourcesDetector();

    CPPUNIT_ASSERT(!predicate->CheckNode(nullptr));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherPurpose_Node));


    auto predicate2 = abstractRule->GetConnectedSourcesDetector();

    CPPUNIT_ASSERT(!predicate2->CheckNode(nullptr));
    CPPUNIT_ASSERT(!predicate2->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate2->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_1_Node));

    CPPUNIT_ASSERT(!predicate2->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(predicate2->CheckNode(source_otherPurpose_Node));
  }

  void GetSourcesDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->GetSourcesDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetSourcesDetector(dest_1);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));

    predicate = rule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));

    predicate = rule->GetSourcesDetector(dest_2, mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_1_Node));

    predicate = abstractRule->GetSourcesDetector(dest_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(!predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));

    predicate = abstractRule->GetSourcesDetector(dest_1);

    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(source_otherRule_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_otherPurpose_Node));

    CPPUNIT_ASSERT(predicate->CheckNode(source_implicit_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_data_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_idOnly_1_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(source_1_Node));
  }

  void GetDestinationsDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetDestinationsDetector(nullptr),
                                 itk::ExceptionObject);

    auto predicate = rule->GetDestinationsDetector(source_otherRule);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1_Node));

    predicate = rule->GetDestinationsDetector(source_otherPurpose);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1_Node));

    predicate = rule->GetDestinationsDetector(source_implicit_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate =
      rule->GetDestinationsDetector(source_implicit_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1_Node));

    predicate = rule->GetDestinationsDetector(source_data_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate =
      rule->GetDestinationsDetector(source_data_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate =
      rule->GetDestinationsDetector(source_data_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_1_Node));

    predicate = rule->GetDestinationsDetector(source_idOnly_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate =
      rule->GetDestinationsDetector(source_idOnly_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate =
      rule->GetDestinationsDetector(source_idOnly_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));

    predicate = rule->GetDestinationsDetector(source_1);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2_Node));
    predicate =
      rule->GetDestinationsDetector(source_1, mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2_Node));
    predicate =
      rule->GetDestinationsDetector(source_1, mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2_Node));

    predicate = abstractRule->GetDestinationsDetector(source_otherPurpose);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    predicate = abstractRule->GetDestinationsDetector(source_otherPurpose);
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
  }

  void GetDestinationDetector()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->GetDestinationDetector(nullptr, "uid1"),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (relation uid is invalid) does not throw.",
                                 rule->GetDestinationDetector(source_1, "invalid uid"),
                                 itk::ExceptionObject);

    auto predicate = rule->GetDestinationDetector(source_1, "uid3");
    CPPUNIT_ASSERT(!predicate->CheckNode(unRelated_Node));
    CPPUNIT_ASSERT(predicate->CheckNode(dest_1_Node));
    CPPUNIT_ASSERT(!predicate->CheckNode(dest_2_Node));
  }

  void Connect()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->Connect(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Connect(source_1, nullptr),
                                 itk::ExceptionObject);

    // check upgrade of an implicit connection
    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Implicit_Data);
    rule->Connect(source_implicit_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_implicit_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    auto dcmRefs = GetReferenceSequenceIndices(source_implicit_1, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Additional dicom reference was defined instead of using the existing one.", dcmRefs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(source_implicit_1, "dest_1", "image", "Test", 0));

    // check upgrade and reuse of an data connection (no new relation should be generated).
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_Data);
    rule->Connect(source_data_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_data_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);

    auto relUID = rule->GetRelationUIDs(source_data_1, dest_1);
    CPPUNIT_ASSERT(relUID.size() == 1);

    std::string name = "MITK.Relations.1.destinationUID";
    auto prop = source_data_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == dest_1->GetUID());
    
    name = "MITK.Relations.1.ruleID";
    prop = source_data_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect ruleID was stored.", prop->GetValueAsString() == rule->GetRuleID());

    name = "MITK.Relations.1.SourceImageSequenceItem";
    prop = source_data_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == "0");

    dcmRefs = GetReferenceSequenceIndices(source_data_1, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Additional dicom reference was defined instead of using the existing one.", dcmRefs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(source_data_1, "dest_1", "image", "Test", 0));

    // check actualization of an id only connection
    rule->Connect(source_idOnly_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_idOnly_1, dest_1) ==
                   mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);

    // check actualization of an existing connection
    rule->Connect(source_1, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was defined instead of updating exting one.",
                           rule->GetExistingRelations(source_1).size() == 1);
    name = "MITK.Relations.1.destinationUID";
    prop = source_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == dest_1->GetUID());

    name = "MITK.Relations.1.ruleID";
    prop = source_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect ruleID was stored.", prop->GetValueAsString() == rule->GetRuleID());

    name = "MITK.Relations.1.SourceImageSequenceItem";
    prop = source_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == "0");

    dcmRefs = GetReferenceSequenceIndices(source_1, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Additional dicom reference was defined instead of using the existing one.", dcmRefs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(source_1, "dest_1", "image", "Test", 0));

    // check creation of an new connection
    rule->Connect(unRelated, dest_1);
    CPPUNIT_ASSERT(rule->HasRelation(unRelated, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Relation was not defined instead of updating exting one.",
      rule->GetExistingRelations(unRelated).size() == 1);
    name = "MITK.Relations.1.destinationUID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == dest_1->GetUID());

    name = "MITK.Relations.1.ruleID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect ruleID was stored.", prop->GetValueAsString() == rule->GetRuleID());

    name = "MITK.Relations.1.SourceImageSequenceItem";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == "0");

    dcmRefs = GetReferenceSequenceIndices(unRelated, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Additional dicom reference was defined instead of using the existing one.", dcmRefs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Dicom reference squence is corrupted. Should be just an index 0.", dcmRefs[0] == "0");
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(unRelated, "dest_1", "image", "Test", 0));

    // check creation of a 2nd connection of the same purpose
    rule->Connect(unRelated, dest_2);
    CPPUNIT_ASSERT(rule->HasRelation(unRelated, dest_2) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    CPPUNIT_ASSERT_MESSAGE("Additional relation was not defined.",
      rule->GetExistingRelations(unRelated).size() == 2);
    name = "MITK.Relations.1.destinationUID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == dest_1->GetUID());

    name = "MITK.Relations.1.ruleID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect ruleID was stored.", prop->GetValueAsString() == rule->GetRuleID());

    name = "MITK.Relations.1.SourceImageSequenceItem";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == "0");

    name = "MITK.Relations.2.destinationUID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == dest_2->GetUID());

    name = "MITK.Relations.2.ruleID";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect ruleID was stored.", prop->GetValueAsString() == rule->GetRuleID());

    name = "MITK.Relations.2.SourceImageSequenceItem";
    prop = unRelated->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == "1");


    dcmRefs = GetReferenceSequenceIndices(unRelated, dest_2);
    CPPUNIT_ASSERT_MESSAGE("Additional dicom reference was not defined.", dcmRefs.size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(unRelated, "dest_1", "image", "Test", 0));
    CPPUNIT_ASSERT_MESSAGE("Dicom reference is not correct.", IsCorrectDICOMReference(unRelated, "dest_2", "image", "Test", 1));
  }

  void Disconnect()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (source is nullptr) does not throw.",
                                 rule->Disconnect(nullptr, dest_1),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Disconnect(source_1, nullptr),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (destination is nullptr) does not throw.",
                                 rule->Disconnect(nullptr, "uid"),
                                 itk::ExceptionObject);

    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    rule->Disconnect(source_1, unRelated);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, unRelated) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT_MESSAGE("Other relationdata property was removed.", IsCorrectDICOMReference(source_1, "dest_1", "image", "Test", 0));

    //check if index correction is correct, when disconnecting
    rule->Connect(source_1, dest_2);
    rule->Connect(source_1, unRelated);
    rule->Disconnect(source_1, dest_2);
    CPPUNIT_ASSERT(rule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(this->hasRelationProperties(source_1, "1"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_1, "2"));
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "3"));
    CPPUNIT_ASSERT(this->hasRelationProperties(source_1, "4"));
    CPPUNIT_ASSERT_MESSAGE("Dicom reference to dest_1 has been removed.", IsCorrectDICOMReference(source_1, "dest_1", "image", "Test", 0));
    CPPUNIT_ASSERT_MESSAGE("Dicom reference to dest_2 (other purpose) has been removed or has not a corrected sequence index (1 instead of 2).", IsCorrectDICOMReference(source_1, "dest_2", "image", "otherpurpose", 1));
    CPPUNIT_ASSERT_MESSAGE("Dicom reference to unRelated has been removed or has not a corrected sequence index (1 instead of 2).", IsCorrectDICOMReference(source_1, "unRelated", "image", "Test", 2));

    std::string name = "MITK.Relations.4.destinationUID";
    auto prop = source_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE(
      "Destination uid was not stored with the correct key. Already existing session should be used.", prop);
    CPPUNIT_ASSERT_MESSAGE("Incorrect destination uid was stored.", prop->GetValueAsString() == unRelated->GetUID());
    name = "MITK.Relations.4.SourceImageSequenceItem";
    prop = source_1->GetProperty(name.c_str());
    CPPUNIT_ASSERT_MESSAGE("SourceImageSequenceItem was not actualized correctly.", prop->GetValueAsString() == "2");

    rule->Disconnect(source_otherPurpose, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Data of other rule purpose was removed.", this->hasRelationProperties(source_otherPurpose, "1"));
  }

  void Connect_abstract()
  {
    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (abstract does not connect) does not throw.",
      abstractRule->Connect(nullptr, dest_1),
      itk::ExceptionObject);

    CPPUNIT_ASSERT_THROW_MESSAGE("Violated precondition (abstract does not connect) does not throw.",
      abstractRule->Connect(source_1, nullptr),
      itk::ExceptionObject);
  }

  void Disconnect_abstract()
  {

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    abstractRule->Disconnect(source_1, dest_2);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_2) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "2"));

    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::Connected_ID);
    abstractRule->Disconnect(source_1, dest_1);
    CPPUNIT_ASSERT(abstractRule->HasRelation(source_1, dest_1) == mitk::PropertyRelationRuleBase::RelationType::None);
    CPPUNIT_ASSERT(!this->hasRelationProperties(source_1, "1"));

    abstractRule->Disconnect(source_otherPurpose, dest_1);
    CPPUNIT_ASSERT_MESSAGE("Data of other rule type was removed.", !this->hasRelationProperties(source_otherPurpose, "1"));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkSourceImageRelationRule)
