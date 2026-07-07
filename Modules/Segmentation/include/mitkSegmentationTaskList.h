/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTaskList_h
#define mitkSegmentationTaskList_h

#include <mitkBaseData.h>
#include <mitkSegmentationTaskListMacros.h>

#include <MitkSegmentationExports.h>

#include <mitkFileSystem.h>
#include <optional>

namespace mitk
{
  /** \brief A list of segmentation tasks.
   *
   * See \ref MITKSegmentationTaskListsPage for more information.
   */
  class MITKSEGMENTATION_EXPORT SegmentationTaskList : public BaseData
  {
  public:
    class MITKSEGMENTATION_EXPORT Task
    {
    public:
      struct MITKSEGMENTATION_EXPORT Form
      {
        fs::path Path;
        fs::path Result;
      };

      struct MITKSEGMENTATION_EXPORT Scene
      {
        fs::path Path;
        std::string Image;
        std::string Segmentation;
      };

      Task();
      ~Task();

      void SetDefaults(const Task* defaults);

      mitkSegmentationTaskValueMacro(std::string, Name)
      mitkSegmentationTaskValueMacro(std::string, Description)
      mitkSegmentationTaskValueMacro(fs::path, Image)
      mitkSegmentationTaskValueMacro(fs::path, Segmentation)
      mitkSegmentationTaskValueMacro(std::string, LabelName)
      mitkSegmentationTaskValueMacro(fs::path, LabelNameSuggestions)
      mitkSegmentationTaskValueMacro(fs::path, Preset)
      mitkSegmentationTaskValueMacro(fs::path, Result)
      mitkSegmentationTaskValueMacro(bool, Dynamic)
      mitkSegmentationTaskValueMacro(Form, Form)
      mitkSegmentationTaskValueMacro(Scene, Scene)

    private:
      const Task* m_Defaults;
    };

    mitkClassMacro(SegmentationTaskList, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    mitkSegmentationTaskListValueMacro(std::string, Name)
    mitkSegmentationTaskListValueMacro(std::string, Description)
    mitkSegmentationTaskListValueMacro(fs::path, Image)
    mitkSegmentationTaskListValueMacro(fs::path, Segmentation)
    mitkSegmentationTaskListValueMacro(std::string, LabelName)
    mitkSegmentationTaskListValueMacro(fs::path, LabelNameSuggestions)
    mitkSegmentationTaskListValueMacro(fs::path, Preset)
    mitkSegmentationTaskListValueMacro(fs::path, Result)
    mitkSegmentationTaskListValueMacro(bool, Dynamic)
    mitkSegmentationTaskListValueMacro(Task::Form, Form)
    mitkSegmentationTaskListValueMacro(Task::Scene, Scene)

    /** \brief Returns the number of tasks in the list. */
    size_t GetNumberOfTasks() const;

    /**
     * \brief Adds a task to the list.
     * \param[in] subtask The task to add.
     * \return The index of the newly added task.
     */
    size_t AddTask(const Task& subtask);

    /**
     * \brief Returns a const pointer to the task at the given index.
     * \param[in] index The task index.
     * \return Pointer to the task, or nullptr if index is out of range.
     */
    const Task* GetTask(size_t index) const;

    /**
     * \brief Returns a mutable pointer to the task at the given index.
     * \param[in] index The task index.
     * \return Pointer to the task, or nullptr if index is out of range.
     */
    Task* GetTask(size_t index);

    /** \brief Returns the default task settings. */
    const Task& GetDefaults() const;

    /**
     * \brief Sets the default task settings applied to tasks lacking explicit values.
     * \param[in] defaults The default settings.
     */
    void SetDefaults(const Task& defaults);

    /** \brief Returns whether all tasks in the list are done. */
    bool IsDone() const;

    /**
     * \brief Returns whether the task at the given index is done.
     * \param[in] index The task index.
     */
    bool IsDone(size_t index) const;

    /** \brief Returns the file path from which this task list was loaded. */
    fs::path GetInputLocation() const;

    /** \brief Returns the base directory for resolving relative paths. */
    fs::path GetBasePath() const;

    /**
     * \brief Resolves a possibly relative path against the base path.
     * \param[in] path The path to resolve.
     * \return The absolute path.
     */
    fs::path GetAbsolutePath(const fs::path& path) const;

    /**
     * \brief Returns the interim (temporary) result path for the given path.
     * \param[in] path The original result path.
     * \return The interim path.
     */
    fs::path GetInterimPath(const fs::path& path) const;

    /**
     * \brief Saves the segmentation result for the task at the given index.
     * \param[in] index The task index.
     * \param[in] segmentation The segmentation data to save.
     * \param[in] saveAsInterimResult If true, saves to an interim location instead of the final result path.
     */
    void SaveTask(size_t index, const BaseData* segmentation, bool saveAsInterimResult = false);

    /** \brief Returns a const iterator to the first task. */
    std::vector<Task>::const_iterator begin() const;
    /** \brief Returns a const iterator past the last task. */
    std::vector<Task>::const_iterator end() const;

    /** \brief Returns an iterator to the first task. */
    std::vector<Task>::iterator begin();
    /** \brief Returns an iterator past the last task. */
    std::vector<Task>::iterator end();

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject*) override;

  protected:
    mitkCloneMacro(Self)

    SegmentationTaskList();
    SegmentationTaskList(const Self& other);
    ~SegmentationTaskList() override;

  private:
    Task m_Defaults;
    std::vector<Task> m_Tasks;
  };
}

#endif
