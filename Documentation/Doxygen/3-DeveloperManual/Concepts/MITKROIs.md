# MITK ROIs {#MITKROIsPage}

[TOC]

## Overview

MITK ROIs are a JSON-based file format defining a list of region of interests (ROIs).

MITK ROIs must be considered experimental at the moment and are prone to change without any prior warning.

## File format

MITK ROIs are JSON files containing a JSON object as root, which must contain the two mandatory properties `FileFormat` and `Version`:

~~~{.json}
{
  "FileFormat": "MITK ROI",
  "Version": 1
}
~~~

### ROIs

...
