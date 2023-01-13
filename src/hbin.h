/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2020 Sergey Chaban <sergey.chaban@gmail.com> */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HBIN_API_DECL
#	define HBIN_API_DECL
#endif

#ifndef HBIN_API_CALL
#	define HBIN_API_CALL
#endif

#define HBIN_NONE NULL

#define HBIN_FN(_name) hbin##_name
#define HBIN_IFC(_ret, _name) HBIN_API_DECL _ret HBIN_API_CALL HBIN_FN(_name)

#define HBIN_BGEO_FN(_name) bgeo##_name
#define HBIN_BGEO_IFC(_ret, _name) HBIN_API_DECL _ret HBIN_API_CALL HBIN_BGEO_FN(_name)

#define HBIN_BCLIP_FN(_name) bclip##_name
#define HBIN_BCLIP_IFC(_ret, _name) HBIN_API_DECL _ret HBIN_API_CALL HBIN_BCLIP_FN(_name)

typedef void* HBIN_BGEO;
typedef void* HBIN_BCLIP;

typedef float HBIN_FLOAT3[3];
typedef float HBIN_FLOAT2[2];

typedef void* HBIN_PRIM;
typedef int (*HBIN_PRIM_CB)(const HBIN_PRIM prim, void* pUserData);

typedef struct _HBIN_STRING {
	const char* pChars;
	size_t len;
} HBIN_STRING;

typedef struct _HBIN_CAPTURE {
	int32_t node;
	float wght;
} HBIN_CAPTURE;

HBIN_IFC(HBIN_STRING, NameFromPath)(HBIN_STRING path);
HBIN_IFC(int, StringsEqual)(HBIN_STRING str1, HBIN_STRING str2);
HBIN_IFC(int, StringsEqualC)(HBIN_STRING str, const char* pCStr);
HBIN_IFC(HBIN_STRING, StringAtIdx)(const void* pMem, const int32_t idx);
HBIN_IFC(void, PrintString)(HBIN_STRING str);

HBIN_BGEO_IFC(int, Valid)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, Version)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPoints)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPrims)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPointGroups)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPrimGroups)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPointAttrs)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumVertexAttrs)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumPrimAttrs)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, NumDetailAttrs)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(void, PointPos)(HBIN_FLOAT3 pos, const HBIN_BGEO bgeo, const int32_t pntId);
HBIN_BGEO_IFC(void, PointNrm)(HBIN_FLOAT3 nrm, const HBIN_BGEO bgeo, const int32_t pntId);
HBIN_BGEO_IFC(void, PointRGB)(HBIN_FLOAT3 rgb, const HBIN_BGEO bgeo, const int32_t pntId);
HBIN_BGEO_IFC(void, PointUVW)(HBIN_FLOAT3 uvw, const HBIN_BGEO bgeo, const int32_t pntId);
HBIN_BGEO_IFC(void, PointUV)(HBIN_FLOAT2 uv, const HBIN_BGEO bgeo, const int32_t pntId);
HBIN_BGEO_IFC(void, PointVecAttr)(HBIN_FLOAT3 vec, const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId);
HBIN_BGEO_IFC(float, PointFloatAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId);
HBIN_BGEO_IFC(HBIN_STRING, PointStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId);
HBIN_BGEO_IFC(int32_t, FindPointByStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const HBIN_STRING attrVal);
HBIN_BGEO_IFC(int32_t, FindPointByCStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const char* pAttrVal);
HBIN_BGEO_IFC(HBIN_STRING, PointAttrName)(const HBIN_BGEO bgeo, const int32_t attrId);
HBIN_BGEO_IFC(int, PointAttrIsVec)(const HBIN_BGEO bgeo, const int32_t attrId);
HBIN_BGEO_IFC(int, PointAttrIsStr)(const HBIN_BGEO bgeo, const int32_t attrId);
HBIN_BGEO_IFC(int32_t, CountTriangles)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(int32_t, CountPolygons)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(void, ForEachPrim)(const HBIN_BGEO bgeo, HBIN_PRIM_CB callback, void* pUserData);
HBIN_BGEO_IFC(const HBIN_BGEO, PrimBgeo)(const HBIN_PRIM prim);
HBIN_BGEO_IFC(int32_t, PrimIsPoly)(const HBIN_PRIM prim);
HBIN_BGEO_IFC(int32_t, PrimIsSphere)(const HBIN_PRIM prim);
HBIN_BGEO_IFC(int32_t, PrimNumVertices)(const HBIN_PRIM prim);
HBIN_BGEO_IFC(int32_t, PrimVertexPntId)(const HBIN_PRIM prim, const int32_t vtxId);
HBIN_BGEO_IFC(int32_t, PrimMaterialId)(const HBIN_PRIM prim);
HBIN_BGEO_IFC(int32_t, NumMaterials)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(HBIN_STRING, MaterialPath)(const HBIN_BGEO bgeo, const int32_t mtlId);
HBIN_BGEO_IFC(HBIN_STRING, DetailStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName);
HBIN_BGEO_IFC(int32_t, NumCaptureNodes)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(HBIN_STRING, CaptureNodePath)(const HBIN_BGEO bgeo, const int32_t nodeId);
HBIN_BGEO_IFC(int32_t, MaxCapturesPerPoint)(const HBIN_BGEO bgeo);
HBIN_BGEO_IFC(HBIN_CAPTURE, PointCapture)(const HBIN_BGEO bgeo, const int32_t pntId, const int32_t wgtId);
HBIN_BGEO_IFC(int32_t, SkeletonNames)(const HBIN_BGEO bgeo, const char* pAttrName, HBIN_STRING* pNames);
HBIN_BGEO_IFC(int32_t, SkeletonParents)(const HBIN_BGEO bgeo, const char* pAttrName, int32_t* pParents);
HBIN_BGEO_IFC(int32_t, SkeletonTransforms)(const HBIN_BGEO bgeo, const char* pAttrName, float* pXforms);

HBIN_BGEO_IFC(void, MakeVertexBuffer)(
	const HBIN_BGEO bgeo,
	void* pMem, const int32_t stride,
	const int32_t posOffs, const int32_t nrmOffs,
	const int32_t rgbOffs, const int32_t texOffs,
	const int32_t wgtOffs, const int32_t idxOffs,
	const int32_t maxWghts, int32_t* pInflCounts
);
HBIN_BGEO_IFC(int32_t, GetTriangles)(
	const HBIN_BGEO bgeo, uint16_t* pIdx16, uint32_t* pIdx32, int32_t* pMtlIds
);


HBIN_BCLIP_IFC(int, Valid)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(int32_t, Version)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(double, SampleRate)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(double, StartIndex)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(int32_t, TrackLength)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(int32_t, NumTracks)(const HBIN_BCLIP bclip);
HBIN_BCLIP_IFC(void, AllTracks)(const HBIN_BCLIP bclip, float* pSmps /* [numTracks][trackLen] */, HBIN_STRING* pNames /* [numTracks] */);

#ifdef __cplusplus
}
#endif
