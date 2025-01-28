# Order of configuration application
In the MITK dicom reader infrastructure all 3D+t configurations will be loaded from this directory.
They will be applied in alphabetic order when the best (minimal volume count) configuration for loading is searched.
The numeric file prefix is used to steer the order.

REMARK: If you change the names to alter the order, don't forget to change the names accordingly in "../../../files.txt".