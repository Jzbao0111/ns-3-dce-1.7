/*
 * gVirtuS -- A GPGPU transparent virtualization component.
 *
 * Copyright (C) 2009-2010  The University of Napoli Parthenope at Naples.
 *
 * This file is part of gVirtuS.
 *
 * gVirtuS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * gVirtuS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gVirtuS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Written by: Giuseppe Coviello <giuseppe.coviello@uniparthenope.it>,
 *             Department of Applied Science
 */

/**
 * @file   CudaUtil.cpp
 * @author Giuseppe Coviello <giuseppe.coviello@uniparthenope.it>
 * @date   Sun Oct 11 17:16:48 2009
 *
 * @brief
 *
 *
 */

#include "cuda-util.h"

#include <cstdio>
#include <iostream>

#include </usr/local/cuda-7.5/include/cuda.h>

using namespace std;

CudaUtil::CudaUtil()
{
}

CudaUtil::CudaUtil(const CudaUtil& orig)
{
}

CudaUtil::~CudaUtil()
{
}

char * CudaUtil::MarshalHostPointer(const void* ptr)
{
    char *marshal = new char[CudaUtil::MarshaledHostPointerSize];
    MarshalHostPointer(ptr, marshal);
    return marshal;
}

void CudaUtil::MarshalHostPointer(const void * ptr, char * marshal)
{
    sprintf(marshal, "%p", ptr);
}

char * CudaUtil::MarshalDevicePointer(const void* devPtr)
{
    char *marshal = new char[CudaUtil::MarshaledDevicePointerSize];
    MarshalDevicePointer(devPtr, marshal);
    return marshal;
}

void CudaUtil::MarshalDevicePointer(const void* devPtr, char * marshal)
{
    sprintf(marshal, "%p", devPtr);
}

Buffer * CudaUtil::MarshalFatCudaBinary(__cudaFatCudaBinary* bin, Buffer * marshal)
{
    if(marshal == NULL)
        marshal = new Buffer();
    size_t size = 0;
    int count = 0;

    marshal->Add(bin->magic);
    marshal->Add(bin->version);
    marshal->Add(bin->gpuInfoVersion);

    if (bin->key) size = strlen(bin->key) + 1;
    else size = 0;
    marshal->Add(size);
    marshal->Add(bin->key, size);

    if (bin->ident) size = strlen(bin->ident) + 1;
    else size = 0;
    marshal->Add(size);
    marshal->Add(bin->ident, size);

    if (bin->usageMode) size = strlen(bin->usageMode) + 1;
    else size = 0;
    marshal->Add(size);
    marshal->Add(bin->usageMode, size);


    if (bin->ptx) {
        for(count = 0; bin->ptx[count].gpuProfileName != NULL; count++);
    }
    else {
    	count = 0;
    }
    marshal->Add(count);
    for(int i = 0; i < count; i++) {
        if (bin->ptx[i].gpuProfileName) {
        	size = strlen(bin->ptx[i].gpuProfileName) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->ptx[i].gpuProfileName, size);

        if (bin->ptx[i].ptx) {
        	size = strlen(bin->ptx[i].ptx) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->ptx[i].ptx, size);
    }

    if (bin->cubin) {
    	for(count = 0; bin->cubin[count].gpuProfileName != NULL; count++);
    }
    else {
    	count = 0;
    }
    marshal->Add(count);
    for(int i = 0; i < count; i++) {
        if (bin->cubin[i].gpuProfileName) {
        	size = strlen(bin->cubin[i].gpuProfileName) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->cubin[i].gpuProfileName, size);

        if (bin->cubin[i].cubin) {
            size = strlen(bin->cubin[i].cubin) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->cubin[i].cubin, size);
    }

    /* Achtung: no debug is possible */
    marshal->Add(0);

    count = 0;
    marshal->Add(count);
    for(int i = 0; i < count; i++) {
        if (bin->exported[i].name) {
        	size = strlen(bin->exported[i].name) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->exported[i].name, size);
    }

    count = 0;
    marshal->Add(count);
    for(int i = 0; i < count; i++) {
        if (bin->imported[i].name) {
        	size = strlen(bin->imported[i].name) + 1;
        }
        else {
        	size = 0;
        }
        marshal->Add(size);
        marshal->Add(bin->imported[i].name, size);
    }

    marshal->Add(bin->flags);

    /* Achtung: no dependends added */
    marshal->Add(0);

#ifndef CUDA_VERSION
#error CUDA_VERSION not defined
#endif
#if CUDA_VERSION >= 2030
    marshal->Add(bin->characteristic);
#endif
    
    return marshal;
}

__cudaFatCudaBinary * CudaUtil::UnmarshalFatCudaBinary(Buffer* marshal)
{
    __cudaFatCudaBinary * bin = new __cudaFatCudaBinary;
    size_t size = 0;
    int i = 0, count = 0;

    bin->magic = marshal->Get<unsigned long>();
    bin->version = marshal->Get<unsigned long>();
    bin->gpuInfoVersion = marshal->Get<unsigned long>();

    size = marshal->Get<size_t>();
    bin->key = marshal->Get<char>(size);

    size = marshal->Get<size_t>();
    bin->ident = marshal->Get<char>(size);

    size = marshal->Get<size_t>();
    bin->usageMode = marshal->Get<char>(size);

    count = marshal->Get<int>();
    bin->ptx = new __cudaFatPtxEntry[count + 1];
    for(i = 0; i < count; i++) {
        size = marshal->Get<size_t>();
        bin->ptx[i].gpuProfileName = marshal->Get<char>(size);

        size = marshal->Get<size_t>();
        bin->ptx[i].ptx = marshal->Get<char>(size);
    }
    bin->ptx[i].gpuProfileName = NULL;
    bin->ptx[i].ptx = NULL;

    count = marshal->Get<int>();
    bin->cubin = new __cudaFatCubinEntry[count + 1];
    for(i = 0; i < count; i++) {
        size = marshal->Get<size_t>();
        bin->cubin[i].gpuProfileName = marshal->Get<char>(size);

        size = marshal->Get<size_t>();
        bin->cubin[i].cubin = marshal->Get<char>(size);
    }
    bin->cubin[i].gpuProfileName = NULL;
    bin->cubin[i].cubin = NULL;

    /* Achtung: no debug is possible */
    marshal->Get<int>();
    bin->debug = new __cudaFatDebugEntry;
    bin->debug->gpuProfileName = NULL;
    bin->debug->debug = NULL;

    bin->debugInfo = NULL;

    count = marshal->Get<int>();
    if(count == 0)
        bin->exported = NULL;
    else {
        bin->exported = new __cudaFatSymbol[count + 1];
        for(i = 0; i < count; i++) {
            size = marshal->Get<size_t>();
            bin->exported[i].name = marshal->Get<char>(size);
        }
        bin->exported[i].name = NULL;
    }

    count = marshal->Get<int>();
    if(count == 0)
        bin->imported = NULL;
    else {
        bin->imported = new __cudaFatSymbol[count + 1];
        for(i = 0; i < count; i++) {
            size = marshal->Get<size_t>();
            bin->imported[i].name = marshal->Get<char>(size);
        }
        bin->imported[i].name = NULL;
    }

    bin->flags = marshal->Get<unsigned int>();

    marshal->Get<int>();
    bin->dependends = NULL;

#ifndef CUDA_VERSION
#error CUDA_VERSION not defined
#endif
#if CUDA_VERSION >= 2030
    bin->characteristic = marshal->Get<unsigned int>();
#endif
#if CUDA_VERSION >= 3010
    bin->elf = NULL;
#endif
    
    return bin;
}

void CudaUtil::DumpFatCudaBinary(__cudaFatCudaBinary* bin, ostream & out)
{
    out << endl << "FatCudaBinary" << endl << "----------" << endl;
    out << "magic: " << bin->magic << endl;
    out << "version: " << bin->version << endl;
    out << "gpuInfoVersion: " << bin->gpuInfoVersion << endl;
    out << "key: " << bin->key << endl;
    out << "ident: " << bin->ident << endl;
    out << "usageMode: " << bin->usageMode << endl;
    out << "ptx:" << endl;
    int i = 0;
    for(i = 0; bin->ptx[i].gpuProfileName != NULL; i++) {
        out << '\t' << "gpuProfileName[" << i << "]: " << bin->ptx[i].gpuProfileName << endl;
        out << '\t' << "ptx[" << i << "]: " << bin->ptx[i].ptx << endl;
    }
    out << "***" << i << endl;
    out << "cubin:" << endl;
    for(int i = 0; bin->cubin[i].gpuProfileName != NULL; i++) {
        out << '\t' << "gpuProfileName[" << i << "]: " << bin->cubin[i].gpuProfileName << endl;
        out << '\t' << "cubin[" << i << "]: " << bin->cubin[i].cubin << endl;
    }
    out << "exported:" << endl;
    for(int i = 0; bin->exported != NULL && bin->exported[i].name != NULL; i++) {
        out << '\t' << "name[" << i << "]: " << bin->exported[i].name << endl;
    }
    out << "imported:" << endl;
    for(int i = 0; bin->imported != NULL && bin->imported[i].name != NULL; i++) {
        out << '\t' << "name[" << i << "]: " << bin->imported[i].name << endl;
    }
    out << "flags: " << bin->flags << endl;
#ifndef CUDA_VERSION
#error CUDA_VERSION not defined
#endif
#if CUDA_VERSION == 2030
    out << "characteristic: " << bin->characteristic << endl;
#endif
    out << "----------" << endl << endl;
}