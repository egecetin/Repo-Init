# From https://stackoverflow.com/questions/35745344/cmake-target-version-increment

set(CACHE_FILE "../BuildNumberCache.txt")

# Reading data from file + incrementation
if(EXISTS ${CACHE_FILE})
  file(READ ${CACHE_FILE} INCREMENTED_VALUE)
  math(EXPR INCREMENTED_VALUE "${INCREMENTED_VALUE}+1")
else()
  set(INCREMENTED_VALUE "1")
endif()

# Update the cache
file(WRITE ${CACHE_FILE} "${INCREMENTED_VALUE}")
set(BUILD_VERSION ${INCREMENTED_VALUE})
