/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKTOTALSEGMENTATORTOOL_H
#define MITKTOTALSEGMENTATORTOOL_H

#include "mitkSegWithPreviewTool.h"
#include <MitkSegmentationExports.h>
#include "mitkProcessExecutor.h"


namespace us
{
  class ModuleResource;
}

namespace mitk
{

  /**
    \brief TotalSegmentator segmentation tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT TotalSegmentatorTool : public SegWithPreviewTool
  {
  public:
    mitkClassMacro(TotalSegmentatorTool, SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    void Activated() override;

    itkSetMacro(MitkTempDir, std::string);
    itkGetConstMacro(MitkTempDir, std::string); 
    
    itkSetMacro(SubTask, std::string);
    itkGetConstMacro(SubTask, std::string);
    
    itkSetMacro(PythonPath, std::string);
    itkGetConstMacro(PythonPath, std::string);

    itkSetMacro(GpuId, int);
    itkGetConstMacro(GpuId, int);

    itkSetMacro(Fast, bool);
    itkGetConstMacro(Fast, bool);
    itkBooleanMacro(Fast);

   /**
    * @brief Event triggered after model download started.
    */
    mitk::Message1<const bool> TotalSegDownloadMessageEvent;

  protected:
    TotalSegmentatorTool();
    ~TotalSegmentatorTool();

    /**
     * @brief Overridden method from the tool manager to execute the segmentation
     * Implementation:
     * 1. Creates temp directory, if not done already.
     * 2. Parses Label names from map_to_binary.py for using later on.
     * 3. Calls "run_totalsegmentator" method.
     * 4. Expects an output image to be saved in the temporary directory by the python process. Loads it as
     *    MultiLabelSegmentation and sets to previewImage.
     *
     * @param inputAtTimeStep
     * @param oldSegAtTimeStep
     * @param previewImage
     * @param timeStep
     */
    void DoUpdatePreview(const Image* inputAtTimeStep, const Image* oldSegAtTimeStep, MultiLabelSegmentation* previewImage, TimeStepType timeStep) override;
    void UpdatePrepare() override;

  private:

    /**
     * @brief Runs Totalsegmentator python process with desired arguments
     * 
     */
    void run_totalsegmentator(ProcessExecutor*, const std::string&, const std::string&, bool, bool, int, const std::string&);

    /**
     * @brief Applies the m_LabelMapTotal lookup table on the output segmentation MultiLabelSegmentation.
     * 
     */
    void MapLabelsToSegmentation(const mitk::MultiLabelSegmentation*, mitk::MultiLabelSegmentation*, std::map<mitk::Label::PixelType, std::string>&);

    /**
     * @brief Parses map_to_binary.py file to extract label ids and names
     * and stores as a map for reference in m_LabelMapTotal
     * 
     */
    void ParseLabelMapTotalDefault();

    /**
     * @brief Get the Label Map Path from the virtual environment location
     * 
     * @return std::string 
     */
    std::string GetLabelMapPath();

    /**
     * @brief Agglomerate many individual mask image files into one multi-label MultiLabelSegmentation in the
     * given filePath order.
     * 
     * @param filePaths 
     * @param dimension 
     * @param geometry 
     * @return MultiLabelSegmentation::Pointer 
     */
    MultiLabelSegmentation::Pointer AgglomerateLabelFiles(std::vector<std::string>& filePaths, const unsigned int* dimension, mitk::BaseGeometry* geometry);

    std::string m_MitkTempDir;
    std::string m_PythonPath;
    std::string m_SubTask = "total";
    int m_GpuId = 0;
    std::map<mitk::Label::PixelType, std::string> m_LabelMapTotal;
    std::map<mitk::Label::PixelType, std::string> m_LabelMapTotalMR;
    bool m_Fast = true;
    const std::string TEMPLATE_FILENAME = "XXXXXX_000_0000.nii.gz";
    const std::string DEFAULT_TOTAL_TASK = "total";
    const std::string DEFAULT_TOTAL_TASK_MRI = "total_mr";
    const std::unordered_map<std::string, std::vector<std::string>> SUBTASKS_MAP =
    {
      {"body", { "body.nii.gz", "body_trunc.nii.gz", "body_extremities.nii.gz", "skin.nii.gz"}},
      {"hip_implant", {"hip_implant.nii.gz"}},
      {"cerebral_bleed", {"intracerebral_hemorrhage.nii.gz"}},
      {"lung_vessels", {"lung_vessels.nii.gz", "lung_trachea_bronchia.nii.gz"}},
      {"pleural_pericard_effusion", {"pleural_effusion.nii.gz", "pericardial_effusion.nii.gz"}},
      {"head_glands_cavities", {"eye_left.nii.gz", "eye_right.nii.gz", "eye_lens_left.nii.gz", "eye_lens_right.nii.gz", "optic_nerve_left.nii.gz", "optic_nerve_right.nii.gz", "parotid_gland_left.nii.gz", "parotid_gland_right.nii.gz", "submandibular_gland_right.nii.gz", "submandibular_gland_left.nii.gz", "nasopharynx.nii.gz", "oropharynx.nii.gz", "hypopharynx.nii.gz", "nasal_cavity_right.nii.gz", "nasal_cavity_left.nii.gz", "auditory_canal_right.nii.gz", "auditory_canal_left.nii.gz", "soft_palate.nii.gz", "hard_palate.nii.gz" }},
      {"head_muscles", {"masseter_right.nii.gz", "masseter_left.nii.gz", "temporalis_right.nii.gz", "temporalis_left.nii.gz", "lateral_pterygoid_right.nii.gz", "lateral_pterygoid_left.nii.gz", "medial_pterygoid_right.nii.gz", "medial_pterygoid_left.nii.gz", "tongue.nii.gz", "digastric_right.nii.gz", "digastric_left.nii.gz"}},
      {"headneck_bones_vessels", {"larynx_air.nii.gz" , "thyroid_cartilage.nii.gz" , "hyoid.nii.gz" , "cricoid_cartilage.nii.gz", "zygomatic_arch_right.nii.gz", "zygomatic_arch_left.nii.gz", "styloid_process_right.nii.gz", "styloid_process_left.nii.gz", "internal_carotid_artery_right.nii.gz", "internal_carotid_artery_left.nii.gz", "internal_jugular_vein_right.nii.gz", "internal_jugular_vein_left.nii.gz"}},
      {"headneck_muscles", {"sternocleidomastoid_right.nii.gz", "sternocleidomastoid_left.nii.gz", "superior_pharyngeal_constrictor.nii.gz", "middle_pharyngeal_constrictor.nii.gz", "inferior_pharyngeal_constrictor.nii.gz", "trapezius_right.nii.gz", "trapezius_left.nii.gz", "platysma_right.nii.gz", "platysma_left.nii.gz", "levator_scapulae_right.nii.gz", "levator_scapulae_left.nii.gz", "anterior_scalene_right.nii.gz", "anterior_scalene_left.nii.gz", "middle_scalene_right.nii.gz", "middle_scalene_left.nii.gz", "posterior_scalene_right.nii.gz", "posterior_scalene_left.nii.gz", "sterno_thyroid_right.nii.gz", "sterno_thyroid_left.nii.gz", "thyrohyoid_right.nii.gz", "thyrohyoid_left.nii.gz", "prevertebral_right.nii.gz", "prevertebral_left.nii.gz"}},
      {"liver_vessels", {"liver_vessels.nii.gz" , "liver_tumor.nii.gz"}},
      {"oculomotor_muscles", {"skull.nii.gz", "eyeball_right.nii.gz", "lateral_rectus_muscle_right.nii.gz", "superior_oblique_muscle_right.nii.gz", "levator_palpebrae_superioris_right.nii.gz", "superior_rectus_muscle_right.nii.gz", "medial_rectus_muscle_left.nii.gz", "inferior_oblique_muscle_right.nii.gz", "inferior_rectus_muscle_right.nii.gz", "optic_nerve_left.nii.gz", "eyeball_left.nii.gz", "lateral_rectus_muscle_left.nii.gz", "superior_oblique_muscle_left.nii.gz", "levator_palpebrae_superioris_left.nii.gz", "superior_rectus_muscle_left.nii.gz", "medial_rectus_muscle_right.nii.gz", "inferior_oblique_muscle_left.nii.gz", "inferior_rectus_muscle_left.nii.gz", "optic_nerve_right.nii.gz"}},
      {"lung_nodules", {"lung.nii.gz", "lung_nodules.nii.gz", }}, 
      {"kidney_cysts", {"kidney_cyst_left.nii.gz", "kidney_cyst_right.nii.gz"}},
      {"breasts", {"breast.nii.gz"}},
      {"liver_segments", {"liver_segment_1.nii.gz", "liver_segment_2.nii.gz", "liver_segment_3.nii.gz", "liver_segment_4.nii.gz", "liver_segment_5.nii.gz", "liver_segment_6.nii.gz", "liver_segment_7.nii.gz", "liver_segment_8.nii.gz"}},
      {"liver_segments_mr", {"liver_segment_1.nii.gz", "liver_segment_2.nii.gz", "liver_segment_3.nii.gz", "liver_segment_4.nii.gz", "liver_segment_5.nii.gz", "liver_segment_6.nii.gz", "liver_segment_7.nii.gz", "liver_segment_8.nii.gz"}},
    };  
  }; // class
} // namespace
#endif
