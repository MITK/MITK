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

#ifndef mitkSceneIOTestScenarioProvider_h_included
#define mitkSceneIOTestScenarioProvider_h_included

#include "mitkStandaloneDataStorage.h"

namespace mitk
{

class SceneIOTestScenarioProvider
{
public:

  /// Typedef for type BuilderMethodPointer which is function pointer to member of SceneIOTestScenarioProvider
  typedef DataStorage::Pointer (SceneIOTestScenarioProvider::*BuilderMethodPointer)() const;

  /**
    Structure to describe a single scenario.

    Holds some descriptive members plus a pointer to a
    method in SceneIOTestScenarioProvider that is able
    to create a DataStorage. This DataStorage shall
    represent some particularity to be tested in a
    related test.
  */
  struct Scenario
  {
    std::string key; ///< Description / ID.
    bool serializable; ///< Do we expect that this can be stored in a .mitk file?
    std::string referenceArchiveFilename; ///< Absolute filename with a reference .mitk file.
    bool referenceArchiveLoadable; ///< Do we expect that the reference can be loaded without errors?

    /// Construct the DataStorage for this scenario.
    DataStorage::Pointer BuildDataStorage() const;

    /**
      \param _key Description / ID.
      \param _scenarioProvider object that contains the member function in _providerMethod
      \param _providerMethod pointer to a member that creates a DataStorage for the scenario
      \param _isSerializable Do we expect that this can be stored in a .mitk file?
      \param _referenceArchiveFilename Absolute filename with a reference .mitk file.
      \param _isReferenceLoadable Do we expect that the reference can be loaded without errors?
    */
    Scenario(const std::string& _key,
        const SceneIOTestScenarioProvider* _scenarioProvider,
        SceneIOTestScenarioProvider::BuilderMethodPointer _providerMethod,
        bool _isSerializable,
        const std::string& _referenceArchiveFilename,
        bool _isReferenceLoadable);

  private:

    const SceneIOTestScenarioProvider* m_ScenarioProvider;
    SceneIOTestScenarioProvider::BuilderMethodPointer m_ProviderMethod;
  };

  /// List of Scenarios.
  typedef std::vector<Scenario> ScenarioList;

  /// Returns _all_ registered scenarios.
  ScenarioList GetAllScenarios() const;

private:

  ScenarioList m_Scenarios;

  /// Configures how many items count as many for some tests.
  unsigned int m_HowMuchIsMany = 100;

  void AddScenario(const std::string& key, BuilderMethodPointer creator, bool isSerializable, const std::string& referenceArchiveFilename = std::string(), bool isReferenceLoadable = false);

  /**
    An empty storage.
  */
  DataStorage::Pointer EmptyStorage() const;

  /**
    One single node without anything.
  */
  DataStorage::Pointer OneEmptyNode() const;

  /**
    One single node with a name.
  */
  DataStorage::Pointer OneEmptyNamedNode() const;

  /**
    Flat list.

    \verbatim
     |-o
     |-o
     |-o
     |-o
     ...
    \endverbatim
  */
  DataStorage::Pointer ManyTopLevelNodes() const;

  /**
    Degenerated tree.

    A tree like this:
    \verbatim
      |-o
        |-o
          |-o
          ...
    \endverbatim
  */
  DataStorage::Pointer LineOfManyOnlyChildren() const;

  /**
    Nodes with multiple parents.
  */
  DataStorage::Pointer ComplicatedFamilySituation() const;

public:

  // Helper to simplify writing the registration
  #define AddSaveAndRestoreScenario(name) AddScenario(#name, &mitk::SceneIOTestScenarioProvider:: ## name, true);

  // this one in the header so it is clearly visible when someone
  // adds a test to the bottom of the list
  SceneIOTestScenarioProvider()
  {
      /// declare all your test cases here!
      AddSaveAndRestoreScenario(EmptyStorage);
      AddSaveAndRestoreScenario(OneEmptyNode);
      AddSaveAndRestoreScenario(OneEmptyNamedNode);
      AddSaveAndRestoreScenario(ManyTopLevelNodes);
      AddSaveAndRestoreScenario(LineOfManyOnlyChildren);
      AddSaveAndRestoreScenario(ComplicatedFamilySituation);
  }


}; // class



} // namespace

#endif

