# Called by DCMTK.cmake (ExternalProject_Add) as a patch for DCMTK.
# Makes pdf2dcm use "DOC" as modality for Encapsulated PDFs

#message ("Patching dcmdata/apps/pdf2dcm.cc to use modality 'DOC'. Using template ${TEMPLATE_FILE}")

# read whole file
file(STRINGS dcmdata/apps/pdf2dcm.cc sourceCode NEWLINE_CONSUME)

# substitute "OT" for "DOC" (only single occurence where modality tag is set)
string(REGEX REPLACE "\"OT\"" "\"DOC\"" sourceCode ${sourceCode})

# set variable CONTENTS, which is substituted in TEMPLATE_FILE
set(CONTENTS ${sourceCode})
configure_file(${TEMPLATE_FILE} dcmdata/apps/pdf2dcm.cc @ONLY)

