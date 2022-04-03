set(CACHE_FILE "build/BuildNumberCache.txt")

#Reading data from file + incrementation
IF(EXISTS ${CACHE_FILE})
    file(READ ${CACHE_FILE} INCREMENTED_VALUE)
    math(EXPR INCREMENTED_VALUE "${INCREMENTED_VALUE}+1")
ELSE()
    set(INCREMENTED_VALUE "1")
ENDIF()

#Update the cache
file(WRITE ${CACHE_FILE} "${INCREMENTED_VALUE}")
set(BUILD_VERSION ${INCREMENTED_VALUE})