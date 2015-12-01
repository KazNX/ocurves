//
// author Kazys Stepanas
//
// Copyright (c) 2012 CSIRO
//
#include "mutex.h"

#include <pthread.h>

Mutex::Mutex()
  : impl(0)
{
  // New here won't end up back in the heap tracker because it's not included.
  pthread_mutex_t *m = new pthread_mutex_t;
  pthread_mutex_init(m, 0);
  impl = m;
}


Mutex::~Mutex()
{
  pthread_mutex_t *m = (pthread_mutex_t *)impl;
  pthread_mutex_destroy(m);
  delete m;
}


void Mutex::lock()
{
  pthread_mutex_lock((pthread_mutex_t *)impl);
}


void Mutex::unlock()
{
  pthread_mutex_unlock((pthread_mutex_t *)impl);
}
