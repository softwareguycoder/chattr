// server_exceptions.c - Implementations of the functions found in
// server_exceptions.h that display errors on the screen and then clean up
// the server
//

#include "stdafx.h"
#include "server.h"

#include "server_exceptions.h"
#include "server_functions.h"

void ThrowNullReferenceException() {
    LogError(ERROR_INVALID_PTR_ARG);

    if (GetErrorLogFileHandle() != stderr) {
        fprintf(stderr, ERROR_INVALID_PTR_ARG);
    }

    CleanupServer(ERROR);
}
