# Copyright (C) 2013 Max Oberberger <max@oberbergers.de>
#
# Last modified: 2013 February 23, 20:05:02 by max
#
# This file is part of ba-hla.
# 
# ba-hla is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# ba-hla is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ba-hla.  If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 2.8)
project(ba-hla)

set(cmake_verbose_makefile ON)
set(cmake_c_flags_debug ON)
set(cmake_cxx_flags_debug ON)

## enable debugging
set(cmake_build_type Debug)

include_directories(/usr/local/include/rti1516)
add_library(ba-hla-lib ExampleFedAmb.cpp ExampleCPPFederate.cpp)
add_executable(ba-hla main.cpp)
target_link_libraries(ba-hla ba-hla-lib rti1516 fedtime1516)

install(TARGETS ba-hla DESTINATION bin)
