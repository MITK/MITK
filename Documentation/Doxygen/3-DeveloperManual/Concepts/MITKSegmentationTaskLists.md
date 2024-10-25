# MITK Segmentation Task Lists {#MITKSegmentationTaskListsPage}

[TOC]

## Overview

MITK Segmentation Task Lists are a JSON-based file format defining a list of segmentation tasks.
Segmentation tasks consist at least of a path to a reference image and a unique result path.
The result path specifies where the final segmentation of the task is expected to be located once it is done.

Optional properties of a segmentation task include a task name and description as well as various presettings for the segmentation like a label name, a list of suggested names and colors for new labels, a label set preset, or even a pre-segmentation to begin with.
The complete set of properties is specified further below in the file format specification.

MITK Segmentation Task Lists must be considered experimental at the moment and are prone to change without any prior warning.

## File format

MITK Segmentation Task Lists are JSON files containing a JSON object as root, which must contain the two mandatory properties `FileFormat` and `Version`:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1
}
~~~

We also recommend to specify an optional `Name` that is used in the application if present instead of the plain filename of the JSON file:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Name": "My First Task List"
}
~~~

### Tasks

The root object must also contain a mandatory `Tasks` array containing JSON objects that specify the individual tasks of the task list.
A minimum task object must consist of `Image` and `Result` file paths.
`Image` is a path to the reference image for the task and `Result` is a path where the final segmentation is expected to be stored once it is done.
Paths can be absolute or relative to the MITK Segmentation Task List file.

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Tasks": [
    {
      "Image": "images/input.nrrd",
      "Result": "results/output.nrrd"
    }
  ]
}
~~~

In addition, tasks can define various optional properties that mainly specify the start conditions for a segmentation:

- `Name` (*string*): A name for the task.
- `Description` (*string*): A short description/definition of the task.
- `LabelName` (*string*): The name of the first label in a new segmentation that is created for the task on the fly.
- `LabelNameSuggestions` (*file path*): A Label Suggestions JSON file specifying names and optional colors that are suggested to the user for new labels in the segmentation.
- `Preset` (*file path*): A Label Set Preset XML file in MITK's .lsetp file format. The preset is applied to a new segmentation that is created for the task on the fly. We recommend to use the Segmentation plugin to create such label set preset files as described in its [user guide](@ref org_mitk_views_segmentationlabelpresets).
- `Segmentation` (*file path*): A pre-segmentation that a user can start with or should refine.
- `Dynamic` (*boolean*): In case `Image` refers to a dynamic (3d+t) image, specifies whether the segmentation should be static (*false*), i.e. equal for all time steps, or dynamic (*true*), i.e. individual for each time step.

### Task defaults / common properties

If a task list contains multiple tasks with common properties, they do not have to be specified for each and every task again and again.
Instead, the root object may contain an optional `Defaults` object that is identical in format to the tasks specified above.
There is one exception, though: A `Defaults` object must not contain a `Result` file path, since result files of tasks must be distinct by definition.
As the name indicates, default properties can still be overridden by individual tasks if they are specified explicitly.

### Label name suggestions

The `LabelNameSuggestions` property of segmentation tasks is supposed to reference a JSON file that consists of an array of objects with a mandatory `name` property and an optional `color` property.
For example:

~~~{.json}
[
  {
    "name": "Abdomen",
    "color": "red"
  },
  {
    "name": "Lung",
    "color": "#00ff00"
  },
  {
    "name": "Heart"
  },
  {
    "name": "Aortic Valve",
    "color": "CornflowerBlue"
  }
]
~~~

### Example

The following example is a complete showcase of the properties and features listed above.
It defines four tasks, three of which refer to the same reference image so it is specified as default.

Remember that the only task property required to be distinct is `Result` so you are pretty free in your task design.
For simplicity, we chose to define tasks around organs for this example and named the tasks accordingly:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 1,
  "Name": "Example Segmentation Task List",
  "Defaults": {
    "Image": "images/Pic3D.nrrd"
  },
  "Tasks": [
    {
      "Name": "Liver",
      "LabelName": "Liver",
      "LabelNameSuggestions": "suggestions/label_suggestions.json",
      "Description": "This task provides an image and label name suggestions for new labels. The segmentation will start with an empty label named Liver.",
      "Result": "results/liver.nrrd"
    },
    {
      "Name": "Kidneys",
      "Description": "This task provides an image and a label set preset that is applied to the new segmentation.",
      "Preset": "presets/kidneys.lsetp",
      "Result": "results/kidneys.nrrd"
    },
    {
      "Name": "Spleen",
      "Description": "This task provides an image and an initial (pre-)segmentation.",
      "Segmentation": "segmentations/spleen.nrrd",
      "Result": "results/spleen.nrrd"
    },
    {
      "Name": "Surprise",
      "Description": "And now for something completely different. This task overrides the default Image and starts with an empty static segmentation for a dynamic image.",
      "Image": "images/US4DCyl.nrrd",
      "Result": "results/US4DCyl.nrrd",
      "Dynamic": false
    }
  ]
}
~~~

### Version 2 and MITK Forms

Version 2 of the MITK Segmentation Task List file format adds support for MITK Forms.
An MITK Form is a feedback/survey form just like a Google Form for example.
A task object (including the `Defaults` object) can now reference an MITK Forms file to show the form together with a loaded task.
A user can then submit their responses per task into a common CSV response file.
It is highly recommended to set the `Name` of a task as it is used in the response file to identify the corresponding task.

In the following example, each task displays a common form for feedback purposes:

~~~{.json}
{
  "FileFormat": "MITK Segmentation Task List",
  "Version": 2,
  "Name": "Example Segmentation Task List with MITK Forms",
  "Defaults": {
    "Form": {
      "Path": "forms/feedback.json",
      "Result": "results/feedback.csv"
    }
  },
  "Tasks": [
    {
      "Name": "Case 01",
      "Image": "images/01.nrrd",
      "Result": "results/01.nrrd"
    },
    {
      "Name": "Case 02",
      "Image": "images/02.nrrd",
      "Result": "results/02.nrrd"
    }
  ]
}
~~~

The MITK Forms file defines a form asking for any trouble and the possibility to attach screenshots:

~~~{.json}
{
  "FileFormat": "MITK Form",
  "Version": 1,
  "Sections": [
    {
      "Title": "Feedback",
      "Description": "Please fill out this feedback form for all cases.",
      "Questions": [
        {
          "Text": "Did you have any trouble with this case?",
          "Required": true,
          "Type": "Multiple choice",
          "Options": [
            "Yes",
            "No"
          ]
        },
        {
          "Text": "Optionally take screenshots if you want to illustrate something:",
          "Type": "Screenshot"
        }
      ]
    }
  ]
}
~~~

The submitted CSV responses might look like the following table:

"Timestamp"            | "Task"    | "Did you have any trouble with this case?" | "Optionally take screenshots if you want to illustrate something:"
---------------------- | --------- | ------------------------------------------ | ------------------------------------------------------------------
"2024-09-26T09:15:35Z" | "Case 01" | "Yes"                                      | "screenshots/screenshot_ZDeCZI.png"
"2024-09-26T09:23:58Z" | "Case 02" | "No"                                       | ""
