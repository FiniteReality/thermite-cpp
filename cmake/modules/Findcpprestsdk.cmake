include(LibFindMacros)

libfind_package(cpprestsdk OpenSSL)

libfind_pkg_detect(
    cpprestsdk cpprest
    FIND_PATH http_client.h
    PATH_SUFFIXES cpprest
    FIND_LIBRARY cpprest
)
set(cpprestsdk_VERSION ${cpprestsdk_PKGCONF_VERSION})
libfind_process(cpprestsdk)
