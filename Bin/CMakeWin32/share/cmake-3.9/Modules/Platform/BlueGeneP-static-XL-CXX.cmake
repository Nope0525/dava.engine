# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


__BlueGeneP_set_static_flags(XL CXX)

# -qhalt=s       = Halt on severe error messages
string(APPEND CMAKE_CXX_FLAGS_INIT " -qhalt=s")
