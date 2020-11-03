/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneIOTestScenarioProvider_h_included
#define mitkSceneIOTestScenarioProvider_h_included

#include "mitkStandaloneDataStorage.h"

namespace mitk
{
  /**
    \brief Provides DataStorages that serve as test scenarios.

    This class provides the structure Scenario to describe a
    single test scenario for SceneIO and similar objects.
    A single scenario consists of
    - a DataStorage (in reality a method that constructs one)
    - an identifier / name
    - some flags that describe the scenario

    \warning Probably we should split the scenario and
             its description (== our expectations) at some
             time. This will be necessary once that we have
             two SceneIO mechanisms that differ in their
             capabilities.
  */
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
      std::string key;                      ///< Description / ID.
      bool serializable;                    ///< Do we expect that this can be stored in a .mitk file?
      std::string referenceArchiveFilename; ///< Absolute filename with a reference .mitk file.
      bool referenceArchiveLoadable;        ///< Do we expect that the reference can be loaded without errors?
      double comparisonPrecision; ///< Precision used for floating point comparisons after save/load cycle (eps).

      /// Construct the DataStorage for this scenario.
      DataStorage::Pointer BuildDataStorage() const;

      /**
        \param _key Description / ID.
        \param _scenarioProvider object that contains the member function in _providerMethod
        \param _providerMethod pointer to a member that creates a DataStorage for the scenario
        \param _isSerializable Do we expect that this can be stored in a .mitk file?
        \param _referenceArchiveFilename Absolute filename with a reference .mitk file.
        \param _isReferenceLoadable Do we expect that the reference can be loaded without errors?
        \param _comparisonPrecision Precision used for floating point comparisions after save/load cycle (eps).
      */
      Scenario(const std::string &_key,
               const SceneIOTestScenarioProvider *_scenarioProvider,
               SceneIOTestScenarioProvider::BuilderMethodPointer _providerMethod,
               bool _isSerializable,
               const std::string &_referenceArchiveFilename,
               bool _isReferenceLoadable,
               double _comparisonPrecision);

    private:
      const SceneIOTestScenarioProvider *m_ScenarioProvider;
      SceneIOTestScenarioProvider::BuilderMethodPointer m_ProviderMethod;
    };

    /// List of Scenarios.
    typedef std::vector<Scenario> ScenarioList;

    /// Returns _all_ registered scenarios.
    ScenarioList GetAllScenarios() const;

  private:
    ScenarioList m_Scenarios;

    /// Configures how many items count as many for some tests.
    int m_HowMuchIsMany;

    /**
      Registers one scenario with its description and a method for DataStorage creations.
    */
    void AddScenario(const std::string &key,
                     BuilderMethodPointer creator,
                     bool isSerializable,
                     const std::string &referenceArchiveFilename = std::string(),
                     bool isReferenceLoadable = false,
                     double comparisionPrecision = mitk::eps);

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

    /**
      Basic core type Image.
    */
    DataStorage::Pointer Image() const;

    /**
      Basic core type Surface.
    */
    DataStorage::Pointer Surface() const;

    /**
      Basic core type PointSet.
    */
    DataStorage::Pointer PointSet() const;

    /**
      GeometryData object (separate for specific precision).
    */
    DataStorage::Pointer GeometryData() const;

    /**
      Properties for various render windows, containing no or long names or values.
    */
    DataStorage::Pointer SpecialProperties() const;

  public:
// Helper to simplify writing the registration
#define AddSaveAndRestoreScenario(name) AddScenario(#name, &mitk::SceneIOTestScenarioProvider::name, true);

    // this one in the header so it is clearly visible when someone
    // adds a test to the bottom of the list
    SceneIOTestScenarioProvider() : m_HowMuchIsMany(50)
    {
      /// declare all your test cases here!
      AddSaveAndRestoreScenario(EmptyStorage);
      AddSaveAndRestoreScenario(OneEmptyNode);
      AddSaveAndRestoreScenario(OneEmptyNamedNode);
      AddSaveAndRestoreScenario(ManyTopLevelNodes);
      AddSaveAndRestoreScenario(LineOfManyOnlyChildren);
      AddSaveAndRestoreScenario(ComplicatedFamilySituation);

      AddSaveAndRestoreScenario(Image);
      AddSaveAndRestoreScenario(Surface);
      AddSaveAndRestoreScenario(PointSet);

      if (sizeof(size_t) != 4)
        // this test is deactivated on 32 bit systems since it fails
        // on the only 32 bit dartclient. To activate it there, one
        // would have to debug the precision problem on a 32 bit
        // machine and either adapt expectations or fix a bug.
        AddSaveAndRestoreScenario(GeometryData);

      AddSaveAndRestoreScenario(SpecialProperties);

      // AddScenario("GeometryData", &mitk::SceneIOTestScenarioProvider::GeometryData, true, std::string(), false,
      // mitk::eps);
    }

  }; // class

} // namespace

#endif
