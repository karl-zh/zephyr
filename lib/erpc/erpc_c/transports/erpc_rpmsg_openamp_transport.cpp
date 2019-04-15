/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_rpmsg_openamp_transport.h"
#include "erpc_message_buffer.h"
#include <cstdio>
#include <string>
//#include <termios.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

RpmsgOpenAMPTransport::RpmsgOpenAMPTransport(const char *portName, long baudRate)
: proc(0)
, m_serialHandle(0)
, m_portName(portName)
, m_baudRate(baudRate)
{
}

RpmsgOpenAMPTransport::~RpmsgOpenAMPTransport(void)
{
    serial_close(m_serialHandle);
}

erpc_status_t RpmsgOpenAMPTransport::init(uint8_t vtime, uint8_t vmin)
{
    serial_open(m_portName);
    return kErpcStatus_Success;
}

erpc_status_t RpmsgOpenAMPTransport::underlyingSend(const uint8_t *data, uint32_t size)
{
    uint32_t bytesWritten = serial_write(m_serialHandle, (char *)data, size);

    return size != bytesWritten ? kErpcStatus_SendFailed : kErpcStatus_Success;
}
erpc_status_t RpmsgOpenAMPTransport::underlyingReceive(uint8_t *data, uint32_t size)
{
    uint32_t bytesRead = serial_read(m_serialHandle, (char *)data, size);

    return size != bytesRead ? kErpcStatus_ReceiveFailed : kErpcStatus_Success;
}
