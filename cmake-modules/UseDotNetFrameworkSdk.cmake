#
# A CMake Module for using C# .NET.
#
# The following variables are set:
#   (none)
#
# This file is based on the work of GDCM:
#   http://gdcm.svn.sf.net/viewvc/gdcm/trunk/CMake/UseDotNETFrameworkSDK.cmake
# Copyright (c) 2006-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#

set( CSHARP_COMPILER ${CSHARP_DOTNET_COMPILER_${CSHARP_DOTNET_VERSION}} )
message( STATUS "Using .NET compiler version ${CSHARP_DOTNET_VERSION}" )
