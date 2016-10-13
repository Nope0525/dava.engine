if ( GMOCK_FOUND )
    return ()
endif ()
set ( GMOCK_FOUND 1 )

include (PlatformSettings)
include (GlobalVariables)

set(GMOCK_INCLUDE ${DAVA_THIRD_PARTY_ROOT_PATH}/include/googlemock)
if (WIN32)
  set(GMOCK_LIB_DEBUG ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/gmock.lib)
  set(GMOCK_LIB_RELEASE ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/gmock.lib)
elseif (MACOS)
  set(GMOCK_LIB ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libgmock.a)
endif()
