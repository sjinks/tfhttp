#include "stdafx.h"
#include "tlsutils.h"

void dispose_tls_context(tls* ctx)
{
    tls_close(ctx);
    tls_free(ctx);
}
