IMPORTANT
---------

These external libraries are included merely as part of the Visual Studio solution.
They can of course be used with other projects, but the debug binaries are built
with _ITERATOR_DEBUG_LEVEL=0 on the preprocessor line. This means that any subsequent
projects which use these libraries need also define the iterator debug level. Random
errors in stl containers are linking errors complaining of debug level mismatch are
almost certainly down to mixed library versions.