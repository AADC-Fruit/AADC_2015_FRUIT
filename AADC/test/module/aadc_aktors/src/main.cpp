/**
 *
 * Test starter module.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: VG8D3AW $
 * $Date: 2013-01-11 11:29:24 +0100 (Fr, 11 Jan 2013) $
 * $Revision: 17743 $
 *
 * @remarks
 *
 */
#include "stdafx.h"

ADTF_DEFINE_RUNTIME()

int main(int argc, const char* argv[])
{
    #if (_ENABLE_MEMORY_TRACING)
        cMemoryWatch oMemoryWatcher;
    #endif // _ENABLE_MEMORY_TRACING

    
    #ifndef DISABLE_CPPUNIT

        // create runtime
        int rc = ::RunTestBench(argc, argv);

    #else

    #endif

    return rc;
}
