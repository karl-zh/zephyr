/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_rpmsg_openamp_transport.h"
#include "erpc_transport_setup.h"

using namespace erpc;


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<RpmsgOpenAMPTransport> s_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_rpmsg_openamp_init(const char *portName, long baudRate)
{
    const uint8_t vtime = 0;
    const uint8_t vmin = 1;
    s_transport.construct(portName, baudRate);
    if (s_transport->init(vtime, vmin) == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_transport.get());
    }
    return NULL;
}
