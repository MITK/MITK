QT_BUILD_DIR=/opt/toolkits/ubuntu-12.04/qt-5.1/lib/

for lib in $QT_BUILD_DIR/libQt5*.so
do
  libname=${lib/*libQt5/Qt5}
  libname=${libname/.so/}

  config_filename=MITK_${libname}_Config.cmake
  echo "Creating ${config_filename}"
  cat << EOF > ${config_filename}
find_package($libname REQUIRED)
qt5_use_modules(\${MODULE_NAME} ${libname#Qt5})
EOF

done
