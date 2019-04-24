# $Id: LCGConfig-version.cmake 726426 2016-02-26 14:21:55Z aknue $
#
# This file is used to figure out whether the LCG version that the user
# requested can be delivered or not.
#
# It is steered by two variables. Since CMake's find_package(...) function
# can only accept numeric version values, one can't specify something like
# "81a" as the version unfortunately. To get around it, the user needs to
# specify a variable called LCG_VERSION_POSTFIX if they want to use an LCG
# version that has some custom postfix in its name. In this example
# the variable would have to be set to "a".
#

# Set the LCG root directory:
if( NOT LCG_RELEASE_BASE )
   if( ENV{LCG_RELEASE_BASE} )
      set( LCG_RELEASE_BASE $ENV{LCG_RELEASE_BASE} CACHE
         PATH "Directory holding LCG releases" )
   else()
      set( LCG_RELEASE_BASE "/afs/cern.ch/sw/lcg/releases" CACHE
         PATH "Directory holding LCG releases" )
   endif()
endif()
# Variable used by Gaudi:
set( LCG_releases_base ${LCG_RELEASE_BASE}
   CACHE PATH "Directory holding LCG releases" FORCE )

# If a directory with the requested version string exists, then we're
# done already.
if( EXISTS
      ${LCG_RELEASE_BASE}/LCG_${PACKAGE_FIND_VERSION}${LCG_VERSION_POSTFIX} )
   set( PACKAGE_VERSION ${PACKAGE_FIND_VERSION}${LCG_VERSION_POSTFIX} )
   set( PACKAGE_VERSION_NUMBER ${PACKAGE_FIND_VERSION} )
   set( PACKAGE_VERSION_COMPATIBLE TRUE )
   set( PACKAGE_VERSION_EXACT TRUE )
else()
   # If it doesn't exist, then pick up the latest release, and check if it's
   # newer than the one requested.
   message( FATAL "Can't find the requested LCG version. ")
endif()

# Set the LCG version to be used:
set( LCG_VERSION ${PACKAGE_VERSION}
   CACHE STRING "LCG version used" FORCE )
set( LCG_VERSION_NUMBER ${PACKAGE_VERSION_NUMBER}
   CACHE STRING "LCG version (just the number) used" FORCE )
set( LCG_VERSION_POSTFIX ${LCG_VERSION_POSTFIX}
   CACHE STRING "LCG version postfix used" FORCE )

# Mark all these cache variables as advanced options:
mark_as_advanced( LCG_releases_base LCG_VERSION LCG_VERSION_NUMBER
   LCG_VERSION_POSTFIX )
