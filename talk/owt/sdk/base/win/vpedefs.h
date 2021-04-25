// Copyright (C) <2021> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef OWT_BASE_WIN_VPEDEFS_H
#define OWT_BASE_WIN_VPEDEFS_H

enum ScalingMode : int {
  SCALING_MODE_DEFAULT = 0,                 // Default
  SCALING_MODE_QUALITY,                     // LowerPower
  SCALING_MODE_SUPERRESOLUTION              // SuperREsolution
};

enum VPEMode : int {
  VPE_MODE_NONE = 0x0,
  VPE_MODE_PREPROC = 0x1,
  VPE_MODE_CPU_GPU_COPY = 0x3,
};

enum VPE_VERSION_ENUM : int {
  VPE_VERSION_1_0 = 0x0001,
  VPE_VERSION_2_0 = 0x0002,
  VPE_VERSION_3_0 = 0x0003,
  VPE_VERSION_UNKNOWN = 0xffff,
};

enum VPE_SUPER_RESOLUTION_MODE : int {
  DEFAULT_SCENARIO_MODE = 0,
  CAMERA_SCENARIO_MODE = 1,
};

enum VPE_CPU_GPU_COPY_DIRECTION : int {
    VPE_CPU_GPU_COPY_DIRECTION_CPU_TO_GPU,
    VPE_CPU_GPU_COPY_DIRECTION_GPU_TO_CPU,
    VPE_CPU_GPU_COPY_DIRECTION_MMC_IN,
    VPE_CPU_GPU_COPY_DIRECTION_NOW_MMC_IN,
    VPE_CPU_GPU_COPY_DIRECTION_MMC_OUT,
    VPE_CPU_GPU_COPY_DIRECTION_NOW_MMC_OUT,
};

typedef struct _SR_SCALING_MODE {
  UINT Fastscaling;
}SR_SCALING_MODE, *PSR_SCALING_MODE;

typedef struct _VPE_VERSION {
  UINT Version;
}VPE_VERSION, *PVPE_VERSION;

typedef struct _VPE_MODE {
  UINT Mode;
}VPE_MODE, *PVPE_MODE;

typedef struct _VPE_FUNCTION {
  UINT Function;               
  union {
    void* pSrCalingmode;
    void* pVPEMode;
    void* pVPEVersion;
    void* pSRParams;
    void* pCpuGpuCopyParam;
  };
} VPE_FUNCTION, *PVPE_FUNCTION;

typedef struct _VPE_SR_PARAMS {
  UINT bEnable : 1;  // [in], Enable SR
  UINT ReservedBits : 31;
  VPE_SUPER_RESOLUTION_MODE SRMode;
  UINT Reserved[4];
}VPE_SR_PARAMS, *PVPE_SR_PARAMS;

typedef struct _VPE_CPU_GPU_COPY_PARAM {
  VPE_CPU_GPU_COPY_DIRECTION Direction;
  UINT MemSize;
  void* pSystemMem;
}VPE_CPU_GPU_COPY_PARAM, *PVPE_CPU_GPU_COPY_PARAM;

#endif