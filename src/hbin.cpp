/* SPDX-License-Identifier: MIT */
/* SPDX-FileCopyrightText: 2020 Sergey Chaban <sergey.chaban@gmail.com> */

#include "hbin.h"

#ifdef HBIN_NO_CLIB
static size_t hbinStrLen(const char* pStr) {
	size_t len = 0;
	if (pStr) {
		const char* p = pStr;
		while (1) {
			if (!*p) break;
			++p;
		}
		len = (size_t)(p - pStr);
	}
	return len;
}
static void hbinMemCpy(void* pDst, const void* pSrc, const size_t len) {
	if (pDst && pSrc) {
		uint8_t* pd = (uint8_t*)pDst;
		const uint8_t* ps = (const uint8_t*)pSrc;
		size_t i;
		for (i = 0; i < len; ++i) {
			*pd++ = *ps++;
		}
	}
}
static int hbinMemCmp(const void* pMem1, const void* pMem2, const size_t len) {
	int cmp = -1;
	if (pMem1 && pMem2 && len > 0) {
		size_t n = len;
		const uint8_t* p1 = (const uint8_t*)pMem1;
		const uint8_t* p2 = (const uint8_t*)pMem2;
		cmp = 0;
		for (; n > 0 && *p1 == *p2; --n) {
			++p1;
			++p2;
		}
		if (n > 0) {
			cmp = *p1 < *p2 ? -1 : 1;
		}
	}
	return cmp;
}
#else
#include <string.h>
#include <stdio.h>
#define hbinStrLen strlen
#define hbinMemCpy memcpy
#define hbinMemCmp memcmp
#endif

enum HBIN_PRIMTYPE {
	HBIN_PRIMTYPE_Poly,
	HBIN_PRIMTYPE_NURBSCurve,
	HBIN_PRIMTYPE_BezierCurve,
	HBIN_PRIMTYPE_Mesh,
	HBIN_PRIMTYPE_NURBSMesh,
	HBIN_PRIMTYPE_BezierMesh,
	HBIN_PRIMTYPE_PasteSurf,
	HBIN_PRIMTYPE_Circle,
	HBIN_PRIMTYPE_Sphere,
	HBIN_PRIMTYPE_Tube,
	HBIN_PRIMTYPE_Part,
	HBIN_PRIMTYPE_MetaBall,
	HBIN_PRIMTYPE_MetaSQuad,
	HBIN_PRIMTYPE_TriFan,
	HBIN_PRIMTYPE_TriStrip,
	HBIN_PRIMTYPE_TriBezier,
	HBIN_PRIMTYPE_Volume
};

static const char* s_pBgeoMtlAttrName = "shop_materialpath";
static const char* s_pBgeoCaptPathName = "pCaptPath";
static const char* s_pBgeoCaptAttrName = "pCapt";

static uint32_t hbinU32(const uint8_t* pMem) {
	uint32_t b3 = pMem[0];
	uint32_t b2 = pMem[1];
	uint32_t b1 = pMem[2];
	uint32_t b0 = pMem[3];
	return (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

static int32_t hbinI32(const uint8_t* pMem) {
	return (int32_t)hbinU32(pMem);
}

static uint16_t hbinU16(const uint8_t* pMem) {
	uint16_t b1 = pMem[0];
	uint16_t b0 = pMem[1];
	return (b0 | (b1 << 8));
}

static int16_t hbinI16(const uint8_t* pMem) {
	uint16_t b1 = pMem[0];
	uint16_t b0 = pMem[1];
	return (int16_t)(b0 | (b1 << 8));
}

static float hbinF32(const uint8_t* pMem) {
	float f;
	uint32_t u = hbinU32(pMem);
	hbinMemCpy(&f, &u, sizeof(float));
	return f;
}

static double hbinF64(const uint8_t* pMem) {
	double d = 1.0;
	uint8_t b[8];
	int i;
	hbinMemCpy(b, &d, sizeof(b));
	if (b[0] == 0) {
		/* little-endian */
		for (i = 0; i < 8; ++i) {
			b[7 - i] = pMem[i];
		}
	} else {
		/* big-endian */
		for (i = 0; i < 8; ++i) {
			b[i] = pMem[i];
		}
	}
	hbinMemCpy(&d, b, sizeof(double));
	return d;
}

HBIN_IFC(HBIN_STRING, NameFromPath)(HBIN_STRING path) {
	HBIN_STRING name;
	name.pChars = NULL;
	name.len = 0;
	if (path.len > 0 && path.pChars) {
		const char* pChr = path.pChars + path.len - 1;
		size_t len = 0;
		while (len < path.len) {
			if (*pChr == '/') {
				++pChr;
				break;
			}
			++len;
			if (len < path.len) {
				--pChr;
			}
		}
		name.pChars = len ? pChr : NULL;
		name.len = len;
	}
	return name;
}

HBIN_IFC(int, StringsEqual)(HBIN_STRING str1, HBIN_STRING str2) {
	if (str1.len != str2.len) return 0;
	if (str1.pChars == str2.pChars) return 1;
	return hbinMemCmp(str1.pChars, str2.pChars, str1.len) == 0;
}

HBIN_IFC(int, StringsEqualC)(HBIN_STRING str, const char* pCStr) {
	HBIN_STRING cstr;
	cstr.pChars = pCStr;
	cstr.len = pCStr ? hbinStrLen(pCStr) : 0;
	return hbinStringsEqual(str, cstr);
}

HBIN_IFC(HBIN_STRING, StringAtIdx)(const void* pMem, const int32_t idx) {
	HBIN_STRING str;
	str.pChars = NULL;
	str.len = 0;
	if (pMem) {
		hbinMemCpy(&str, (HBIN_STRING*)pMem + idx, sizeof(HBIN_STRING));
	}
	return str;
}

HBIN_IFC(void, PrintString)(HBIN_STRING str) {
#ifndef HBIN_NO_CLIB
	int i;
	if (str.pChars && str.len > 0) {
		for (i = 0; i < str.len; ++i) {
			printf("%c", str.pChars[i]);
		}
	} else {
		printf("<none>");
	}
#endif
}


HBIN_BGEO_IFC(int, Valid)(const HBIN_BGEO bgeo) {
	int valid = 0;
	if (bgeo) {
		static const char* pSig = "Bgeo";
		const uint8_t* pTop = (const uint8_t*)bgeo;
		if (hbinMemCmp(pTop, pSig, 4) == 0) {
			if (pTop[4] == 'V') {
				valid = 1;
			}
		}
	}
	return valid;
}

static int32_t bgeoI32(const HBIN_BGEO bgeo, const uint32_t offs) {
	int32_t val = -1;
	if (HBIN_BGEO_FN(Valid)(bgeo)) {
		const uint8_t* pTop = (const uint8_t*)bgeo;
		val = hbinI32(pTop + offs);
	}
	return val;
}

HBIN_BGEO_IFC(int32_t, Version)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 5); }
HBIN_BGEO_IFC(int32_t, NumPoints)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 9); }
HBIN_BGEO_IFC(int32_t, NumPrims)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0xD); }
HBIN_BGEO_IFC(int32_t, NumPointGroups)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x11); }
HBIN_BGEO_IFC(int32_t, NumPrimGroups)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x15); }
HBIN_BGEO_IFC(int32_t, NumPointAttrs)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x19); }
HBIN_BGEO_IFC(int32_t, NumVertexAttrs)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x1D); }
HBIN_BGEO_IFC(int32_t, NumPrimAttrs)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x21); }
HBIN_BGEO_IFC(int32_t, NumDetailAttrs)(const HBIN_BGEO bgeo) { return bgeoI32(bgeo, 0x25); }

static int32_t bgeoCalcItemRecSize(const HBIN_BGEO bgeo, const int32_t nattr, const uint32_t offs, const int32_t stdRecSize, const uint8_t** ppNext) {
	int32_t recSize = 0;
	if (nattr >= 0) {
		recSize = stdRecSize;
		if (nattr > 0) {
			int32_t i, j;
			int attrSize;
			uint32_t attrType;
			int defValSize = 0;
			int attrValSize = 0;
			const uint8_t* pTop = (const uint8_t*)bgeo;
			const uint8_t* pAttrDescr = pTop + offs;
			for (i = 0; i < nattr; ++i) {
				int nameSize = hbinI16(pAttrDescr);
				pAttrDescr += 2;
				if (nameSize < 0) {
					nameSize = hbinI32(pAttrDescr);
					pAttrDescr += 4;
				}
				if (nameSize <= 0) {
					break;
				}
				pAttrDescr += nameSize;
				attrSize = hbinI16(pAttrDescr);
				pAttrDescr += 2;
				if (attrSize < 0) {
					attrSize = hbinI32(pAttrDescr);
					pAttrDescr += 4;
				}
				attrType = hbinU32(pAttrDescr);
				pAttrDescr += 4;
				attrType &= 0xFFFF;
				defValSize = 0;
				attrValSize = 0;
				if (attrType == 4) { /* index */
					const uint8_t* pStrs;
					int32_t nstrs = hbinI32(pAttrDescr);
					pAttrDescr += 4;
					pStrs = pAttrDescr;
					for (j = 0; j < nstrs; ++j) {
						int strSize = hbinI16(pStrs);
						pStrs += 2;
						if (strSize < 0) {
							strSize = hbinI32(pStrs);
							pStrs += 4;
						}
						pStrs += strSize;
					}
					defValSize = (int)(pStrs - pAttrDescr);
					attrValSize = 4;
				} else {
					switch (attrType) {
						case 0: /* float */
						case 1: /* int */
							attrValSize = 4 * attrSize;
							break;
						case 5: /* vector */
							attrValSize = 4 * 3;
							break;
						default:
							break;
					}
					defValSize = attrValSize;
				}
				if (defValSize) {
					pAttrDescr += defValSize;
					recSize += attrValSize;
				} else {
					recSize = 0;
					break;
				}
			}
			if (ppNext) {
				*ppNext = pAttrDescr;
			}
		}
	}
	return recSize;
}

static int32_t bgeoCalcPntRecSize(const HBIN_BGEO bgeo, const uint8_t** ppPtsTop) {
	int32_t nattr = HBIN_BGEO_FN(NumPointAttrs)(bgeo);
	int32_t stdRecSize = 4 * 4; /* float32 x, y, z, w */
	return bgeoCalcItemRecSize(bgeo, nattr, 0x29, stdRecSize, ppPtsTop);
}

HBIN_BGEO_IFC(void, PointPos)(HBIN_FLOAT3 pos, const HBIN_BGEO bgeo, const int32_t pntId) {
	int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
	if (npts > 0 && (uint32_t)pntId < (uint32_t)npts) {
		const uint8_t* pPts = NULL;
		int32_t recSize = bgeoCalcPntRecSize(bgeo, &pPts);
		if (recSize > 0 && pPts) {
			int i;
			const uint8_t* pPntRec = pPts + (pntId * recSize);
			for (i = 0; i < 3; ++i) {
				pos[i] = hbinF32(pPntRec + (i * 4));
			}
		}
	}
}

static HBIN_STRING bgeoFindAttrName(const HBIN_BGEO bgeo, const int nattr, const uint32_t offs, const int32_t stdRecSize, const int iattr) {
	HBIN_STRING name;
	name.pChars = NULL;
	name.len = 0;
	if (nattr > 0 && (uint32_t)iattr < (uint32_t)nattr) {
		int32_t i, j;
		const uint8_t* pTop = (const uint8_t*)bgeo;
		const uint8_t* pAttrDescr = pTop + offs;
		int32_t attrValOffs = stdRecSize;
		for (i = 0; i < nattr; ++i) {
			int attrSize;
			uint32_t attrType;
			uint32_t attrTypeFull;
			const uint8_t* pStrs;
			int defValSize = 0;
			int attrValSize = 0;
			int attrNameLen = hbinI16(pAttrDescr);
			pAttrDescr += 2;
			if (attrNameLen < 0) {
				attrNameLen = hbinI32(pAttrDescr);
				pAttrDescr += 4;
			}
			if (attrNameLen <= 0) {
				break;
			}
			if (i == iattr) {
				name.pChars = (const char*)pAttrDescr;
				name.len = attrNameLen;
				break;
			}
			pAttrDescr += attrNameLen;
			attrSize = hbinI16(pAttrDescr);
			pAttrDescr += 2;
			if (attrSize < 0) {
				attrSize = hbinI32(pAttrDescr);
				pAttrDescr += 4;
			}
			attrType = hbinU32(pAttrDescr);
			attrTypeFull = attrType;
			pAttrDescr += 4;
			attrType &= 0xFFFF;
			if (attrType == 4) { /* index */
				int32_t nstrs = hbinI32(pAttrDescr);
				pAttrDescr += 4;
				pStrs = pAttrDescr;
				for (j = 0; j < nstrs; ++j) {
					int strSize = hbinI16(pStrs);
					pStrs += 2;
					if (strSize < 0) {
						strSize = hbinI32(pStrs);
						pStrs += 4;
					}
					pStrs += strSize;
				}
				defValSize = (int)(pStrs - pAttrDescr);
				attrValSize = 4;
			} else {
				switch (attrType) {
					case 0: /* float */
					case 1: /* int */
						attrValSize = 4 * attrSize;
						break;
					case 5: /* vector */
						attrValSize = 4 * 3;
						break;
					default:
						break;
				}
				defValSize = attrValSize;
			}
			if (defValSize) {
				pAttrDescr += defValSize;
			} else {
				if (attrType != 4) {
					break;
				}
			}
			attrValOffs += attrValSize;
		}
	}
	return name;
}

static int32_t bgeoFindAttrInfo(const HBIN_BGEO bgeo, const int nattr, const uint32_t offs, const int32_t stdRecSize, const char* pName, HBIN_STRING* pBinName, int32_t* pValOffs, int32_t* pType, int32_t* pSize, const uint8_t** ppDescr) {
	int32_t attrId = -1;
	size_t nameLen = 0;
	if (pName) {
		nameLen = hbinStrLen(pName);
	} else if (pBinName) {
		nameLen = pBinName->len;
	}
	if (nameLen > 0 && nattr > 0) {
		int32_t i, j;
		const uint8_t* pTop = (const uint8_t*)bgeo;
		const uint8_t* pAttrDescr = pTop + offs;
		int32_t attrValOffs = stdRecSize;
		for (i = 0; i < nattr; ++i) {
			int attrSize;
			uint32_t attrType;
			uint32_t attrTypeFull;
			const uint8_t* pStrs;
			int defValSize = 0;
			int attrValSize = 0;
			int attrNameLen = hbinI16(pAttrDescr);
			pAttrDescr += 2;
			if (attrNameLen < 0) {
				attrNameLen = hbinI32(pAttrDescr);
				pAttrDescr += 4;
			}
			if (attrNameLen <= 0) {
				break;
			}
			if (nameLen == attrNameLen) {
				if (pName) {
					if (hbinMemCmp(pAttrDescr, pName, nameLen) == 0) {
						attrId = i;
					}
				} else if (pBinName) {
					if (hbinMemCmp(pAttrDescr, pBinName->pChars, nameLen) == 0) {
						attrId = i;
					}
				}
			}
			pAttrDescr += attrNameLen;
			attrSize = hbinI16(pAttrDescr);
			pAttrDescr += 2;
			if (attrSize < 0) {
				attrSize = hbinI32(pAttrDescr);
				pAttrDescr += 4;
			}
			attrType = hbinU32(pAttrDescr);
			attrTypeFull = attrType;
			pAttrDescr += 4;
			attrType &= 0xFFFF;
			if (ppDescr) {
				*ppDescr = pAttrDescr;
			}
			if (attrType == 4) { /* index */
				int32_t nstrs = hbinI32(pAttrDescr);
				pAttrDescr += 4;
				pStrs = pAttrDescr;
				for (j = 0; j < nstrs; ++j) {
					int strSize = hbinI16(pStrs);
					pStrs += 2;
					if (strSize < 0) {
						strSize = hbinI32(pStrs);
						pStrs += 4;
					}
					pStrs += strSize;
				}
				defValSize = (int)(pStrs - pAttrDescr);
				attrValSize = 4;
			} else {
				switch (attrType) {
					case 0: /* float */
					case 1: /* int */
						attrValSize = 4 * attrSize;
						break;
					case 5: /* vector */
						attrValSize = 4 * 3;
						break;
					default:
						break;
				}
				defValSize = attrValSize;
			}
			if (defValSize) {
				pAttrDescr += defValSize;
			} else {
				if (attrType != 4) {
					attrId = -1;
					break;
				}
			}
			if (attrId >= 0) {
				if (pValOffs) {
					*pValOffs = attrValOffs;
				}
				if (pType) {
					*pType = attrTypeFull;
				}
				if (pSize) {
					*pSize = attrSize;
				}
				break;
			} else {
				attrValOffs += attrValSize;
			}
		}
	}
	return attrId;
}

HBIN_BGEO_IFC(void, PointNrm)(HBIN_FLOAT3 nrm, const HBIN_BGEO bgeo, const int32_t pntId) {
	HBIN_BGEO_FN(PointVecAttr)(nrm, bgeo, "N", pntId);
}

HBIN_BGEO_IFC(void, PointRGB)(HBIN_FLOAT3 rgb, const HBIN_BGEO bgeo, const int32_t pntId) {
	HBIN_BGEO_FN(PointVecAttr)(rgb, bgeo, "Cd", pntId);
}

HBIN_BGEO_IFC(void, PointUVW)(HBIN_FLOAT3 uvw, const HBIN_BGEO bgeo, const int32_t pntId) {
	HBIN_BGEO_FN(PointVecAttr)(uvw, bgeo, "uv", pntId);
}

HBIN_BGEO_IFC(void, PointUV)(HBIN_FLOAT2 uv, const HBIN_BGEO bgeo, const int32_t pntId) {
	HBIN_FLOAT3 uvw;
	HBIN_BGEO_FN(PointVecAttr)(uvw, bgeo, "uv", pntId);
	uv[0] = uvw[0];
	uv[1] = uvw[1];
}

HBIN_BGEO_IFC(void, PointVecAttr)(HBIN_FLOAT3 vec, const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId) {
	int32_t i;
	int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
	int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
	for (i = 0; i < 3; ++i) {
		vec[i] = 0.0f;
	}
	if (nattr && (unsigned)pntId < (unsigned)npts) {
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, pAttrName, NULL, &valOffs, &attrType, &attrSize, NULL);
		if (attrId >= 0) {
			const uint8_t* pPts = NULL;
			int32_t recSize = bgeoCalcPntRecSize(bgeo, &pPts);
			attrType &= 0xFFFF;
			if (recSize > 0 && pPts) {
				const uint8_t* pVal = pPts + (pntId * recSize) + valOffs;
				int nelem = 0;
				if (attrType == 5) {
					nelem = 3;
				} else if (attrType == 0 || attrType == 1) {
					nelem = attrSize;
					if (nelem > 3) {
						nelem = 3;
					}
				}
				if (attrType == 1) {
					for (i = 0; i < nelem; ++i) {
						vec[i] = (float)hbinI32(pVal + (i * 4));
					}
				} else {
					for (i = 0; i < nelem; ++i) {
						vec[i] = hbinF32(pVal + (i * 4));
					}
				}
			}
		}
	}
}

HBIN_BGEO_IFC(float, PointFloatAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId) {
	HBIN_FLOAT3 vec;
	HBIN_BGEO_FN(PointVecAttr)(vec, bgeo, pAttrName, pntId);
	return vec[0];
}

HBIN_BGEO_IFC(HBIN_STRING, PointStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const int32_t pntId) {
	int32_t i;
	HBIN_STRING str;
	int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
	int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
	str.pChars = NULL;
	str.len = 0;
	if (nattr > 0) {
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0 && attrType == 4 && attrSize == 1 && pDescr) {
			const uint8_t* pPts = NULL;
			int32_t recSize = bgeoCalcPntRecSize(bgeo, &pPts);
			attrType &= 0xFFFF;
			if (recSize > 0 && pPts) {
				const uint8_t* pStrs;
				const uint8_t* pVal = pPts + (pntId * recSize) + valOffs;
				int32_t strId = hbinI32(pVal);
				int32_t nstrs = hbinI32(pDescr);
				pDescr += 4;
				pStrs = pDescr;
				for (i = 0; i < nstrs; ++i) {
					int strSize = hbinI16(pStrs);
					pStrs += 2;
					if (strSize < 0) {
						strSize = hbinI32(pStrs);
						pStrs += 4;
					}
					if (strId == i) {
						str.pChars = (const char*)pStrs;
						str.len = strSize;
						break;
					}
					pStrs += strSize;
				}
			}
		}
	}
	return str;
}

HBIN_BGEO_IFC(int32_t, FindPointByStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const HBIN_STRING attrVal) {
	int32_t i;
	int32_t pntId = -1;
	int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
	int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
	if (nattr > 0) {
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0 && attrType == 4 && attrSize == 1 && pDescr) {
			const uint8_t* pPts = NULL;
			int32_t recSize = bgeoCalcPntRecSize(bgeo, &pPts);
			attrType &= 0xFFFF;
			if (recSize > 0 && pPts) {
				HBIN_STRING str;
				const uint8_t* pStrs;
				const uint8_t* pVal = pPts + valOffs;
				int32_t nstrs = hbinI32(pDescr);
				int32_t strValId = -1;
				pDescr += 4;
				pStrs = pDescr;
				for (i = 0; i < nstrs; ++i) {
					int strSize = hbinI16(pStrs);
					pStrs += 2;
					if (strSize < 0) {
						strSize = hbinI32(pStrs);
						pStrs += 4;
					}
					str.pChars = (const char*)pStrs;
					str.len = strSize;
					if (hbinStringsEqual(attrVal, str)) {
						strValId = i;
						break;
					}
					pStrs += strSize;
				}
				for (i = 0; i < npts; ++i) {
					int32_t strId = hbinI32(pVal);
					if (strId == strValId) {
						pntId = i;
						break;
					}
					pVal += recSize;
				}
			}
		}
	}
	return pntId;
}

HBIN_BGEO_IFC(int32_t, FindPointByCStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName, const char* pAttrVal) {
	int32_t pntId = -1;
	if (pAttrName && pAttrVal) {
		HBIN_STRING str;
		str.pChars = pAttrVal;
		str.len = hbinStrLen(pAttrVal);
		pntId = HBIN_BGEO_FN(FindPointByStrAttr)(bgeo, pAttrName, str);
	}
	return pntId;
}

HBIN_BGEO_IFC(HBIN_STRING, PointAttrName)(const HBIN_BGEO bgeo, const int32_t attrId) {
	HBIN_STRING name;
	name.pChars = NULL;
	name.len = 0;
	if (bgeo) {
		int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
		name = bgeoFindAttrName(bgeo, nattr, 0x29, 4 * 4, attrId);
	}
	return name;
}

HBIN_BGEO_IFC(int, PointAttrIsVec)(const HBIN_BGEO bgeo, const int32_t attrId) {
	int res = 0;
	HBIN_STRING name = HBIN_BGEO_FN(PointAttrName)(bgeo, attrId);
	if (name.pChars && name.len > 0) {
		int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
		if (nattr > 0) {
			int32_t attrType = 0;
			int32_t attrSize = 0;
			int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, NULL, &name, NULL, &attrType, &attrSize, NULL);
			if (attrId >= 0) {
				if (attrType == 5) {
					res = 1;
				} else if (attrType == 0 || attrType == 1) {
					if (attrSize >= 3) {
						res = 1;
					}
				}
			}
		}
	}
	return res;
}

HBIN_BGEO_IFC(int, PointAttrIsStr)(const HBIN_BGEO bgeo, const int32_t attrId) {
	int res = 0;
	HBIN_STRING name = HBIN_BGEO_FN(PointAttrName)(bgeo, attrId);
	if (name.pChars && name.len > 0) {
		int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
		if (nattr > 0) {
			int32_t attrType = 0;
			int32_t attrSize = 0;
			int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, NULL, &name, NULL, &attrType, &attrSize, NULL);
			if (attrId >= 0 && attrType == 4 && attrSize == 1) {
				res = 1;
			}
		}
	}
	return res;
}

static int bgeoCountTrisCB(const HBIN_PRIM prim, void* pUserData) {
	int32_t* pCnt = (int32_t*)pUserData;
	if (!pCnt) return 0;
	if (bgeoPrimIsPoly(prim)) {
		int32_t nvtx = bgeoPrimNumVertices(prim);
		if (nvtx == 3) {
			++(*pCnt);
		}
	}
	return 1;
}

HBIN_BGEO_IFC(int32_t, CountTriangles)(const HBIN_BGEO bgeo) {
	int32_t ntri = 0;
	int32_t nprim = HBIN_BGEO_FN(NumPrims)(bgeo);
	if (nprim > 0) {
		HBIN_BGEO_FN(ForEachPrim)(bgeo, bgeoCountTrisCB, &ntri);
	}
	return ntri;
}

static int bgeoCountPolsCB(const HBIN_PRIM prim, void* pUserData) {
	int32_t* pCnt = (int32_t*)pUserData;
	if (!pCnt) return 0;
	if (bgeoPrimIsPoly(prim)) {
		++(*pCnt);
	}
	return 1;
}

HBIN_BGEO_IFC(int32_t, CountPolygons)(const HBIN_BGEO bgeo) {
	int32_t npol = 0;
	int32_t nprim = HBIN_BGEO_FN(NumPrims)(bgeo);
	if (nprim > 0) {
		HBIN_BGEO_FN(ForEachPrim)(bgeo, bgeoCountPolsCB, &npol);
	}
	return npol;
}

typedef struct _HBIN_PRIM_S {
	const uint8_t* pTop;
	const uint8_t* pRec;
	const uint8_t* pIdx;
	uint32_t vtxAttrsOffs;
	uint32_t primAttrsOffs;
	uint32_t primRecsOffs;
	int32_t vtxAttrsRecSize;
	int32_t primAttrsRecSize;
	int32_t idxSize;
	int32_t recSize;
	int32_t nvtx;
	int32_t type;
	int32_t mtlId;
	int32_t id;
} HBIN_PRIM_S;

static const uint8_t* bgeoForEachPrimImpl(const HBIN_BGEO bgeo, HBIN_PRIM_CB callback, void* pUserData) {
	const uint8_t* pPrim = NULL;
	int nprim = HBIN_BGEO_FN(NumPrims)(bgeo);
	if (nprim >= 0) {
		const uint8_t* pTop = (const uint8_t*)bgeo;
		int nattr = HBIN_BGEO_FN(NumPrimAttrs)(bgeo);
		HBIN_PRIM_S prim;
		int npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		const uint8_t* pPts = NULL;
		int32_t pntRecSize = bgeoCalcPntRecSize(bgeo, &pPts);
		int nattrVtx = HBIN_BGEO_FN(NumVertexAttrs)(bgeo);
		int iprim = 0;
		int runLen = -1;
		int cont = 1;
		int32_t mtlValOffs = -1;
		int32_t type = 0;
		prim.pTop = pTop;
		prim.vtxAttrsOffs = 0;
		prim.primAttrsOffs = 0;
		prim.primRecsOffs = pPts ? (uint32_t)(pPts + (npts * pntRecSize) - pTop) : 0x29;
		prim.vtxAttrsRecSize = 0;
		prim.primAttrsRecSize = 0;
		prim.idxSize = npts > 0xFFFF ? 4 : 2;
		prim.recSize = 0;
		if (nattrVtx) {
			const uint8_t* pNext = NULL;
			prim.vtxAttrsOffs = prim.primRecsOffs;
			prim.vtxAttrsRecSize = bgeoCalcItemRecSize(bgeo, nattrVtx, prim.vtxAttrsOffs, 0, &pNext);
			prim.primRecsOffs = (uint32_t)(pNext - pTop);
		}
		if (nattr) {
			const uint8_t* pNext = NULL;
			prim.primAttrsOffs = prim.primRecsOffs;
			prim.primAttrsRecSize = bgeoCalcItemRecSize(bgeo, nattr, prim.primAttrsOffs, 0, &pNext);
			prim.primRecsOffs = (uint32_t)(pNext - pTop);
		}
		if (prim.primAttrsOffs) {
			int32_t valOffs = 0;
			int32_t attrType = 0;
			int32_t attrSize = 0;
			int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, prim.primAttrsOffs, 0, s_pBgeoMtlAttrName, NULL, &valOffs, &attrType, &attrSize, NULL);
			if (attrId >= 0 && attrType == 4) {
				mtlValOffs = valOffs;
			}
		}
		pPrim = pTop + prim.primRecsOffs;
		while (cont && iprim < nprim) {
			int isRun = 0;
			if (runLen <= 0) {
				type = hbinI32(pPrim);
				pPrim += 4;
				if (type == -1) {
					isRun = 1;
				}
				if (isRun) {
					runLen = hbinU16(pPrim);
					pPrim += 2;
					type = hbinI32(pPrim);
					pPrim += 4;
					--runLen;
				} else {
					runLen = 0;
				}
			} else {
				--runLen;
			}
			prim.id = iprim;
			prim.type = type;
			prim.pRec = pPrim;
			prim.pIdx = NULL;
			prim.nvtx = 0;
			prim.mtlId = -1;
			if (type == 1) {
				/* Poly */
				int nvtx = hbinI32(pPrim);
				prim.type = HBIN_PRIMTYPE_Poly;
				prim.nvtx = nvtx;
				prim.pIdx = prim.pRec + 4 + 1;
				prim.recSize = 4 + 1 + (prim.idxSize * nvtx) + (prim.vtxAttrsRecSize * nvtx) + prim.primAttrsRecSize;
				if (mtlValOffs >= 0) {
					prim.mtlId = hbinI32(prim.pIdx + ((prim.idxSize + prim.vtxAttrsRecSize) * nvtx) + mtlValOffs);
				}
				cont = callback ? callback(&prim, pUserData) : 1;
				pPrim += prim.recSize;
				++iprim;
			} else if (type == 0x2000) {
				/* Sphere */
				prim.type = HBIN_PRIMTYPE_Sphere;
				prim.nvtx = 1;
				prim.pIdx = prim.pRec;
				prim.recSize = prim.idxSize + (3 * 3 * 4) + prim.vtxAttrsRecSize + prim.primAttrsRecSize;
				if (mtlValOffs >= 0) {
					prim.mtlId = hbinI32(prim.pIdx + prim.idxSize + (3 * 3 * 4) + prim.vtxAttrsRecSize + mtlValOffs);
				}
				cont = callback ? callback(&prim, pUserData) : 1;
				pPrim += prim.recSize;
				++iprim;
			} else {
				cont = 0;
			}
		}
	}
	return pPrim;
}

HBIN_BGEO_IFC(void, ForEachPrim)(const HBIN_BGEO bgeo, HBIN_PRIM_CB callback, void* pUserData) {
	if (!callback) return;
	bgeoForEachPrimImpl(bgeo, callback, pUserData);
}

HBIN_BGEO_IFC(const HBIN_BGEO, PrimBgeo)(const HBIN_PRIM prim) {
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	return (pPrim ? (const HBIN_BGEO)pPrim->pTop : NULL);
}

HBIN_BGEO_IFC(int32_t, PrimIsPoly)(const HBIN_PRIM prim) {
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	return (pPrim && pPrim->type == HBIN_PRIMTYPE_Poly);
}

HBIN_BGEO_IFC(int32_t, PrimIsSphere)(const HBIN_PRIM prim) {
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	return (pPrim && pPrim->type == HBIN_PRIMTYPE_Sphere);
}

HBIN_BGEO_IFC(int32_t, PrimNumVertices)(const HBIN_PRIM prim) {
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	return pPrim ? pPrim->nvtx : 0;
}

HBIN_BGEO_IFC(int32_t, PrimVertexPntId)(const HBIN_PRIM prim, const int32_t vtxId) {
	int32_t pntId = -1;
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	int32_t nvtx = pPrim && pPrim->pIdx ? pPrim->nvtx : 0;
	if (nvtx > 0 && (uint32_t)vtxId < (uint32_t)nvtx) {
		const uint8_t* pIdx = pPrim->pIdx + (vtxId * (pPrim->idxSize + pPrim->vtxAttrsRecSize));
		if (pPrim->idxSize == 2) {
			pntId = hbinU16(pIdx);
		} else if (pPrim->idxSize == 4) {
			pntId = hbinI32(pIdx);
		}
	}
	return pntId;
}

HBIN_BGEO_IFC(int32_t, PrimMaterialId)(const HBIN_PRIM prim) {
	const HBIN_PRIM_S* pPrim = (const HBIN_PRIM_S*)prim;
	return pPrim ? pPrim->mtlId : -1;
}

HBIN_BGEO_IFC(int32_t, NumMaterials)(const HBIN_BGEO bgeo) {
	int32_t nmtl = 0;
	int32_t nattr = HBIN_BGEO_FN(NumPrimAttrs)(bgeo);
	if (nattr > 0) {
		int npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		const uint8_t* pPts = NULL;
		int32_t pntRecSize = bgeoCalcPntRecSize(bgeo, &pPts);
		int nattrVtx = HBIN_BGEO_FN(NumVertexAttrs)(bgeo);
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = pPts ? (uint32_t)(pPts + (npts * pntRecSize) - pTop) : 0x29;
		if (nattrVtx) {
			const uint8_t* pNext = NULL;
			bgeoCalcItemRecSize(bgeo, nattrVtx, attrsOffs, 0, &pNext);
			attrsOffs = (uint32_t)(pNext - pTop);
		}
		if (attrsOffs) {
			int32_t valOffs = 0;
			int32_t attrType = 0;
			int32_t attrSize = 0;
			const uint8_t* pDescr = NULL;
			int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, s_pBgeoMtlAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
			if (attrId >= 0 && attrType == 4 && pDescr) {
				nmtl = hbinI32(pDescr);
				if (nmtl < 0) {
					nmtl = 0;
				}
			}
		}
	}
	return nmtl;
}

HBIN_BGEO_IFC(HBIN_STRING, MaterialPath)(const HBIN_BGEO bgeo, const int32_t mtlId) {
	int32_t nmtl = 0;
	int32_t nattr = HBIN_BGEO_FN(NumPrimAttrs)(bgeo);
	HBIN_STRING path;
	path.pChars = NULL;
	path.len = 0;
	if (nattr > 0 && mtlId >= 0) {
		int npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		const uint8_t* pPts = NULL;
		int32_t pntRecSize = bgeoCalcPntRecSize(bgeo, &pPts);
		int nattrVtx = HBIN_BGEO_FN(NumVertexAttrs)(bgeo);
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = pPts ? (uint32_t)(pPts + (npts * pntRecSize) - pTop) : 0x29;
		if (nattrVtx) {
			const uint8_t* pNext = NULL;
			bgeoCalcItemRecSize(bgeo, nattrVtx, attrsOffs, 0, &pNext);
			attrsOffs = (uint32_t)(pNext - pTop);
		}
		if (attrsOffs) {
			int32_t valOffs = 0;
			int32_t attrType = 0;
			int32_t attrSize = 0;
			const uint8_t* pDescr = NULL;
			int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, s_pBgeoMtlAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
			if (attrId >= 0 && attrType == 4 && pDescr) {
				nmtl = hbinI32(pDescr);
				if (nmtl > 0) {
					pDescr += 4;
					if ((uint32_t)mtlId < (uint32_t)nmtl) {
						int32_t i;
						int32_t len = 0;
						for (i = 0; i <= mtlId; ++i) {
							len = hbinI16(pDescr);
							pDescr += 2;
							if (len < 0) {
								len = hbinI32(pDescr);
								pDescr += 4;
							}
							path.pChars = (const char*)pDescr;
							pDescr += len;
						}
						path.len = len;
					}
				}
			}
		}
	}
	return path;
}

static const uint8_t* bgeoDetailAttrsTop(const HBIN_BGEO bgeo) {
	const uint8_t* pAttrs = NULL;
	int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
	if (nattr > 0) {
		int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
		int32_t nprims = HBIN_BGEO_FN(NumPrims)(bgeo);
		int nattrPnt = HBIN_BGEO_FN(NumPointAttrs)(bgeo);
		int nattrVtx = HBIN_BGEO_FN(NumVertexAttrs)(bgeo);
		int nattrPrim = HBIN_BGEO_FN(NumPrimAttrs)(bgeo);
		if (npts == 0 && nprims == 0 && nattrPnt == 0 && nattrVtx == 0 && nattrPrim == 0) {
			const uint8_t* pTop = (const uint8_t*)bgeo;
			pAttrs = pTop + 0x29;
		} else {
			pAttrs = bgeoForEachPrimImpl(bgeo, NULL, NULL);
		}
	}
	return pAttrs;
}

HBIN_BGEO_IFC(HBIN_STRING, DetailStrAttr)(const HBIN_BGEO bgeo, const char* pAttrName) {
	const uint8_t* pAttrs = NULL;
	HBIN_STRING str;
	str.pChars = NULL;
	str.len = 0;
	pAttrs = bgeoDetailAttrsTop(bgeo);
	if (pAttrs) {
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0) {
			if (attrType == 4 && attrSize > 0) {
				int32_t nstrs = hbinI32(pDescr);
				if (nstrs > 0) {
					str.len = hbinI16(pDescr + 4);
					str.pChars = (const char*)(pDescr + 6);
				}
			}
		}
	}
	return str;
}

HBIN_BGEO_IFC(int32_t, NumCaptureNodes)(const HBIN_BGEO bgeo) {
	int32_t n = 0;
	const uint8_t* pAttrs = bgeoDetailAttrsTop(bgeo);
	if (pAttrs) {
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, s_pBgeoCaptPathName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0) {
			if (attrType == 4 && attrSize > 0) {
				n = hbinI32(pDescr);
			}
		}
	}
	return n;
}

HBIN_BGEO_IFC(HBIN_STRING, CaptureNodePath)(const HBIN_BGEO bgeo, const int32_t nodeId) {
	const uint8_t* pAttrs = bgeoDetailAttrsTop(bgeo);
	HBIN_STRING path;
	path.pChars = NULL;
	path.len = 0;
	if (pAttrs && nodeId >= 0) {
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, s_pBgeoCaptPathName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0) {
			int32_t i;
			int32_t ncap = 0;
			int32_t len = 0;
			if (attrType == 4 && attrSize > 0) {
				ncap = hbinI32(pDescr);
			}
			if (ncap > 0 && nodeId < ncap) {
				pDescr += 4;
				for (i = 0; i <= nodeId; ++i) {
					len = hbinI16(pDescr);
					pDescr += 2;
					if (len < 0) {
						len = hbinI32(pDescr);
						pDescr += 4;
					}
					path.pChars = (const char*)pDescr;
					pDescr += len;
				}
				path.len = len;
			}
		}
	}
	return path;
}

HBIN_BGEO_IFC(int32_t, MaxCapturesPerPoint)(const HBIN_BGEO bgeo) {
	int32_t nwgt = 0;
	int32_t nattr = HBIN_BGEO_FN(NumPointAttrs)(bgeo);
	if (nattr) {
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, s_pBgeoCaptAttrName, NULL, &valOffs, &attrType, &attrSize, NULL);
		if (attrId >= 0 && attrType == 0x10000) {
			nwgt = attrSize / 2;
		}
	}
	return nwgt;
}

HBIN_BGEO_IFC(HBIN_CAPTURE, PointCapture)(const HBIN_BGEO bgeo, const int32_t pntId, const int32_t wgtId) {
	int32_t npts = HBIN_BGEO_FN(NumPoints)(bgeo);
	int32_t nattr = npts > 0 ? HBIN_BGEO_FN(NumPointAttrs)(bgeo) : 0;
	HBIN_CAPTURE capt;
	capt.node = -1;
	capt.wght = 0.0f;
	if (nattr && (uint32_t)pntId < (uint32_t)npts) {
		int32_t nwgt = 0;
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, 0x29, 4 * 4, s_pBgeoCaptAttrName, NULL, &valOffs, &attrType, &attrSize, NULL);
		if (attrId >= 0 && attrType == 0x10000) {
			nwgt = attrSize / 2;
		}
		if (nwgt > 0 && (uint32_t)wgtId < (uint32_t)nwgt) {
			const uint8_t* pPts = NULL;
			int32_t recSize = bgeoCalcPntRecSize(bgeo, &pPts);
			if (recSize > 0 && pPts) {
				const uint8_t* pWgt = pPts + (pntId * recSize) + valOffs + (wgtId * 8);
				capt.node = (int32_t)hbinF32(pWgt);
				capt.wght = hbinF32(pWgt + 4);
			}
		}
	}
	return capt;
}

HBIN_BGEO_IFC(int32_t, SkeletonNames)(const HBIN_BGEO bgeo, const char* pAttrName, HBIN_STRING* pNames) {
	int32_t n = 0;
	const uint8_t* pAttrs = bgeoDetailAttrsTop(bgeo);
	if (pAttrs > 0 && pAttrName) {
		int32_t i;
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0 && attrType == 4 && pDescr) {
			n = hbinI32(pDescr);
			if (n > 0 && pNames) {
				pDescr += 4;
				for (i = 0; i < n; ++i) {
					pNames[i].len = hbinU16(pDescr);
					pDescr += 2;
					if (pNames[i].len == 0xFFFF) {
						pNames[i].len = hbinI32(pDescr);
						pDescr += 4;
					}
					pNames[i].pChars = (const char*)pDescr;
					pDescr += pNames[i].len;
				}
			}
		}
	}
	return n;
}

HBIN_BGEO_IFC(int32_t, SkeletonParents)(const HBIN_BGEO bgeo, const char* pAttrName, int32_t* pParents) {
	int32_t n = 0;
	const uint8_t* pAttrs = bgeoDetailAttrsTop(bgeo);
	if (pAttrs > 0 && pAttrName) {
		int32_t i;
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0 && attrType == 1 && pDescr) {
			n = attrSize;
			if (n > 0 && pParents) {
				const uint8_t* pVals = NULL;
				bgeoCalcItemRecSize(bgeo, nattr, attrsOffs, 0, &pVals);
				if (pVals) {
					for (i = 0; i < n; ++i) {
						pParents[i] = hbinI32(pVals + valOffs + (i * 4));
					}
				}
			}
		}
	}
	return n;
}

HBIN_BGEO_IFC(int32_t, SkeletonTransforms)(const HBIN_BGEO bgeo, const char* pAttrName, float* pXforms) {
	int32_t n = 0;
	const uint8_t* pAttrs = bgeoDetailAttrsTop(bgeo);
	if (pAttrs > 0 && pAttrName) {
		int32_t i;
		const uint8_t* pTop = (const uint8_t*)bgeo;
		uint32_t attrsOffs = (uint32_t)(pAttrs - pTop);
		int32_t nattr = HBIN_BGEO_FN(NumDetailAttrs)(bgeo);
		int32_t valOffs = 0;
		int32_t attrType = 0;
		int32_t attrSize = 0;
		const uint8_t* pDescr = NULL;
		int32_t attrId = bgeoFindAttrInfo(bgeo, nattr, attrsOffs, 0, pAttrName, NULL, &valOffs, &attrType, &attrSize, &pDescr);
		if (attrId >= 0 && attrType == 0 && pDescr) {
			n = attrSize;
			if (n > 0 && pXforms) {
				const uint8_t* pVals = NULL;
				bgeoCalcItemRecSize(bgeo, nattr, attrsOffs, 0, &pVals);
				if (pVals) {
					for (i = 0; i < n; ++i) {
						pXforms[i] = hbinF32(pVals + valOffs + (i * 4));
					}
				}
			}
		}
	}
	return n;
}

HBIN_BGEO_IFC(void, MakeVertexBuffer)(
	const HBIN_BGEO bgeo,
	void* pMem, const int32_t stride,
	const int32_t posOffs, const int32_t nrmOffs,
	const int32_t rgbOffs, const int32_t texOffs,
	const int32_t wgtOffs, const int32_t idxOffs,
	const int32_t maxWghts, int32_t* pInflCounts)
{
	int32_t i, j;
	int32_t numVtxCapts = 0;
	int32_t maxCapts = 0;
	int32_t nvtx = 0;
	int32_t numCaptNodes = 0;
	if (!HBIN_BGEO_FN(Valid)(bgeo)) return;
	if (!pMem) return;
	if (stride <= 0) return;
	nvtx = HBIN_BGEO_FN(NumPoints)(bgeo);
	if (nvtx < 1) return;
	if (maxWghts > 0 && wgtOffs > 0 && idxOffs > 0) {
		maxCapts = HBIN_BGEO_FN(MaxCapturesPerPoint)(bgeo);
	}
	if (maxCapts > 0) {
		if (maxWghts < maxCapts) {
			numVtxCapts = maxWghts;
		} else {
			numVtxCapts = maxCapts;
		}
	}
	if (pInflCounts) {
		numCaptNodes = bgeoNumCaptureNodes(bgeo);
		for (i = 0; i < numCaptNodes; ++i) {
			pInflCounts[i] = 0;
		}
	}
	for (i = 0; i < nvtx; ++i) {
		uint8_t* pVtx = (uint8_t*)pMem + (i * stride);
		if (posOffs >= 0) {
			HBIN_BGEO_FN(PointPos)(*(HBIN_FLOAT3*)(pVtx + posOffs), bgeo, i);
		}
		if (nrmOffs >= 0) {
			HBIN_BGEO_FN(PointNrm)(*(HBIN_FLOAT3*)(pVtx + nrmOffs), bgeo, i);
		}
		if (rgbOffs >= 0) {
			HBIN_BGEO_FN(PointRGB)(*(HBIN_FLOAT3*)(pVtx + rgbOffs), bgeo, i);
		}
		if (texOffs >= 0) {
			HBIN_FLOAT2* pTex = (HBIN_FLOAT2*)(pVtx + texOffs);
			HBIN_BGEO_FN(PointUV)(*pTex, bgeo, i);
			(*pTex)[1] = 1.0f - (*pTex)[1];
		}
		if (wgtOffs >= 0 || idxOffs >= 0) {
			float* pWgt = wgtOffs < 0 ? NULL : (float*)(pVtx + wgtOffs);
			int32_t* pIdx = idxOffs < 0 ? NULL : (int32_t*)(pVtx + idxOffs);
			if (pWgt) {
				for (j = 0; j < maxWghts; ++j) {
					pWgt[j] = 0.0f;
				}
				if (maxWghts > 0) {
					pWgt[0] = 1.0f;
				}
			}
			if (pIdx) {
				for (j = 0; j < maxWghts; ++j) {
					pIdx[j] = 0;
				}
			}
			for (j = 0; j < numVtxCapts; ++j) {
				HBIN_CAPTURE capt = HBIN_BGEO_FN(PointCapture)(bgeo, i, j);
				if (pWgt) {
					pWgt[j] = capt.wght;
				}
				if (pIdx) {
					pIdx[j] = capt.node;
				}
			}
			for (j = 0; j < numVtxCapts; ++j) {
				if (pIdx[j] < 0) {
					pWgt[j] = 0.0f;
				}
				if (pWgt[j] <= 0.0f) {
					pIdx[j] = 0;
					pWgt[j] = 0.0f;
				}
			}
			if (pInflCounts) {
				for (j = 0; j < numVtxCapts; ++j) {
					if (pWgt[j] > 0.0f) {
						++pInflCounts[pIdx[j]];
					}
				}
			}
		}
	}
}

typedef struct _BGEO_GETTRIS_WK {
	uint16_t* pIdx16;
	uint32_t* pIdx32;
	int32_t* pMtlIds;
	int32_t triCount;
} BGEO_GETTRIS_WK;

static int bgeoGetTrisCB(const HBIN_PRIM prim, void* pUserData) {
	int32_t i;
	BGEO_GETTRIS_WK* pWk = (BGEO_GETTRIS_WK*)pUserData;
	if (!pWk) return 0;
	if (bgeoPrimIsPoly(prim)) {
		int32_t nvtx = bgeoPrimNumVertices(prim);
		if (nvtx == 3) {
			if (pWk->pIdx16) {
				int idxOrg = pWk->triCount * 3;
				for (i = 0; i < 3; ++i) {
					pWk->pIdx16[idxOrg + i] = (uint16_t)bgeoPrimVertexPntId(prim, i);
				}
			}
			if (pWk->pIdx32) {
				int idxOrg = pWk->triCount * 3;
				for (i = 0; i < 3; ++i) {
					pWk->pIdx32[idxOrg + i] = (uint32_t)bgeoPrimVertexPntId(prim, i);
				}
			}
			if (pWk->pMtlIds) {
				pWk->pMtlIds[pWk->triCount] = bgeoPrimMaterialId(prim);
			}
			++pWk->triCount;
		}
	}
	return 1;
}

HBIN_BGEO_IFC(int32_t, GetTriangles)(
	const HBIN_BGEO bgeo, uint16_t* pIdx16, uint32_t* pIdx32, int32_t* pMtlIds)
{
	int32_t ntris = 0;
	int32_t nprim = HBIN_BGEO_FN(NumPrims)(bgeo);
	if (nprim > 0) {
		BGEO_GETTRIS_WK wk;
		wk.pIdx16 = pIdx16;
		wk.pIdx32 = pIdx32;
		wk.pMtlIds = pMtlIds;
		wk.triCount = 0;
		HBIN_BGEO_FN(ForEachPrim)(bgeo, bgeoGetTrisCB, &wk);
		ntris = wk.triCount;
	}
	return ntris;
}


HBIN_BCLIP_IFC(int, Valid)(const HBIN_BCLIP bclip) {
	int valid = 0;
	const uint8_t* pTop = (const uint8_t*)bclip;
	if (pTop) {
		static const char* pSig = "bclp";
		if (hbinMemCmp(pTop, pSig, 4) == 0) {
			valid = 1;
		}
	}
	return valid;
}

static const uint8_t* bclipFindPacket(const HBIN_BCLIP bclip, const int32_t pktTag) {
	const uint8_t* pPkt = NULL;
	if (HBIN_BCLIP_FN(Valid)(bclip)) {
		const uint8_t* pTop = (const uint8_t*)bclip;
		const uint8_t* pMem = pTop + 4;
		while (1) {
			int32_t pktSize = hbinI32(pMem);
			if (pktSize > 0) {
				uint16_t tag = hbinU16(pMem + 4);
				if (tag != 0xF) {
					break;
				}
				tag = hbinU16(pMem + 6);
				if (tag == pktTag) {
					pPkt = pMem;
					break;
				}
				if (tag == 0) {
					/* END */
					break;
				}
				pMem += pktSize;
			} else {
				break;
			}
		}
	}
	return pPkt;
}

HBIN_BCLIP_IFC(int32_t, Version)(const HBIN_BCLIP bclip) {
	int32_t ver = 0;
	const uint8_t* pVer = bclipFindPacket(bclip, 9);
	if (pVer) {
		ver = hbinI32(pVer + 8);
	}
	return ver;
}

static int bclipIsInfoF64(const HBIN_BCLIP bclip) {
	int isDbl = 0;
	const uint8_t* pTyp = bclipFindPacket(bclip, 0xA);
	if (pTyp) {
		isDbl = pTyp[8];
	}
	return isDbl;
}

HBIN_BCLIP_IFC(double, SampleRate)(const HBIN_BCLIP bclip) {
	double rate = 0.0;
	const uint8_t* pRate = bclipFindPacket(bclip, 1);
	if (pRate) {
		if (bclipIsInfoF64(bclip)) {
			rate = hbinF64(pRate + 8);
		} else {
			rate = hbinF32(pRate + 8);
		}
	}
	return rate;
}

HBIN_BCLIP_IFC(double, StartIndex)(const HBIN_BCLIP bclip) {
	double start = 0.0;
	const uint8_t* pStart = bclipFindPacket(bclip, 2);
	if (pStart) {
		if (bclipIsInfoF64(bclip)) {
			start = hbinF64(pStart + 8);
		} else {
			start = hbinF32(pStart + 8);
		}
	}
	return start;
}

HBIN_BCLIP_IFC(int32_t, TrackLength)(const HBIN_BCLIP bclip) {
	int32_t len = 0;
	const uint8_t* pLen = bclipFindPacket(bclip, 3);
	if (pLen) {
		len = hbinI32(pLen + 8);
	}
	return len;
}

HBIN_BCLIP_IFC(int32_t, NumTracks)(const HBIN_BCLIP bclip) {
	int n = 0;
	const uint8_t* pTracks = bclipFindPacket(bclip, 5);
	if (pTracks) {
		n = hbinI32(pTracks + 8);
	}
	return n;
}

static int bclipIsTrackF64(const HBIN_BCLIP bclip) {
	int isDbl = 0;
	const uint8_t* pTyp = bclipFindPacket(bclip, 8);
	if (pTyp) {
		isDbl = pTyp[8];
	}
	return isDbl;
}

HBIN_BCLIP_IFC(void, AllTracks)(const HBIN_BCLIP bclip, float* pSmps /* [numTracks][trackLen] */, HBIN_STRING* pNames /* [numTracks] */) {
	int32_t i, j;
	int isDbl = 0;
	int32_t ntrk = 0;
	int32_t nsmp = 0;
	const uint8_t* pTrk = NULL;
	if (!HBIN_BCLIP_FN(Valid)(bclip)) return;
	ntrk = HBIN_BCLIP_FN(NumTracks)(bclip);
	nsmp = HBIN_BCLIP_FN(TrackLength)(bclip);
	isDbl = bclipIsTrackF64(bclip);
	pTrk = bclipFindPacket(bclip, 5);
	if (!pTrk) return;
	pTrk += 0xC;
	for (i = 0; i < ntrk; ++i) {
		while (1) {
			int32_t pktLen = hbinI32(pTrk);
			int32_t pktTag = hbinU16(pTrk + 4);
			if (pktTag != 0x10) return;
			pktTag = hbinU16(pTrk + 6);
			if (pktTag == 1) {
				if (pNames) {
					pNames[i].len = hbinU16(pTrk + 8);
					if (pNames[i].len == 0xFFFF) {
						pNames[i].len = hbinU32(pTrk + 0xA);
						pNames[i].pChars = (const char*)(pTrk + 0xE);
					} else {
						pNames[i].pChars = (const char*)(pTrk + 0xA);
					}
				}
			} else if (pktTag == 2) {
				if (pSmps) {
					float* pDst = pSmps + (i * nsmp);
					const uint8_t* pSrc = pTrk + 8;
					if (isDbl) {
						for (j = 0; j < nsmp; ++j) {
							*pDst = (float)hbinF64(pSrc);
							++pDst;
							pSrc += 8;
						}
					} else {
						for (j = 0; j < nsmp; ++j) {
							*pDst = hbinF32(pSrc);
							++pDst;
							pSrc += 4;
						}
					}
				}
			}
			pTrk += pktLen;
			if (pktTag == 0) { /* END */
				break;
			}
		}
	}
}
