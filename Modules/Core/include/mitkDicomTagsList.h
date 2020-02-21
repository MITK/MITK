#pragma once

#include <string>
#include <vector>
#include <map>

namespace mitk {
  const std::string TAG_IMAGE_TYPE = "0008|0008";
  const std::string TAG_INST_CREATE_DATE = "0008|0012";
  const std::string TAG_INST_CREATE_TIME = "0008|0013";
  const std::string TAG_INST_CREATOR_UID = "0008|0014";
  const std::string TAG_SOP_CLASS_UID = "0008|0016";
  const std::string TAG_SOP_INST_UID = "0008|0018";
  const std::string TAG_STUDY_DATE = "0008|0020";
  const std::string TAG_SERIES_DATE = "0008|0021";
  const std::string TAG_ACQUISITION_DATE = "0008|0022";
  const std::string TAG_IMAGE_DATE = "0008|0023";
  const std::string TAG_ACQUISITION_DATE_TIME = "0080|002a";
  const std::string TAG_STUDY_TIME = "0008|0030";
  const std::string TAG_SERIES_TIME = "0008|0031";
  const std::string TAG_ACQUISITION_TIME = "0008|0032";
  const std::string TAG_IMAGE_TIME = "0008|0033";
  const std::string TAG_ACCESS_NO = "0008|0050";
  const std::string TAG_QR_LEVEL = "0008|0052";
  const std::string TAG_RETR_AET = "0008|0054";
  const std::string TAG_INST_AVAILABILITY = "0008|0056";
  const std::string TAG_FAILED_SOPINSTUID_LIST = "0008|0058";
  const std::string TAG_MODALITY = "0008|0060";
  const std::string TAG_CONVERSION_TYPE = "0008|0064";
  const std::string TAG_PRESENTATION_TYPE = "0008|0068";
  const std::string TAG_MANUFACTOR = "0008|0070";
  const std::string TAG_INSTITUT_NAME = "0008|0080";
  const std::string TAG_INSTITUT_ADDRESS = "0008|0081";
  const std::string TAG_REF_PHYS_NAME = "0008|0090";
  const std::string TAG_REF_PHYS_ADDR = "0008|0092";
  const std::string TAG_REF_PHYS_TEL = "0008|0094";
  const std::string TAG_CODE_VALUE = "0008|0100";
  const std::string TAG_CODING_SCHEME_DESIGNATOR = "0008|0102";
  const std::string TAG_CODE_MEANING = "0008|0104";
  const std::string TAG_STATION_NAME = "0008|1010";
  const std::string TAG_STUDY_DESC = "0008|1030";
  const std::string TAG_PROC_CODE_SEQ = "0008|1032";
  const std::string TAG_SERIES_DESC = "0008|103e";
  const std::string TAG_INSTITUT_DEPT_NAME = "0008|1040";
  const std::string TAG_PERF_PHYS_NAME = "0008|1050";
  const std::string TAG_READ_PHYS_NAME = "0008|1060";
  const std::string TAG_OPERATOR_NAME = "0008|1070";
  const std::string TAG_ADMIT_DIAG_DESC = "0008|1080";
  const std::string TAG_MANFAC_MODEL_NAME = "0008|1090";
  const std::string TAG_REF_STUDY_SEQ = "0008|1110";
  const std::string TAG_REF_STUDY_COMPONENT_SEQ = "0008|1111";
  const std::string TAG_REF_PAT_SEQ = "0008|1120";
  const std::string TAG_REF_IMAGE_SEQ = "0008|1140";
  const std::string TAG_REF_SOP_CLASS_UID = "0008|1150";
  const std::string TAG_REF_SOP_INST_UID = "0008|1155";
  const std::string TAG_REF_SOP_SEQ = "0008|1199";
  const std::string TAG_PAT_NAME = "0010|0010";
  const std::string TAG_PAT_ID = "0010|0020";
  const std::string TAG_PAT_BIRTH_DATE = "0010|0030";
  const std::string TAG_PAT_SEX = "0010|0040";
  const std::string TAG_OTHER_PAT_ID = "0010|1000";
  const std::string TAG_PAT_AGE = "0010|1010";
  const std::string TAG_PAT_SIZE = "0010|1020";
  const std::string TAG_PAT_WEIGHT = "0010|1030";
  const std::string TAG_PAT_ADDR = "0010|1040";
  const std::string TAG_PAT_TEL = "0010|2154";
  const std::string TAG_OCCUPATION = "0010|2180";
  const std::string TAG_ADDITIONAL_PT_HISTORY = "0010|21b0";
  const std::string TAG_PAT_COMMENT = "0010|4000";
  const std::string TAG_CLINICAL_TRIAL_SPONSOR = "0012|0010";
  const std::string TAG_CLINICAL_TRIAL_PROT_NAME = "0012|0021";
  const std::string TAG_CLINICAL_TRIAL_SITE_NAME = "0012|0031";
  const std::string TAG_CLINICAL_TRIAL_CENTER = "0012|0060";
  const std::string TAG_CLINICAL_TRIAL_PROT_ID = "0012|0020";
  const std::string TAG_CLINICAL_TRIAL_SITE_ID = "0012|0030";
  const std::string TAG_CLINICAL_TRIAL_SUBJECT_ID = "0012|0030";
  const std::string TAG_CLINICAL_TRIAL_SUBJECT_READING_ID = "0012|0030";
  const std::string TAG_BODY_PART_EXAMINED = "0018|0015";
  const std::string TAG_SLANT_ANGLE = "0018|0059";
  const std::string TAG_KVP = "0018|0060";
  const std::string TAG_DEVICE_SERIAL_NUMBER = "0018|1000";
  const std::string TAG_PLATE_ID = "0018|1004";
  const std::string TAG_SECONDARY_CAPTURE_DEVICE_ID = "0018|1010";
  const std::string TAG_SECONDARY_CAPTURE_DEVICE_MANUFACTURER = "0018|1016";
  const std::string TAG_SOFTWARE_VERSION = "0018|1020";
  const std::string TAG_SECONDARY_CAPTURE_DATE = "0018|1012";
  const std::string TAG_SECONDARY_CAPTURE_TIME = "0018|1014";
  const std::string TAG_HARDCOPY_DEVICE_MANUFACTURER = "0018|1017";
  const std::string TAG_SECONDARY_CAPTURE_DEVICE_MODEL_NAME = "0018|1018";
  const std::string TAG_SECONDARY_CAPTURE_DEVICE_SOFTWARE_VERSION = "0018|1019";
  const std::string TAG_VIDEO_IMAGE_FORMAT_ACQUIRED = "0018|1022";
  const std::string TAG_PROT_NAME = "0018|1030";
  const std::string TAG_EXPOSURE_TIME = "0018|1150";
  const std::string TAG_XRAYTUBE_CURRENT = "0018|1151";
  const std::string TAG_EXPOSURE_MAS = "0018|1152";
  const std::string TAG_FILTER_TYPE = "0018|1160";
  const std::string TAG_IMGR_PIXEL_SPACING = "0018|1164";
  const std::string TAG_ANODE_MATERIAL = "0018|1191";
  const std::string TAG_DIST_SOURCE_DETECTOR = "0018|1110";
  const std::string TAG_DIST_SOURCE_PATIENT = "0018|1111";
  const std::string TAG_BODY_PART_THICKNESS = "0018|11a0";
  const std::string TAG_COMPRESSION_FORCE = "0018|11a2";
  const std::string TAG_ESTIMATED_RADIOGRAPHIC_MAGNIFICATION_FACTOR = "0018|1114";
  const std::string TAG_FIELD_OF_VIEW_SHAPE = "0018|1147";
  const std::string TAG_FIELD_OF_VIEW_DIMENSION = "0018|1149";
  const std::string TAG_GRID = "0018|1166";
  const std::string TAG_FOCAL_SPOT = "0018|1190";
  const std::string TAG_REL_X_RAY_EXPOSURE = "0018|1405";
  const std::string TAG_ACQUISITION_DEVICE_PROCESSING_DESCRIPTION = "0018|1400";
  const std::string TAG_ACQUISITION_DEVICE_PROCESSING_CODE = "0018|1401";
  const std::string TAG_RELATIVE_XRAY_EXPOSURE = "0018|1405";
  const std::string TAG_POSITIONER_TYPE = "0018|1508";
  const std::string TAG_POSITIONER_PRIMARY_ANGLE = "0018|1510";
  const std::string TAG_DETECTOR_PRIMARY_ANGLE = "0018|1530";
  const std::string TAG_SECONDARY_DET_ANGLE = "0018|1531";
  const std::string TAG_COLLIMATOR_SHAPE = "0018|1700";
  const std::string TAG_COLLIMATOR_LEFT_EDGE = "0018|1702";
  const std::string TAG_COLLIMATOR_RIGHT_EDGE = "0018|1704";
  const std::string TAG_COLLIMATOR_UPPER_EDGE = "0018|1706";
  const std::string TAG_COLLIMATOR_LOWER_EDGE = "0018|1708";
  const std::string TAG_NOMINAL_SCANNED_PXL_SPACING = "0018|2010";
  const std::string TAG_LAST_DETECTOR_CALIBERATION_DATE = "0018|700c";
  const std::string TAG_SENSITIVITY = "0018|6000";
  const std::string TAG_VIEW_POSITION = "0018|5101";
  const std::string TAG_DETECTOR_CONDITIONS_NOMINAL_FLAG = "0018|7000";
  const std::string TAG_DETECTOR_TEMP = "0018|7001";
  const std::string TAG_DETECTOR_TYPE = "0018|7004";
  const std::string TAG_DETECTOR_ID = "0018|700A";
  const std::string TAG_DETECTOR_CONFIG = "0018|7005";
  const std::string TAG_DETECTOR_BINNING = "0018|701a";
  const std::string TAG_DETECTOR_ELEMENT_PHYSICAL_SIZE = "0018|7020";
  const std::string TAG_DETECTOR_ELEMENT_SPACING = "0018|7022";
  const std::string TAG_DETECTOR_ACTIVE_SHAPE = "0018|7024";
  const std::string TAG_DETECTOR_ACTIVE_DIMENSION = "0018|7026";
  const std::string TAG_FIELD_OF_VIEW_ORIGIN = "0018|7030";
  const std::string TAG_FIELD_OF_VIEW_ROTATION = "0018|7032";
  const std::string TAG_FIELD_OF_VIEW_HORIZONTAL_FLIP = "0018|7034";
  const std::string TAG_FILTER_MATERIAL = "0018|7050";
  const std::string TAG_FILTER_THICKNESS_MIN = "0018|7052";
  const std::string TAG_FILTER_THICKNESS_MAX = "0018|7054";
  const std::string TAG_EXPOSURE_CONTROL_MODE = "0018|7060";
  const std::string TAG_EXPOSURE_CONTROL_MODE_DESC = "0018|7062";
  const std::string TAG_EXPOSURE_STATUS = "0018|7064";
  const std::string TAG_ISOCENTRE_REF_SEQUENCE = "0018|9462";
  const std::string TAG_ISOCENTRE_PRIMARY_ANGLE = "0018|9463";
  const std::string TAG_ISOCENTRE_SECONDARY_ANGLE = "0018|9464";
  const std::string TAG_ISOCENTRE_DETECTOR_ROT_ANGLE = "0018|9465";
  const std::string TAG_TABLE_X_POS_TO_ISO_CTR = "0018|9466";
  const std::string TAG_TABLE_Y_POS_TO_ISO_CTR = "0018|9467";
  const std::string TAG_TABLE_Z_POS_TO_ISO_CTR = "0018|9468";
  const std::string TAG_TABLE_HORIZ_ROT_ANGLE = "0018|9469";
  const std::string TAG_TABLE_HEAD_TILT_ANGLE = "0018|9470";
  const std::string TAG_TABLE_CRADLE_TILT_ANGLE = "0018|9471";
  const std::string TAG_SANDBOX_PRIVATE_CREATOR = "0019|0010";
  const std::string TAG_MOTOR_SOURCE_LONGITUDINAL = "0019|1001";
  const std::string TAG_MOTOR_SOURCE_TRANSVERSE = "0019|1002";
  const std::string TAG_MOTOR_DETECTOR_LONGITUDINAL = "0019|1003";
  const std::string TAG_MOTOR_DETECTOR_TRANSVERSE = "0019|1004";
  const std::string TAG_MOTOR_SOURCE_ROTATIONAL = "0019|1005";
  const std::string TAG_MOTOR_DETECTOR_ROTATIONAL = "0019|1006";
  const std::string TAG_MOTOR_OBJECT_ROTATIONAL = "0019|1007";
  const std::string TAG_NUMBER_SPITS = "0019|1008";
  const std::string TAG_NUMBER_FRAMES = "0019|1009";
  const std::string TAG_STUDY_INST_UID = "0020|000d";
  const std::string TAG_SERIES_INST_UID = "0020|000e";
  const std::string TAG_STUDY_ID = "0020|0010";
  const std::string TAG_SERIES_NO = "0020|0011";
  const std::string TAG_ACQUISITION_NO = "0020|0012";
  const std::string TAG_IMAGE_NO = "0020|0013";
  const std::string TAG_PATIENT_ORIENTATION = "0020|0020";
  const std::string TAG_OVERLAY_NO = "0020|0022";
  const std::string TAG_CURVE_NO = "0020|0024";
  const std::string TAG_LUT_NO = "0020|0026";
  const std::string TAG_IMAGE_POSITION = "0020|0032";
  const std::string TAG_IMAGE_ORIENTATION = "0020|0037";
  const std::string TAG_LATERALITY = "0020|0060";
  const std::string TAG_IMAGE_LATERALITY = "0020|0062";
  const std::string TAG_NO_PAT_REL_STUDIES = "0020|1200";
  const std::string TAG_NO_PAT_REL_SERIES = "0020|1202";
  const std::string TAG_NO_PAT_REL_IMAGES = "0020|1204";
  const std::string TAG_NO_STUDY_REL_SERIES = "0020|1206";
  const std::string TAG_NO_STUDY_REL_IMAGES = "0020|1208";
  const std::string TAG_NO_SERIES_REL_IMAGES = "0020|1209";
  const std::string TAG_TEMPORAL_INDEX = "0020|9128";
  const std::string TAG_SAMPLES_PER_PX = "0028|0002";
  const std::string TAG_PHOTOMETRIC = "0028|0004";
  const std::string TAG_ROWS = "0028|0010";
  const std::string TAG_COLUMNS = "0028|0011";
  const std::string TAG_PLANES = "0028|0012";
  const std::string TAG_PIXEL_SPACING = "0028|0030";
  const std::string TAG_BITS_ALLOC = "0028|0100";
  const std::string TAG_BITS_STORED = "0028|0101";
  const std::string TAG_HIGH_BIT = "0028|0102";
  const std::string TAG_PX_REPRESENT = "0028|0103";
  const std::string TAG_SMALLEST_IMG_PX_VALUE = "0028|0106";
  const std::string TAG_LARGEST_IMG_PX_VALUE = "0028|0107";
  const std::string TAG_QUALITY_CONTROL_IMAGE = "0028|0300";
  const std::string TAG_BURNED_IN_ANNOTATION = "0028|0301";
  const std::string TAG_PXL_INTENSITY_RELATIONSHIP = "0028|1040";
  const std::string TAG_PXL_INTENSITY_SIGN = "0028|1041";
  const std::string TAG_WINDOW_CENTER = "0028|1050";
  const std::string TAG_WINDOW_WIDTH = "0028|1051";
  const std::string TAG_RESCALE_INTERCEPT = "0028|1052";
  const std::string TAG_RESCALE_SLOPE = "0028|1053";
  const std::string TAG_RESCALE_TYPE = "0028|1054";
  const std::string TAG_RED_PAL_LUT = "0028|1201";
  const std::string TAG_GREEN_PAL_LUT = "0028|1202";
  const std::string TAG_BLUE_PAL_LUT = "0028|1203";
  const std::string TAG_SEG_RED_PAL_LUT = "0028|1221";
  const std::string TAG_SEG_GREEN_PAL_LUT = "0028|1222";
  const std::string TAG_SEG_BLUE_PAL_LUT = "0028|1223";
  const std::string TAG_LOSSY_IMAGE_COMPRESSION = "0028|2110";
  const std::string TAG_ICON_IMAGE_SEQ = "0088|0200";
  const std::string TAG_TOPIC_TITLE = "0088|0904";
  const std::string TAG_TOPIC_SUBJECT = "0088|0906";
  const std::string TAG_TOPIC_AUTHOR = "0088|0910";
  const std::string TAG_TOPIC_KEYWORDS = "0088|0912";
  const std::string TAG_STUDY_STATUS_ID = "0032|000a";
  const std::string TAG_REQ_PHYS = "0032|1032";
  const std::string TAG_REQ_SERVICE = "0032|1033";
  const std::string TAG_REQ_PROC_DESC = "0032|1060";
  const std::string TAG_REQ_CONTRAST_AGENT = "0032|1070";
  const std::string TAG_ADMISSION_ID = "0038|0010";
  const std::string TAG_ROUTE_OF_ADMISS = "0038|0016";
  const std::string TAG_ADMIT_DATE = "0038|0020";
  const std::string TAG_SPS_STATION_AET = "0040|0001";
  const std::string TAG_SPS_START_DATE = "0040|0002";
  const std::string TAG_SPS_START_TIME = "0040|0003";
  const std::string TAG_SPS_END_DATE = "0040|0004";
  const std::string TAG_SPS_END_TIME = "0040|0005";
  const std::string TAG_SPS_PERF_PHYS_NAME = "0040|0006";
  const std::string TAG_SPS_DESC = "0040|0007";
  const std::string TAG_SPS_ACTION_SEQ = "0040|0008";
  const std::string TAG_SPS_ID = "0040|0009";
  const std::string TAG_SPS_STAGE_CODE_SQ = "0040|000a";
  const std::string TAG_SPS_PERF_PHYSICIAN_ID_SQ = "0040|000b";
  const std::string TAG_SPS_STATION_NAME = "0040|0010";
  const std::string TAG_SPS_LOCATION = "0040|0011";
  const std::string TAG_PRE_MEDICATION = "0040|0012";
  const std::string TAG_SPS_STATUS = "0040|0020";
  const std::string TAG_SPS_SEQ = "0040|0100";
  const std::string TAG_REF_STANDALONE_SOP_INST_SEQ = "0040|0220";
  const std::string TAG_PERF_STATION_AET = "0040|0241";
  const std::string TAG_PERF_STATION_NAME = "0040|0242";
  const std::string TAG_PERF_LOCATION = "0040|0243";
  const std::string TAG_PPS_START_DATE = "0040|0244";
  const std::string TAG_PPS_START_TIME = "0040|0245";
  const std::string TAG_PPS_END_DATE = "0040|0250";
  const std::string TAG_PPS_END_TIME = "0040|0251";
  const std::string TAG_PPS_STATUS = "0040|0252";
  const std::string TAG_PPS_ID = "0040|0253";
  const std::string TAG_PPS_DESC = "0040|0254";
  const std::string TAG_PERF_TYPE_DESC = "0040|0255";
  const std::string TAG_PERF_ACTION_SEQ = "0040|0260";
  const std::string TAG_SPS_ATTRIB_SEQ = "0040|0270";
  const std::string TAG_REQ_ATTRIB_SEQ = "0040|0275";
  const std::string TAG_COMMENT_PPS = "0040|0280";
  const std::string TAG_QUANTITY_SEQ = "0040|0293";
  const std::string TAG_QUANTITY = "0040|0294";
  const std::string TAG_MEASURING_UNITS_SEQ = "0040|0295";
  const std::string TAG_BILLING_ITEM_SEQ = "0040|0296";
  const std::string TAG_TOT_TIME_FLUOROS = "0040|0300";
  const std::string TAG_TOT_NUM_EXPOS = "0040|0301";
  const std::string TAG_ENTRANCE_DOSE = "0040|0302";
  const std::string TAG_EXPOSED_AREA = "0040|0303";
  const std::string TAG_DISTANCE_SOURCE_ENTRANCE = "0040|0306";
  const std::string TAG_COMMENT_RADIATION_DOSE = "0040|0310";
  const std::string TAG_X_RAY_OUTPUT = "0040|0312";
  const std::string TAG_HALF_VALUE_LAYER = "0040|0314";
  const std::string TAG_ORGAN_DOSE = "0040|0316";
  const std::string TAG_ORGAN_EXPOSED = "0040|0318";
  const std::string TAG_BILLING_PPS_SEQ = "0040|0320";
  const std::string TAG_FILM_CONSUM_SEQ = "0040|0321";
  const std::string TAG_BILLING_SUPPL_DEVICES_SEQ = "0040|0324";
  const std::string TAG_REF_PPS_SEQ = "0040|0330";
  const std::string TAG_PERF_SERIES_SEQ = "0040|0340";
  const std::string TAG_COMMENTS_ON_SPS = "0040|0400";
  const std::string TAG_MEAS_UNIT_SQ = "0040|08EA";
  const std::string TAG_REQ_PROC_ID = "0040|1001";
  const std::string TAG_REASON_REQ_PROC = "0040|1002";
  const std::string TAG_REQ_PROC_PRIORITY = "0040|1003";
  const std::string TAG_PAT_TRANS_ARRANGE = "0040|1004";
  const std::string TAG_REQ_PROCEDURE_LOC = "0040|1005";
  const std::string TAG_PLACER_ORDER_NUM_PROC = "0040|1006";
  const std::string TAG_FILLER_ORDER_NUM_PROC = "0040|1007";
  const std::string TAG_CONFID_CODE = "0040|1008";
  const std::string TAG_REPORT_PRIORITY = "0040|1009";
  const std::string TAG_RECIPIENTS_OF_RESULT = "0040|1010";
  const std::string TAG_REQ_PROC_COMMENT = "0040|1400";
  const std::string TAG_REASON_ISRQ = "0040|2001";
  const std::string TAG_ISSUE_DATE_ISRQ = "0040|2004";
  const std::string TAG_ISSUE_TIME_ISRQ = "0040|2005";
  const std::string TAG_PLACER_ORDER_NO_ISRQ = "0040|2006";
  const std::string TAG_FILLER_ORDER_NO_ISRQ = "0040|2007";
  const std::string TAG_ORDER_ENTEREDBY = "0040|2008";
  const std::string TAG_ORDER_ENTERER = "0040|2009";
  const std::string TAG_ORDER_CALLBACK_TEL = "0040|2010";
  const std::string TAG_ISRQ_COMMENTS = "0040|2400";
  const std::string TAG_CONFID_CONSTRAIN_PAT_DESC = "0040|3001";
  const std::string TAG_RELATIONSHIP_TYPE = "0040|a010";
  const std::string TAG_VERIFYING_ORGANIZATION = "0040|a027";
  const std::string TAG_VERIFICATION_DATE_TIME = "0040|a030";
  const std::string TAG_OBSERVATION_DATE_TIME = "0040|a032";
  const std::string TAG_VALUE_TYPE = "0040|a040";
  const std::string TAG_CONCEPT_NAME_CODE_SEQ = "0040|a043";
  const std::string TAG_CONTINUITY_OF_CONTENT = "0040|a050";
  const std::string TAG_VERIFYING_OBSERVER_SEQ = "0040|a073";
  const std::string TAG_VERIFYING_OBSERVER_NAME = "0040|a075";
  const std::string TAG_VERF_OBSERVER_ID_CODE_SEQ = "0040|a088";
  const std::string TAG_DATE_TIME = "0040|a120";
  const std::string TAG_MEASURED_VALUE_SEQ = "0040|a300";
  const std::string TAG_PREDECESSOR_DOCUMENTS_SEQ = "0040|a360";
  const std::string TAG_REFERENCED_REQUEST_SEQ = "0040|a370";
  const std::string TAG_PERFORMED_PROCEDURE_CODE_SEQ = "0040|a372";
  const std::string TAG_REQEUSTED_PROCEDURE_EVIDENCE_SEQ = "0040|a375";
  const std::string TAG_PERTINENT_OTHER_EVIDENCE_SEQ = "0040|a385";
  const std::string TAG_COMPLETION_FLAG = "0040|a491";
  const std::string TAG_COMPLETION_FLAG_DESC = "0040|a492";
  const std::string TAG_VERIFICATION_FLAG = "0040|a493";
  const std::string TAG_CONTENT_TEMPLATE_SEQ = "0040|a504";
  const std::string TAG_IDENTICAL_DOCUMENTS_SEQ = "0040|a525";
  const std::string TAG_CONTENT_SEQ = "0040|a730";
  const std::string TAG_TEMPLATE_ID = "0040|db00";
  const std::string TAG_TEMPLATE_VERSION = "0040|db06";
  const std::string TAG_TEMPLATE_LOCAL_VERSION = "0040|db07";
  const std::string TAG_TEMPLATE_EXTENSION_FLAG = "0040|db0b";
  const std::string TAG_TEMPLATE_EXTENSION_ORGANIZATION_UID = "0040|db0c";
  const std::string TAG_TEMPLATE_EXTENSION_CREATOR_UID = "0040|db0d";
  const std::string TAG_REFERENCED_CONTENT_ITEM_ID = "0040|db73";
  const std::string TAG_GRAPHIC_DATA = "0070|0022";
  const std::string TAG_GRAPHIC_TYPE = "0070|0023";
  const std::string TAG_FILM_SESSION_LABEL = "2000|0050";
  const std::string TAG_EXECUTION_STATUS = "2100|0020";
  const std::string TAG_PRESENTATION_LUT_SHAPE = "2050|0020";
  const std::string TAG_PRINTER_NAME = "2110|0030";
  const std::string TAG_PRINT_QUEUE_ID = "2110|0099";
  const std::string TAG_RESULT_ID = "4008|0040";
  const std::string TAG_INTERPRET_TRANS_DATE = "4008|0108";
  const std::string TAG_INTERPRET_AUTHOR = "4008|010C";
  const std::string TAG_INTERPRET_DIAG_DESC = "4008|0115";
  const std::string TAG_INTERPRET_DIAG_CODE_SEQ = "4008|0117";
  const std::string TAG_INTERPRET_ID = "4008|0200";
  const std::string TAG_INTERPRET_TYPE_ID = "4008|0210";
  const std::string TAG_INTERPRET_STATUS_ID = "4008|0212";
  const std::string TAG_ROI_AREA = "6000|1301";
  const std::string TAG_ROI_MEAN = "6000|1302";
  const std::string TAG_ROI_STDDEV = "6000|1303";
  const std::string TAG_PIXEL_DAT = "7fe0|0010";
  const std::string TAG_EX_INFO = "0051|100E";
  const std::string TAG_SLICE_THICKNESS = "0018|0050";

  static const std::vector<std::string> dicomTagsList = {
    "0008|0008", // TAG_IMAGE_TYPE
    "0008|0012", // TAG_INST_CREATE_DATE
    "0008|0013", // TAG_INST_CREATE_TIME
    "0008|0014", // TAG_INST_CREATOR_UID
    "0008|0016", // TAG_SOP_CLASS_UID
    "0008|0018", // TAG_SOP_INST_UID
    "0008|0020", // TAG_STUDY_DATE
    "0008|0021", // TAG_SERIES_DATE
    "0008|0022", // TAG_ACQUISITION_DATE
    "0008|0023", // TAG_IMAGE_DATE
    "0080|002a", // TAG_ACQUISITION_DATE_TIME
    "0008|0030", // TAG_STUDY_TIME
    "0008|0031", // TAG_SERIES_TIME
    "0008|0032", // TAG_ACQUISITION_TIME
    "0008|0033", // TAG_IMAGE_TIME
    "0008|0050", // TAG_ACCESS_NO
    "0008|0052", // TAG_QR_LEVEL
    "0008|0054", // TAG_RETR_AET
    "0008|0056", // TAG_INST_AVAILABILITY
    "0008|0058", // TAG_FAILED_SOPINSTUID_LIST
    "0008|0060", // TAG_MODALITY
    "0008|0064", // TAG_CONVERSION_TYPE
    "0008|0068", // TAG_PRESENTATION_TYPE
    "0008|0070", // TAG_MANUFACTOR
    "0008|0080", // TAG_INSTITUT_NAME
    "0008|0081", // TAG_INSTITUT_ADDRESS
    "0008|0090", // TAG_REF_PHYS_NAME
    "0008|0092", // TAG_REF_PHYS_ADDR
    "0008|0094", // TAG_REF_PHYS_TEL
    "0008|0100", // TAG_CODE_VALUE
    "0008|0102", // TAG_CODING_SCHEME_DESIGNATOR
    "0008|0104", // TAG_CODE_MEANING
    "0008|1010", // TAG_STATION_NAME
    "0008|1030", // TAG_STUDY_DESC
    "0008|1032", // TAG_PROC_CODE_SEQ
    "0008|103e", // TAG_SERIES_DESC
    "0008|1040", // TAG_INSTITUT_DEPT_NAME
    "0008|1050", // TAG_PERF_PHYS_NAME
    "0008|1060", // TAG_READ_PHYS_NAME
    "0008|1070", // TAG_OPERATOR_NAME
    "0008|1080", // TAG_ADMIT_DIAG_DESC
    "0008|1090", // TAG_MANFAC_MODEL_NAME
    "0008|1110", // TAG_REF_STUDY_SEQ
    "0008|1111", // TAG_REF_STUDY_COMPONENT_SEQ
    "0008|1120", // TAG_REF_PAT_SEQ
    "0008|1140", // TAG_REF_IMAGE_SEQ
    "0008|1150", // TAG_REF_SOP_CLASS_UID
    "0008|1155", // TAG_REF_SOP_INST_UID
    "0008|1199", // TAG_REF_SOP_SEQ
    "0010|0010", // TAG_PAT_NAME
    "0010|0020", // TAG_PAT_ID
    "0010|0030", // TAG_PAT_BIRTH_DATE
    "0010|0040", // TAG_PAT_SEX
    "0010|1000", // TAG_OTHER_PAT_ID
    "0010|1010", // TAG_PAT_AGE
    "0010|1020", // TAG_PAT_SIZE
    "0010|1030", // TAG_PAT_WEIGHT
    "0010|1040", // TAG_PAT_ADDR
    "0010|2154", // TAG_PAT_TEL
    "0010|2180", // TAG_OCCUPATION
    "0010|21b0", // TAG_ADDITIONAL_PT_HISTORY
    "0010|4000", // TAG_PAT_COMMENT
    "0012|0010", // TAG_CLINICAL_TRIAL_SPONSOR
    "0012|0021", // TAG_CLINICAL_TRIAL_PROT_NAME
    "0012|0031", // TAG_CLINICAL_TRIAL_SITE_NAME
    "0012|0060", // TAG_CLINICAL_TRIAL_CENTER
    "0012|0020", // TAG_CLINICAL_TRIAL_PROT_ID
    "0012|0030", // TAG_CLINICAL_TRIAL_SITE_ID
    "0012|0030", // TAG_CLINICAL_TRIAL_SUBJECT_ID
    "0012|0030", // TAG_CLINICAL_TRIAL_SUBJECT_READING_ID
    "0018|0015", // TAG_BODY_PART_EXAMINED
    "0018|0059", // TAG_SLANT_ANGLE
    "0018|0060", // TAG_KVP
    "0018|1000", // TAG_DEVICE_SERIAL_NUMBER
    "0018|1004", // TAG_PLATE_ID
    "0018|1010", // TAG_SECONDARY_CAPTURE_DEVICE_ID
    "0018|1016", // TAG_SECONDARY_CAPTURE_DEVICE_MANUFACTURER
    "0018|1020", // TAG_SOFTWARE_VERSION
    "0018|1012", // TAG_SECONDARY_CAPTURE_DATE
    "0018|1014", // TAG_SECONDARY_CAPTURE_TIME
    "0018|1017", // TAG_HARDCOPY_DEVICE_MANUFACTURER
    "0018|1018", // TAG_SECONDARY_CAPTURE_DEVICE_MODEL_NAME
    "0018|1019", // TAG_SECONDARY_CAPTURE_DEVICE_SOFTWARE_VERSION
    "0018|1022", // TAG_VIDEO_IMAGE_FORMAT_ACQUIRED
    "0018|1030", // TAG_PROT_NAME
    "0018|1150", // TAG_EXPOSURE_TIME
    "0018|1151", // TAG_XRAYTUBE_CURRENT
    "0018|1152", // TAG_EXPOSURE_MAS
    "0018|1160", // TAG_FILTER_TYPE
    "0018|1164", // TAG_IMGR_PIXEL_SPACING
    "0018|1191", // TAG_ANODE_MATERIAL
    "0018|1110", // TAG_DIST_SOURCE_DETECTOR
    "0018|1111", // TAG_DIST_SOURCE_PATIENT
    "0018|11a0", // TAG_BODY_PART_THICKNESS
    "0018|11a2", // TAG_COMPRESSION_FORCE
    "0018|1114", // TAG_ESTIMATED_RADIOGRAPHIC_MAGNIFICATION_FACTOR
    "0018|1147", // TAG_FIELD_OF_VIEW_SHAPE
    "0018|1149", // TAG_FIELD_OF_VIEW_DIMENSION
    "0018|1166", // TAG_GRID
    "0018|1190", // TAG_FOCAL_SPOT
    "0018|1405", // TAG_REL_X_RAY_EXPOSURE
    "0018|1400", // TAG_ACQUISITION_DEVICE_PROCESSING_DESCRIPTION
    "0018|1401", // TAG_ACQUISITION_DEVICE_PROCESSING_CODE
    "0018|1405", // TAG_RELATIVE_XRAY_EXPOSURE
    "0018|1508", // TAG_POSITIONER_TYPE
    "0018|1510", // TAG_POSITIONER_PRIMARY_ANGLE
    "0018|1530", // TAG_DETECTOR_PRIMARY_ANGLE
    "0018|1531", // TAG_SECONDARY_DET_ANGLE
    "0018|1700", // TAG_COLLIMATOR_SHAPE
    "0018|1702", // TAG_COLLIMATOR_LEFT_EDGE
    "0018|1704", // TAG_COLLIMATOR_RIGHT_EDGE
    "0018|1706", // TAG_COLLIMATOR_UPPER_EDGE
    "0018|1708", // TAG_COLLIMATOR_LOWER_EDGE
    "0018|2010", // TAG_NOMINAL_SCANNED_PXL_SPACING
    "0018|700c", // TAG_LAST_DETECTOR_CALIBERATION_DATE
    "0018|6000", // TAG_SENSITIVITY
    "0018|5101", // TAG_VIEW_POSITION
    "0018|7000", // TAG_DETECTOR_CONDITIONS_NOMINAL_FLAG
    "0018|7001", // TAG_DETECTOR_TEMP
    "0018|7004", // TAG_DETECTOR_TYPE
    "0018|700A", // TAG_DETECTOR_ID
    "0018|7005", // TAG_DETECTOR_CONFIG
    "0018|701a", // TAG_DETECTOR_BINNING
    "0018|7020", // TAG_DETECTOR_ELEMENT_PHYSICAL_SIZE
    "0018|7022", // TAG_DETECTOR_ELEMENT_SPACING
    "0018|7024", // TAG_DETECTOR_ACTIVE_SHAPE
    "0018|7026", // TAG_DETECTOR_ACTIVE_DIMENSION
    "0018|7030", // TAG_FIELD_OF_VIEW_ORIGIN
    "0018|7032", // TAG_FIELD_OF_VIEW_ROTATION
    "0018|7034", // TAG_FIELD_OF_VIEW_HORIZONTAL_FLIP
    "0018|7050", // TAG_FILTER_MATERIAL
    "0018|7052", // TAG_FILTER_THICKNESS_MIN
    "0018|7054", // TAG_FILTER_THICKNESS_MAX
    "0018|7060", // TAG_EXPOSURE_CONTROL_MODE
    "0018|7062", // TAG_EXPOSURE_CONTROL_MODE_DESC
    "0018|7064", // TAG_EXPOSURE_STATUS
    //X-Ray Iso-Centre Reference System Module - Added by Aili
    "0018|9462", // TAG_ISOCENTRE_REF_SEQUENCE
    "0018|9463", // TAG_ISOCENTRE_PRIMARY_ANGLE
    "0018|9464", // TAG_ISOCENTRE_SECONDARY_ANGLE
    "0018|9465", // TAG_ISOCENTRE_DETECTOR_ROT_ANGLE
    "0018|9466", // TAG_TABLE_X_POS_TO_ISO_CTR
    "0018|9467", // TAG_TABLE_Y_POS_TO_ISO_CTR
    "0018|9468", // TAG_TABLE_Z_POS_TO_ISO_CTR
    "0018|9469", // TAG_TABLE_HORIZ_ROT_ANGLE
    "0018|9470", // TAG_TABLE_HEAD_TILT_ANGLE
    "0018|9471", // TAG_TABLE_CRADLE_TILT_ANGLE
    //Following are Sunnybrook Private tags for Tomo Sandbox
    "0019|0010", // TAG_SANDBOX_PRIVATE_CREATOR
    "0019|1001", // TAG_MOTOR_SOURCE_LONGITUDINAL
    "0019|1002", // TAG_MOTOR_SOURCE_TRANSVERSE
    "0019|1003", // TAG_MOTOR_DETECTOR_LONGITUDINAL
    "0019|1004", // TAG_MOTOR_DETECTOR_TRANSVERSE
    "0019|1005", // TAG_MOTOR_SOURCE_ROTATIONAL
    "0019|1006", // TAG_MOTOR_DETECTOR_ROTATIONAL
    "0019|1007", // TAG_MOTOR_OBJECT_ROTATIONAL
    "0019|1008", // TAG_NUMBER_SPITS
    "0019|1009", // TAG_NUMBER_FRAMES
    "0020|000d", // TAG_STUDY_INST_UID
    "0020|000e", // TAG_SERIES_INST_UID
    "0020|0010", // TAG_STUDY_ID
    "0020|0011", // TAG_SERIES_NO
    "0020|0012", // TAG_ACQUISITION_NO
    "0020|0013", // TAG_IMAGE_NO
    "0020|0020", // TAG_PATIENT_ORIENTATION
    "0020|0022", // TAG_OVERLAY_NO
    "0020|0024", // TAG_CURVE_NO
    "0020|0026", // TAG_LUT_NO
    "0020|0032", // TAG_IMAGE_POSITION
    "0020|0037", // TAG_IMAGE_ORIENTATION
    "0020|0060", // TAG_LATERALITY
    "0020|0062", // TAG_IMAGE_LATERALITY
    "0020|1200", // TAG_NO_PAT_REL_STUDIES
    "0020|1202", // TAG_NO_PAT_REL_SERIES
    "0020|1204", // TAG_NO_PAT_REL_IMAGES
    "0020|1206", // TAG_NO_STUDY_REL_SERIES
    "0020|1208", // TAG_NO_STUDY_REL_IMAGES
    "0020|1209", // TAG_NO_SERIES_REL_IMAGES
    "0020|9128", // TAG_TEMPORAL_INDEX
    "0028|0002", // TAG_SAMPLES_PER_PX
    "0028|0004", // TAG_PHOTOMETRIC
    "0028|0010", // TAG_ROWS
    "0028|0011", // TAG_COLUMNS
    "0028|0012", // TAG_PLANES
    "0028|0030", // TAG_PIXEL_SPACING
    "0028|0100", // TAG_BITS_ALLOC
    "0028|0101", // TAG_BITS_STORED
    "0028|0102", // TAG_HIGH_BIT
    "0028|0103", // TAG_PX_REPRESENT
    "0028|0106", // TAG_SMALLEST_IMG_PX_VALUE
    "0028|0107", // TAG_LARGEST_IMG_PX_VALUE
    "0028|0300", // TAG_QUALITY_CONTROL_IMAGE
    "0028|0301", // TAG_BURNED_IN_ANNOTATION
    "0028|1040", // TAG_PXL_INTENSITY_RELATIONSHIP
    "0028|1041", // TAG_PXL_INTENSITY_SIGN
    "0028|1050", // TAG_WINDOW_CENTER
    "0028|1051", // TAG_WINDOW_WIDTH
    "0028|1052", // TAG_RESCALE_INTERCEPT
    "0028|1053", // TAG_RESCALE_SLOPE
    "0028|1054", // TAG_RESCALE_TYPE
    "0028|1201", // TAG_RED_PAL_LUT
    "0028|1202", // TAG_GREEN_PAL_LUT
    "0028|1203", // TAG_BLUE_PAL_LUT
    "0028|1221", // TAG_SEG_RED_PAL_LUT
    "0028|1222", // TAG_SEG_GREEN_PAL_LUT
    "0028|1223", // TAG_SEG_BLUE_PAL_LUT
    "0028|2110", // TAG_LOSSY_IMAGE_COMPRESSION
    "0088|0200", // TAG_ICON_IMAGE_SEQ
    "0088|0904", // TAG_TOPIC_TITLE
    "0088|0906", // TAG_TOPIC_SUBJECT
    "0088|0910", // TAG_TOPIC_AUTHOR
    "0088|0912", // TAG_TOPIC_KEYWORDS
    "0032|000a", // TAG_STUDY_STATUS_ID
    "0032|1032", // TAG_REQ_PHYS
    "0032|1033", // TAG_REQ_SERVICE
    "0032|1060", // TAG_REQ_PROC_DESC
    "0032|1070", // TAG_REQ_CONTRAST_AGENT
    "0038|0010", // TAG_ADMISSION_ID
    "0038|0016", // TAG_ROUTE_OF_ADMISS
    "0038|0020", // TAG_ADMIT_DATE
    "0040|0001", // TAG_SPS_STATION_AET
    "0040|0002", // TAG_SPS_START_DATE
    "0040|0003", // TAG_SPS_START_TIME
    "0040|0004", // TAG_SPS_END_DATE
    "0040|0005", // TAG_SPS_END_TIME
    "0040|0006", // TAG_SPS_PERF_PHYS_NAME
    "0040|0007", // TAG_SPS_DESC
    "0040|0008", // TAG_SPS_ACTION_SEQ
    "0040|0009", // TAG_SPS_ID
    "0040|000a", // TAG_SPS_STAGE_CODE_SQ
    "0040|000b", // TAG_SPS_PERF_PHYSICIAN_ID_SQ
    "0040|0010", // TAG_SPS_STATION_NAME
    "0040|0011", // TAG_SPS_LOCATION
    "0040|0012", // TAG_PRE_MEDICATION
    "0040|0020", // TAG_SPS_STATUS
    "0040|0100", // TAG_SPS_SEQ
    "0040|0220", // TAG_REF_STANDALONE_SOP_INST_SEQ
    "0040|0241", // TAG_PERF_STATION_AET
    "0040|0242", // TAG_PERF_STATION_NAME
    "0040|0243", // TAG_PERF_LOCATION
    "0040|0244", // TAG_PPS_START_DATE
    "0040|0245", // TAG_PPS_START_TIME
    "0040|0250", // TAG_PPS_END_DATE
    "0040|0251", // TAG_PPS_END_TIME
    "0040|0252", // TAG_PPS_STATUS
    "0040|0253", // TAG_PPS_ID
    "0040|0254", // TAG_PPS_DESC
    "0040|0255", // TAG_PERF_TYPE_DESC
    "0040|0260", // TAG_PERF_ACTION_SEQ
    "0040|0270", // TAG_SPS_ATTRIB_SEQ
    "0040|0275", // TAG_REQ_ATTRIB_SEQ
    "0040|0280", // TAG_COMMENT_PPS
    "0040|0293", // TAG_QUANTITY_SEQ
    "0040|0294", // TAG_QUANTITY
    "0040|0295", // TAG_MEASURING_UNITS_SEQ
    "0040|0296", // TAG_BILLING_ITEM_SEQ
    "0040|0300", // TAG_TOT_TIME_FLUOROS
    "0040|0301", // TAG_TOT_NUM_EXPOS
    "0040|0302", // TAG_ENTRANCE_DOSE
    "0040|0303", // TAG_EXPOSED_AREA
    "0040|0306", // TAG_DISTANCE_SOURCE_ENTRANCE
    "0040|0310", // TAG_COMMENT_RADIATION_DOSE
    "0040|0312", // TAG_X_RAY_OUTPUT
    "0040|0314", // TAG_HALF_VALUE_LAYER
    "0040|0316", // TAG_ORGAN_DOSE
    "0040|0318", // TAG_ORGAN_EXPOSED
    "0040|0320", // TAG_BILLING_PPS_SEQ
    "0040|0321", // TAG_FILM_CONSUM_SEQ
    "0040|0324", // TAG_BILLING_SUPPL_DEVICES_SEQ
    "0040|0330", // TAG_REF_PPS_SEQ
    "0040|0340", // TAG_PERF_SERIES_SEQ
    "0040|0400", // TAG_COMMENTS_ON_SPS
    "0040|08EA", // TAG_MEAS_UNIT_SQ
    "0040|1001", // TAG_REQ_PROC_ID
    "0040|1002", // TAG_REASON_REQ_PROC
    "0040|1003", // TAG_REQ_PROC_PRIORITY
    "0040|1004", // TAG_PAT_TRANS_ARRANGE
    "0040|1005", // TAG_REQ_PROCEDURE_LOC
    "0040|1006", // TAG_PLACER_ORDER_NUM_PROC
    "0040|1007", // TAG_FILLER_ORDER_NUM_PROC
    "0040|1008", // TAG_CONFID_CODE
    "0040|1009", // TAG_REPORT_PRIORITY
    "0040|1010", // TAG_RECIPIENTS_OF_RESULT
    "0040|1400", // TAG_REQ_PROC_COMMENT
    "0040|2001", // TAG_REASON_ISRQ
    "0040|2004", // TAG_ISSUE_DATE_ISRQ
    "0040|2005", // TAG_ISSUE_TIME_ISRQ
    "0040|2006", // TAG_PLACER_ORDER_NO_ISRQ
    "0040|2007", // TAG_FILLER_ORDER_NO_ISRQ
    "0040|2008", // TAG_ORDER_ENTEREDBY
    "0040|2009", // TAG_ORDER_ENTERER
    "0040|2010", // TAG_ORDER_CALLBACK_TEL
    "0040|2400", // TAG_ISRQ_COMMENTS
    "0040|3001", // TAG_CONFID_CONSTRAIN_PAT_DESC
    "0040|a010", // TAG_RELATIONSHIP_TYPE
    "0040|a027", // TAG_VERIFYING_ORGANIZATION
    "0040|a030", // TAG_VERIFICATION_DATE_TIME
    "0040|a032", // TAG_OBSERVATION_DATE_TIME
    "0040|a040", // TAG_VALUE_TYPE
    "0040|a043", // TAG_CONCEPT_NAME_CODE_SEQ
    "0040|a050", // TAG_CONTINUITY_OF_CONTENT
    "0040|a073", // TAG_VERIFYING_OBSERVER_SEQ
    "0040|a075", // TAG_VERIFYING_OBSERVER_NAME
    "0040|a088", // TAG_VERF_OBSERVER_ID_CODE_SEQ
    "0040|a120", // TAG_DATE_TIME
    "0040|a300", // TAG_MEASURED_VALUE_SEQ
    "0040|a360", // TAG_PREDECESSOR_DOCUMENTS_SEQ
    "0040|a370", // TAG_REFERENCED_REQUEST_SEQ
    "0040|a372", // TAG_PERFORMED_PROCEDURE_CODE_SEQ
    "0040|a375", // TAG_REQEUSTED_PROCEDURE_EVIDENCE_SEQ
    "0040|a385", // TAG_PERTINENT_OTHER_EVIDENCE_SEQ
    "0040|a491", // TAG_COMPLETION_FLAG
    "0040|a492", // TAG_COMPLETION_FLAG_DESC
    "0040|a493", // TAG_VERIFICATION_FLAG
    "0040|a504", // TAG_CONTENT_TEMPLATE_SEQ
    "0040|a525", // TAG_IDENTICAL_DOCUMENTS_SEQ
    "0040|a730", // TAG_CONTENT_SEQ
    "0040|db00", // TAG_TEMPLATE_ID
    "0040|db06", // TAG_TEMPLATE_VERSION
    "0040|db07", // TAG_TEMPLATE_LOCAL_VERSION
    "0040|db0b", // TAG_TEMPLATE_EXTENSION_FLAG
    "0040|db0c", // TAG_TEMPLATE_EXTENSION_ORGANIZATION_UID
    "0040|db0d", // TAG_TEMPLATE_EXTENSION_CREATOR_UID
    "0040|db73", // TAG_REFERENCED_CONTENT_ITEM_ID
    "0070|0022", // TAG_GRAPHIC_DATA
    "0070|0023", // TAG_GRAPHIC_TYPE
    "2000|0050", // TAG_FILM_SESSION_LABEL
    "2100|0020", // TAG_EXECUTION_STATUS
    "2050|0020", // TAG_PRESENTATION_LUT_SHAPE
    "2110|0030", // TAG_PRINTER_NAME
    "2110|0099", // TAG_PRINT_QUEUE_ID
    "4008|0040", // TAG_RESULT_ID
    "4008|0108", // TAG_INTERPRET_TRANS_DATE
    "4008|010C", // TAG_INTERPRET_AUTHOR
    "4008|0115", // TAG_INTERPRET_DIAG_DESC
    "4008|0117", // TAG_INTERPRET_DIAG_CODE_SEQ
    "4008|0200", // TAG_INTERPRET_ID
    "4008|0210", // TAG_INTERPRET_TYPE_ID
    "4008|0212", // TAG_INTERPRET_STATUS_ID
    "6000|1301", // TAG_ROI_AREA
    "6000|1302", // TAG_ROI_MEAN
    "6000|1303", // TAG_ROI_STDDEV
    "7fe0|0010", // TAG_PIXEL_DATA
    "0051|100E", // TAG_EX_INFO
    "0018|0050", // TAG_SLICE_THICKNESS

    // Diffusion vendor-specific tags
    "0043|0010",
    "0043|1039",
    "0019|10bb",
    "0019|10bc",
    "0019|10bd",
    "0029|1010",
    "0018|9087",
    "0021|1146",
    "0052|9230",
    "0018|9117",
    "0021|9230",
    "2001|1003",
    "0018|9087",
    "0018|9089"
  };

  static const std::map<std::string, std::string> tagToPropertyMap = {
    // Patient module
    std::make_pair("0010|0010", "dicom.patient.PatientsName"),
    std::make_pair("0010|0020", "dicom.patient.PatientID"),
    std::make_pair("0010|0030", "dicom.patient.PatientsBirthDate"),
    std::make_pair("0010|0040", "dicom.patient.PatientsSex"),
    std::make_pair("0010|0032", "dicom.patient.PatientsBirthTime"),
    std::make_pair("0010|1000", "dicom.patient.OtherPatientIDs"),
    std::make_pair("0010|1001", "dicom.patient.OtherPatientNames"),
    std::make_pair("0010|2160", "dicom.patient.EthnicGroup"),
    std::make_pair("0010|4000", "dicom.patient.PatientComments"),
    std::make_pair("0012|0062", "dicom.patient.PatientIdentityRemoved"),
    std::make_pair("0012|0063", "dicom.patient.DeIdentificationMethod"),

    // General Study module
    std::make_pair("0020|000d", "dicom.study.StudyInstanceUID"),
    std::make_pair("0008|0020", "dicom.study.StudyDate"),
    std::make_pair("0008|0030", "dicom.study.StudyTime"),
    std::make_pair("0008|0070", "dicom.study.Manufacturer"),
    std::make_pair("0008|0080", "dicom.study.InstitutionName"),
    std::make_pair("0008|0090", "dicom.study.ReferringPhysiciansName"),
    std::make_pair("0020|0010", "dicom.study.StudyID"),
    std::make_pair("0008|0050", "dicom.study.AccessionNumber"),
    std::make_pair("0008|1030", "dicom.study.StudyDescription"),
    std::make_pair("0008|1048", "dicom.study.PhysiciansOfRecord"),
    std::make_pair("0008|1060", "dicom.study.NameOfPhysicianReadingStudy"),

    //General Acquisition module
    std::make_pair("0008|0022", "dicom.acquisition.Date"),
    std::make_pair("0008|0032", "dicom.acquisition.Time"),
    std::make_pair("0020|0012", "dicom.acquisition.Number"),
    std::make_pair("0020|0013", "dicom.image.Number"),

    // General Series module
    std::make_pair("0008|0060", "dicom.series.Modality"),
    std::make_pair("0020|000e", "dicom.series.SeriesInstanceUID"),
    std::make_pair("0020|0011", "dicom.series.SeriesNumber"),
    std::make_pair("0020|0060", "dicom.series.Laterality"),
    std::make_pair("0020|4000", "dicom.series.ImageComments"),
    std::make_pair("0008|0021", "dicom.series.SeriesDate"),
    std::make_pair("0008|0031", "dicom.series.SeriesTime"),
    std::make_pair("0008|1050", "dicom.series.PerformingPhysiciansName"),
    std::make_pair("0018|1030", "dicom.series.ProtocolName"),
    std::make_pair("0008|103e", "dicom.series.SeriesDescription"),
    std::make_pair("0008|1070", "dicom.series.OperatorsName"),
    std::make_pair("0018|0015", "dicom.series.BodyPartExamined"),
    std::make_pair("0018|5100", "dicom.series.PatientPosition"),
    std::make_pair("0028|0108", "dicom.series.SmallestPixelValueInSeries"),
    std::make_pair("0028|0109", "dicom.series.LargestPixelValueInSeries"),
    std::make_pair("0008|0016", "dicom.series.SopClassUid"),
    std::make_pair("0020|9128", "dicom.series.TemporalPositionIndex"),

    // VOI LUT module
    std::make_pair("0028|1050", "dicom.voilut.WindowCenter"),
    std::make_pair("0028|1051", "dicom.voilut.WindowWidth"),
    std::make_pair("0028|1055", "dicom.voilut.WindowCenterAndWidthExplanation"),

    // Image Pixel module
    std::make_pair("0028|0004", "dicom.pixel.PhotometricInterpretation"),
    std::make_pair("0028|0010", "dicom.pixel.Rows"),
    std::make_pair("0028|0011", "dicom.pixel.Columns"),

    // Image Plane module
    std::make_pair("0028|0030", "dicom.PixelSpacing"),
    std::make_pair("0018|1164", "dicom.ImagerPixelSpacing"),

    // Misc
    std::make_pair("0008|0005", "dicom.SpecificCharacterSet"),
    std::make_pair("0018|0080", "dicom.TR"),
    std::make_pair("0018|0081", "dicom.TE"),
    std::make_pair("0018|0087", "dicom.MagneticFieldStrength"),
    std::make_pair("0051|100E", "dicom.ExInfo"),
    std::make_pair("0018|0050", "dicom.SliceThickness"),
    std::make_pair("0018|1151", "dicom.XrayTubeCurrent"),
    std::make_pair("0018|0060", "dicom.Kvp"),
    std::make_pair("0020|0032", "dicom.ImagePosition")
  };

  static const std::vector<std::string> propertiesToLocalize = {
    // Patient module
    "dicom.patient.PatientsName",
    "dicom.patient.PatientsBirthDate",
    "dicom.patient.PatientsSex",
    "dicom.patient.PatientsBirthTime",
    "dicom.patient.OtherPatientNames",
    "dicom.patient.EthnicGroup",
    "dicom.patient.PatientComments",
    "dicom.patient.PatientIdentityRemoved",
    "dicom.patient.DeIdentificationMethod",

    // General Study module
    "dicom.study.InstitutionName",
    "dicom.study.StudyDescription",

    // General Series module
    "dicom.series.SeriesDescription",
    "dicom.series.BodyPartExamined"
  };
}
