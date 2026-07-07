/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCLResultWriter_h
#define mitkCLResultWriter_h

#include <MitkCLUtilitiesExports.h>

#include <sstream>
#include <fstream>
#include <vector>

#include <mitkAbstractGlobalImageFeature.h>

namespace mitk
{
  namespace cl
  {
    /**
     * \brief Writes classification feature results to a CSV-style output file.
     *
     * Supports row-by-row construction of output with columns for subject information,
     * feature descriptions, and computed feature values. Results can be written with
     * configurable decimal point characters and optional header rows.
     */
    class MITKCLUTILITIES_EXPORT FeatureResultWriter
    {
    public:
      /**
       * \brief Construct a result writer for the given file path.
       * \param filePath Path to the output file.
       * \param mode Output mode (0 = new file, 1 = append).
       */
      FeatureResultWriter(std::string filePath, int mode);

      /** \brief Destructor. Flushes and closes the output file. */
      ~FeatureResultWriter();

      /**
       * \brief Set a custom decimal point character for numeric output.
       * \param decimal The decimal separator character (e.g. ',' for European locale).
       */
      void SetDecimalPoint(char decimal);

      /**
       * \brief Add subject-identifying information to the current row.
       * \param value The subject identifier string.
       */
      void AddSubjectInformation(std::string value);

      /**
       * \brief Add a string column value to the current row.
       * \param value The string value to add.
       */
      void AddColumn(std::string value);

      /**
       * \brief Add a numeric column value to the current row.
       * \param value The numeric value to add.
       */
      void AddColumn(double value);

      /**
       * \brief Finalize the current row and start a new one.
       * \param endName A label appended at the end of the row.
       */
      void NewRow(std::string endName);

      /**
       * \brief Add a row of feature results to the output.
       * \param desc Description string for this result set.
       * \param slice The slice index (-1 for the entire image).
       * \param stats The list of computed feature name-value pairs.
       * \param withHeader Whether to include a header row before the data.
       * \param withDescription Whether to include feature description columns.
       */
      void AddResult(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader, bool withDescription);

      /**
       * \brief Add a header row describing the feature columns.
       * \param desc Description string for this header.
       * \param slice The slice index.
       * \param stats The feature list used to derive column names.
       * \param withHeader Whether to actually write the header.
       * \param withDescription Whether to include feature description columns.
       */
      void AddHeader(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader, bool withDescription);

    private:
      int m_Mode;
      std::size_t m_CurrentRow;
      int m_CurrentElement;
      std::string m_Separator;
      std::ofstream m_Output;
      std::vector<std::string> m_List;
      std::string m_SubjectInformation;
      bool m_UsedSubjectInformation;
      bool m_UseSpecialDecimalPoint;
      char m_DecimalPoint;
    };
  }
}

#endif
