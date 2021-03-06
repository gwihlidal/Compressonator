//=====================================================================
// Copyright 2008 (c), ATI Technologies Inc. All rights reserved.
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//=====================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "stdafx.h"
#include <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include "TC_PluginAPI.h"
#include "TC_PluginInternal.h"
#include "MIPS.h"

#include "ddraw.h"
#include "d3d9types.h"
#include "MIPS.h"
#include "Compressonator.h"
#include "DDS.h"
#include "DDS_File.h"
#include "DDS_Helpers.h"
#include "Texture.h"


TC_PluginError LoadDDS_FourCC(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Compressed, TDT_XRGB, PreLoopFourCC, LoopFourCC, PostLoopFourCC);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_RGB565(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_8bit, TDT_XRGB, PreLoopRGB565, LoopRGB565, PostLoopRGB565);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_RGB888(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_8bit, TDT_XRGB, 
        PreLoopRGB888, LoopRGB888, PostLoopRGB888);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_RGB8888(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet, bool bAlpha)
{
    ARGB8888Struct* pARGB8888Struct = (ARGB8888Struct*)calloc(sizeof(ARGB8888Struct), 1);
    void* extra = pARGB8888Struct;

    pMipSet->m_format = CMP_FORMAT_ARGB_8888; //  added AMD_ format definition

/*
    if(pDDSD->dwFlags & DDSD_PIXELFORMAT)
    {
        if(pDDSD->ddpfPixelFormat.dwRBitMask != 0xFF0000)
        {
            pARGB8888Struct->nFlags |= EF_UseBitMasks;
        }
        else if(pDDSD->ddpfPixelFormat.dwGBitMask != 0x00FF00)
        {
            pARGB8888Struct->nFlags |= EF_UseBitMasks;
        }
        else if(pDDSD->ddpfPixelFormat.dwBBitMask != 0x0000FF)
        {
            pARGB8888Struct->nFlags |= EF_UseBitMasks;
        }
    }
*/
    pARGB8888Struct->nFlags |= EF_UseBitMasks;

    if(pARGB8888Struct->nFlags & EF_UseBitMasks)
    {    //using bitmasks
        pARGB8888Struct->pMemory = malloc(4 * pDDSD->dwWidth * pDDSD->dwHeight);
        if (pMipSet->m_swizzle) 
        {
            pARGB8888Struct->nBMask = pDDSD->ddpfPixelFormat.dwRBitMask;
            pARGB8888Struct->nGMask = pDDSD->ddpfPixelFormat.dwGBitMask;
            pARGB8888Struct->nRMask = pDDSD->ddpfPixelFormat.dwBBitMask;
        }
        else
        {
            pARGB8888Struct->nRMask = pDDSD->ddpfPixelFormat.dwRBitMask;
            pARGB8888Struct->nGMask = pDDSD->ddpfPixelFormat.dwGBitMask;
            pARGB8888Struct->nBMask = pDDSD->ddpfPixelFormat.dwBBitMask;
        }
        

        int shift = 0;
        int tempMask = pARGB8888Struct->nRMask;
        while(!(tempMask & 0xFF) && tempMask)
        {
            shift += 8;
            tempMask >>= 8;
        }
        pARGB8888Struct->nRShift = shift;

        shift = 0;
        tempMask = pARGB8888Struct->nGMask;
        while(!(tempMask & 0xFF) && tempMask)
        {
            shift += 8;
            tempMask >>= 8;
        }
        pARGB8888Struct->nGShift = shift;

        shift = 0;
        tempMask = pARGB8888Struct->nBMask;
        while(!(tempMask & 0xFF) && tempMask)
        {
            shift += 8;
            tempMask >>= 8;
        }
        pARGB8888Struct->nBShift = shift;
    }
    pMipSet->m_TextureDataType = bAlpha ? TDT_ARGB : TDT_XRGB;

    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_8bit, pMipSet->m_TextureDataType, 
        PreLoopRGB8888, LoopRGB8888, PostLoopRGB8888);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_ARGB2101010(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    pMipSet->m_TextureDataType = TDT_ARGB;
    ChannelFormat channelFormat = CF_2101010;
    void* pChannelFormat = &channelFormat;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, pChannelFormat, channelFormat, pMipSet->m_TextureDataType, 
        PreLoopDefault, (pDDSD->ddpfPixelFormat.dwRBitMask==0x3ff00000) ? LoopR10G10B10A2 : LoopDefault, PostLoopDefault);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_ABGR32F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float32, TDT_ARGB, 
        PreLoopABGR32F, LoopABGR32F, PostLoopABGR32F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_GR32F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float32, TDT_RG, 
        PreLoopABGR32F, LoopABGR32F, PostLoopABGR32F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_R32F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float32, TDT_R, 
        PreLoopABGR32F, LoopABGR32F, PostLoopABGR32F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_R16F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float16, TDT_R, 
        PreLoopABGR16F, LoopABGR16F, PostLoopABGR16F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_G16R16F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float16, TDT_RG, 
        PreLoopABGR16F, LoopABGR16F, PostLoopABGR16F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_ABGR16F(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Float16, TDT_ARGB, 
        PreLoopABGR16F, LoopABGR16F, PostLoopABGR16F);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_G8(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Compressed, TDT_XRGB, 
        PreLoopG8, LoopG8, PostLoopG8);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_AG8(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Compressed, TDT_ARGB, 
        PreLoopAG8, LoopAG8, PostLoopAG8);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_G16(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Compressed, TDT_XRGB, 
        PreLoopG16, LoopG16, PostLoopG16);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_A8(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_Compressed, TDT_ARGB, 
        PreLoopA8, LoopA8, PostLoopA8);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_ABGR16(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_16bit, TDT_ARGB, 
        PreLoopABGR16, LoopABGR16, PostLoopABGR16);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_G16R16(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_16bit, TDT_RG, 
        PreLoopG16R16, LoopABGR16, PostLoopG16R16);
    fclose(pFile);
    return err;
}

TC_PluginError LoadDDS_R16(FILE* pFile, DDSD2* pDDSD, MipSet* pMipSet)
{
    void* extra;
    TC_PluginError err = GenericLoadFunction(pFile, pDDSD, pMipSet, extra, CF_16bit, TDT_R, 
        PreLoopG16R16, LoopABGR16, PostLoopG16R16);
    fclose(pFile);
    return err;
}

TC_PluginError SaveDDS_RGB888(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x000000ff;
    ddsd2.lPitch = pMipSet->m_nWidth * 3;
    ddsd2.ddpfPixelFormat.dwRGBBitCount = 24;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_RGB;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
    {
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
        {
            BYTE* pData = DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData;
            BYTE* pEnd = pData + DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_dwLinearSize;
            while(pData < pEnd)
            {
                fwrite(pData, 3, 1, pFile);
                pData += 4;
            }
        }
    }

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_ARGB8888(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.ddpfPixelFormat.dwRBitMask = 0x00ff0000;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x0000ff00;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x000000ff;
    ddsd2.lPitch = pMipSet->m_nWidth * 4;
    ddsd2.ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_ALPHAPIXELS|DDPF_RGB;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
        {
                BYTE* pbData = DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData;
                BYTE temp;
                int i = 0;
                int height = DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_nHeight;
                int width = DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_nWidth;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++)
                    {
                        temp = pbData[i];
                        pbData[i] = pbData[i + 2];
                        pbData[i + 2] = temp;
                        i += 4;
                    }
                }
                fwrite(pbData, (DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize), 1, pFile);
        }
    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_ARGB2101010(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.ddpfPixelFormat.dwRBitMask = 0x000003ff;
    ddsd2.ddpfPixelFormat.dwGBitMask = 0x000ffc00;
    ddsd2.ddpfPixelFormat.dwBBitMask = 0x3ff00000;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0xc0000000;
    ddsd2.lPitch = pMipSet->m_nWidth * 4;
    ddsd2.ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_ALPHAPIXELS|DDPF_RGB;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            if (pMipSet->m_swizzle)
            {    // to do swizzle data
                fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);
            }
            else
                fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_ABGR16(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 8;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_A16B16G16R16;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_R16(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 2;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_L16;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_RG16(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 4;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_G16R16;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_ABGR16F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 8;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_A16B16G16R16F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_R16F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 2;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_R16F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_RG16F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 4;
    ddsd2.ddpfPixelFormat.dwFlags = DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_G16R16F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_ABGR32F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 16;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_A32B32G32R32F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_R32F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 4;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_R32F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_RG32F(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    // Initialise surface descriptor
    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 8;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_FOURCC|DDPF_ALPHAPIXELS;
    ddsd2.ddpfPixelFormat.dwFourCC = D3DFMT_G32R32F;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_FourCC(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, true);

    ddsd2.ddpfPixelFormat.dwFlags=DDPF_FOURCC;
    if(pMipSet->m_TextureDataType == TDT_ARGB)
        ddsd2.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
    if(pMipSet->m_Flags & MS_AlphaPremult)
        ddsd2.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPREMULT;
    ddsd2.ddpfPixelFormat.dwFourCC = pMipSet->m_dwFourCC;
    ddsd2.ddpfPixelFormat.dwPrivateFormatBitCount = pMipSet->m_dwFourCC2;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_G8(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 8;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_LUMINANCE;
    ddsd2.ddpfPixelFormat.dwLuminanceBitCount = 8;
    ddsd2.ddpfPixelFormat.dwLuminanceBitMask = 0xff;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

TC_PluginError SaveDDS_A8(FILE* pFile, const MipSet* pMipSet)
{
    assert(pFile);
    assert(pMipSet);

    DDSD2 ddsd2;
    SetupDDSD(ddsd2, pMipSet, false);

    ddsd2.lPitch = pMipSet->m_nWidth * 8;
    ddsd2.ddpfPixelFormat.dwFlags=DDPF_ALPHA;
    ddsd2.ddpfPixelFormat.dwAlphaBitDepth = 8;
    ddsd2.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff;

    // Write the data    
    fwrite(&ddsd2, sizeof(DDSD2), 1, pFile);

    int nSlices = (pMipSet->m_TextureType == TT_2D) ? 1 : MaxFacesOrSlices(pMipSet, 0);
    for(int nSlice = 0; nSlice < nSlices; nSlice++)
        for(int nMipLevel = 0 ; nMipLevel < pMipSet->m_nMipLevels ; nMipLevel++)
            fwrite(DDS_CMips->GetMipLevel(pMipSet, nMipLevel, nSlice)->m_pbData, DDS_CMips->GetMipLevel(pMipSet, nMipLevel)->m_dwLinearSize, 1, pFile);

    fclose(pFile);

    return PE_OK;
}

