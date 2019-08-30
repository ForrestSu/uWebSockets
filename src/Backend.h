#ifndef BACKEND_H
#define BACKEND_H

// Default to Libuv
#ifdef USE_ASIO
#include "Asio.h"
#elif defined(USE_EPOLL)
#include "Epoll.h"
#else
#include "Libuv.h"
#endif

#endif // BACKEND_H
