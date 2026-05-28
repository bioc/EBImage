# Declarations of all generic methods used in the package

# Copyright (c) 2005-2007 Oleg Sklyar

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation; either version 2.1
# of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# See the GNU Lesser General Public License for more details.
# LGPL license wording: http://www.gnu.org/licenses/lgpl.html

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

# We do not define a generic for `image()` as it's already a generic
# in the `graphics` package.
# See https://github.com/Bioconductor/BiocGenerics/issues/24#issuecomment-4548115193

## statistics
setGeneric ("hist", function (x, ...) standardGeneric("hist") )

# explicit signature definition to avoid dispatching on arguments other than `...`
setGeneric("abind", signature="...")
