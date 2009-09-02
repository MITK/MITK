MACRO_CREATE_PLUGIN()

ADD_TEST(${BUNDLE-SYMBOLICNAME} ${OSGI_APP} --openCherry.application=coretestapplication --openCherry.testplugin=${BUNDLE-SYMBOLICNAME})