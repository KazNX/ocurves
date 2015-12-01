///
/// @author Kazys Stepanas
///
/// Include to (temporarily) disable memory tracking.
/// @see alloctrack
///
/// Copyright (c) CSIRO 2012
///

// No ifdef guard

#ifdef new
#undef new
#endif // new

#ifdef malloc
#undef malloc
#endif // malloc

#ifdef realloc
#undef realloc
#endif // realloc

#ifdef calloc
#undef calloc
#endif // calloc

#ifdef free
#undef free
#endif // free
