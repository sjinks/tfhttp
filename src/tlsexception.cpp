#include "tlsexception.h"

// This is to ensure that the vtable is only emitted once.
TLSException::~TLSException() = default;
