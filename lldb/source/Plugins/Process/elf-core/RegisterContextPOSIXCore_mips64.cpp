//===-- RegisterContextCorePOSIX_mips64.cpp ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lldb/Core/DataExtractor.h"
#include "lldb/Core/RegisterValue.h"
#include "lldb/Target/Thread.h"
#include "RegisterContextPOSIX.h"
#include "RegisterContextPOSIXCore_mips64.h"

using namespace lldb_private;

RegisterContextCorePOSIX_mips64::RegisterContextCorePOSIX_mips64(Thread &thread,
                                                                 RegisterInfoInterface *register_info,
                                                                 const DataExtractor &gpregset,
                                                                 const DataExtractor &fpregset)
    : RegisterContextPOSIX_mips64(thread, 0, register_info)
{
    m_gpr_buffer.reset(new DataBufferHeap(gpregset.GetDataStart(), gpregset.GetByteSize()));
    m_gpr.SetData(m_gpr_buffer);
    m_gpr.SetByteOrder(gpregset.GetByteOrder());
}

RegisterContextCorePOSIX_mips64::~RegisterContextCorePOSIX_mips64()
{
}

bool
RegisterContextCorePOSIX_mips64::ReadGPR()
{
    return true;
}

bool
RegisterContextCorePOSIX_mips64::ReadFPR()
{
    return false;
}

bool
RegisterContextCorePOSIX_mips64::WriteGPR()
{
    assert(0);
    return false;
}

bool
RegisterContextCorePOSIX_mips64::WriteFPR()
{
    assert(0);
    return false;
}

bool
RegisterContextCorePOSIX_mips64::ReadRegister(const RegisterInfo *reg_info, RegisterValue &value)
{
    lldb::offset_t offset = reg_info->byte_offset;
    uint64_t v = m_gpr.GetMaxU64(&offset, reg_info->byte_size);
    if (offset == reg_info->byte_offset + reg_info->byte_size)
    {
        value = v;
        return true;
    }
    return false;
}

bool
RegisterContextCorePOSIX_mips64::ReadAllRegisterValues(lldb::DataBufferSP &data_sp)
{
    return false;
}

bool
RegisterContextCorePOSIX_mips64::WriteRegister(const RegisterInfo *reg_info, const RegisterValue &value)
{
    return false;
}

bool
RegisterContextCorePOSIX_mips64::WriteAllRegisterValues(const lldb::DataBufferSP &data_sp)
{
    return false;
}

bool
RegisterContextCorePOSIX_mips64::HardwareSingleStep(bool enable)
{
    return false;
}
