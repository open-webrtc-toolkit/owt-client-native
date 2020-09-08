// Copyright (C) <2018> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __MFX_ITT_TRACE_H__
#define __MFX_ITT_TRACE_H__

#ifdef ITT_SUPPORT
#include <ittnotify.h>
#endif


#ifdef ITT_SUPPORT

static inline __itt_domain* mfx_itt_get_domain() {
    static __itt_domain *domain = NULL;

    if (!domain) domain = __itt_domain_create("MFX_SAMPLES");
    return domain;
}

class MFX_ITT_Tracer
{
public:
    MFX_ITT_Tracer(const char* trace_name)
    {
        m_domain = mfx_itt_get_domain();
        if (m_domain)
            __itt_task_begin(m_domain, __itt_null, __itt_null, __itt_string_handle_create(trace_name));
    }
    ~MFX_ITT_Tracer()
    {
        if (m_domain) __itt_task_end(m_domain);
    }
private:
    __itt_domain* m_domain;
};
#define MFX_ITT_TASK(x) MFX_ITT_Tracer __mfx_itt_tracer(x);

#else
#define MFX_ITT_TASK(x)
#endif

#endif //__MFX_ITT_TRACE_H__
