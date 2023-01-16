#include "crosscore.hpp"
#include "hbin.h"
#include "hbin2json.hpp"

struct BgeoContext {
	HBIN_BGEO bgeo;
	size_t aryCnt;
	int num;
	FILE* pOut;
};


static int triIdxPrimCB(const HBIN_PRIM prim, void* pCtxMem) {
	BgeoContext* pCtx = (BgeoContext*)pCtxMem;
	if (!pCtx) return 0;
	if (!pCtx->pOut) return 0;
	if (bgeoPrimIsPoly(prim)) {
		int32_t nvtx = bgeoPrimNumVertices(prim);
		if (nvtx == 3) {
			for (int j = 0; j < 3; ++j) {
				int32_t pid = bgeoPrimVertexPntId(prim, j);
				::fprintf(pCtx->pOut, "%d", pid);
				if (j < 2) {
					::fprintf(pCtx->pOut, ", ");
				}
			}
			--pCtx->aryCnt;
			if (pCtx->aryCnt > 0) {
				::fprintf(pCtx->pOut, ", ");
			}
		}
	}
	return 1;
}

static int polIdxPrimCB(const HBIN_PRIM prim, void* pCtxMem) {
	BgeoContext* pCtx = (BgeoContext*)pCtxMem;
	if (!pCtx) return 0;
	if (!pCtx->pOut) return 0;
	if (bgeoPrimIsPoly(prim)) {
		int32_t nvtx = bgeoPrimNumVertices(prim);
		for (int32_t j = 0; j < nvtx; ++j) {
			int32_t pid = bgeoPrimVertexPntId(prim, j);
			::fprintf(pCtx->pOut, "%d", pid);
			if (j < nvtx - 1) {
				::fprintf(pCtx->pOut, ", ");
			}
		}
		--pCtx->aryCnt;
		if (pCtx->aryCnt > 0) {
			::fprintf(pCtx->pOut, ", ");
		}
	}
	return 1;
}

static int polRangePrimCB(const HBIN_PRIM prim, void* pCtxMem) {
	BgeoContext* pCtx = (BgeoContext*)pCtxMem;
	if (!pCtx) return 0;
	if (!pCtx->pOut) return 0;
	if (bgeoPrimIsPoly(prim)) {
		int nvtx = bgeoPrimNumVertices(prim);
		::fprintf(pCtx->pOut, "%d, %d", pCtx->num, nvtx);
		pCtx->num += nvtx;
		--pCtx->aryCnt;
		if (pCtx->aryCnt > 0) {
			::fprintf(pCtx->pOut, ", ");
		}
	}
	return 1;
}

void write_bgeo_json(HBIN_BGEO bgeo, FILE* pOut) {
	if (!bgeoValid(bgeo)) return;
	if (pOut == nullptr) pOut = stdout;
	char nameBuf[256];
	BgeoContext ctx;
	ctx.bgeo = bgeo;
	ctx.pOut = pOut;
	int npnt = bgeoNumPoints(bgeo);
	int ntri = bgeoCountTriangles(bgeo);
	int npol = bgeoCountPolygons(bgeo);
	int nmtl = bgeoNumMaterials(bgeo);
	int npntAttrs = bgeoNumPointAttrs(bgeo);
	int nprimAttrs = bgeoNumPrimAttrs(bgeo);
	int ncaptNodes = bgeoNumCaptureNodes(bgeo);
	int maxCaptsPerPnt = bgeoMaxCapturesPerPoint(bgeo);
	int npntVecAttrs = 0;
	for (int i = 0; i < npntAttrs; ++i) {
		if (bgeoPointAttrIsVec(bgeo, i)) {
			++npntVecAttrs;
		}
	}
	int npntStrAttrs = 0;
	for (int i = 0; i < npntAttrs; ++i) {
		if (bgeoPointAttrIsStr(bgeo, i)) {
			++npntStrAttrs;
		}
	}
	::fprintf(pOut, "{\n");
	::fprintf(pOut, "  \"dataType\" : \"geo\",\n");
	::fprintf(pOut, "  \"npnt\" : %d,\n", npnt);
	::fprintf(pOut, "  \"ntri\" : %d,\n", ntri);
	::fprintf(pOut, "  \"npol\" : %d,\n", npol);
	::fprintf(pOut, "  \"nmtl\" : %d,\n", nmtl);
	::fprintf(pOut, "  \"npntAttrs\" : %d,\n", npntAttrs);
	::fprintf(pOut, "  \"npntVecAttrs\" : %d,\n", npntVecAttrs);
	::fprintf(pOut, "  \"npntStrAttrs\" : %d,\n", npntStrAttrs);
	::fprintf(pOut, "  \"nprimAttrs\" : %d,\n", nprimAttrs);
	::fprintf(pOut, "  \"ncaptNodes\" : %d,\n", ncaptNodes);
	::fprintf(pOut, "  \"maxCaptsPerPnt\" : %d,\n", maxCaptsPerPnt);
	::fprintf(pOut, "  \"pntAttrNames : \" : [");
	for (int i = 0; i < npntAttrs; ++i) {
		HBIN_STRING attrName = bgeoPointAttrName(bgeo, i);
		::fprintf(pOut, "\"");
		hbin_str_out(pOut, attrName);
		::fprintf(pOut, "\"");
		if (i < npntAttrs-1) {
			::fprintf(pOut, ", ");
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntVecAttrNames\" : [");
	if (npntVecAttrs > 0) {
		size_t aryCnt = npntVecAttrs;
		for (int i = 0; i < npntAttrs; ++i) {
			if (bgeoPointAttrIsVec(bgeo, i)) {
				HBIN_STRING attrName = bgeoPointAttrName(bgeo, i);
				::fprintf(pOut, "\"");
				hbin_str_out(pOut, attrName);
				::fprintf(pOut, "\"");
				--aryCnt;
				if (aryCnt > 0) {
					::fprintf(pOut, ", ");
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntStrAttrNames\" : [");
	if (npntStrAttrs > 0) {
		size_t aryCnt = npntStrAttrs;
		for (int i = 0; i < npntAttrs; ++i) {
			if (bgeoPointAttrIsStr(bgeo, i)) {
				HBIN_STRING attrName = bgeoPointAttrName(bgeo, i);
				::fprintf(pOut, "\"");
				hbin_str_out(pOut, attrName);
				::fprintf(pOut, "\"");
				--aryCnt;
				if (aryCnt > 0) {
					::fprintf(pOut, ", ");
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"captNodes\" : [");
	if (ncaptNodes > 0) {
		size_t aryCnt = ncaptNodes;
		for (int i = 0; i < ncaptNodes; ++i) {
			HBIN_STRING nodePath = bgeoCaptureNodePath(bgeo, i);
			::fprintf(pOut, "\"");
			hbin_str_out(pOut, nodePath);
			::fprintf(pOut, "\"");
			--aryCnt;
			if (aryCnt > 0) {
				::fprintf(pOut, ", ");
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pnts\" : [");
	for (int i = 0; i < npnt; ++i) {
		HBIN_FLOAT3 pos;
		bgeoPointPos(pos, bgeo, i);
		::fprintf(pOut, "%f, %f, %f", pos[0], pos[1], pos[2]);
		if (i < npnt-1) {
			::fprintf(pOut, ", ");
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntsVecData\" : [");
	if (npntVecAttrs > 0) {
		size_t aryCnt = npnt * npntVecAttrs;
		for (int i = 0; i < npntAttrs; ++i) {
			if (bgeoPointAttrIsVec(bgeo, i)) {
				HBIN_STRING attrName = bgeoPointAttrName(bgeo, i);
				if (attrName.pChars && attrName.len > 0 && attrName.len < sizeof(nameBuf) - 1) {
					nxCore::mem_copy(nameBuf, attrName.pChars, attrName.len);
					nameBuf[attrName.len] = 0;
					HBIN_FLOAT3 vec;
					for (int j = 0; j < npnt; ++j) {
						bgeoPointVecAttr(vec, bgeo, nameBuf, j);
						::fprintf(pOut, "%f, %f, %f", vec[0], vec[1], vec[2]);
						--aryCnt;
						if (aryCnt > 0) {
							::fprintf(pOut, ", ");
						}
					}
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntsStrData\" : [");
	if (npntStrAttrs > 0) {
		size_t aryCnt = npnt * npntStrAttrs;
		for (int i = 0; i < npntAttrs; ++i) {
			if (bgeoPointAttrIsStr(bgeo, i)) {
				HBIN_STRING attrName = bgeoPointAttrName(bgeo, i);
				if (attrName.pChars && attrName.len > 0 && attrName.len < sizeof(nameBuf) - 1) {
					nxCore::mem_copy(nameBuf, attrName.pChars, attrName.len);
					nameBuf[attrName.len] = 0;
					for (int j = 0; j < npnt; ++j) {
						HBIN_STRING hstr = bgeoPointStrAttr(bgeo, nameBuf, j);
						::fprintf(pOut, "\"");
						hbin_str_out(pOut, hstr);
						::fprintf(pOut, "\"");
						--aryCnt;
						if (aryCnt > 0) {
							::fprintf(pOut, ", ");
						}
					}
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntsCaptNodes\" : [");
	if (maxCaptsPerPnt > 0) {
		size_t aryCnt = npnt * maxCaptsPerPnt;
		for (int i = 0; i < npnt; ++i) {
			for (int j = 0; j < maxCaptsPerPnt; ++j) {
				HBIN_CAPTURE capt = bgeoPointCapture(bgeo, i, j);
				::fprintf(pOut, "%d", capt.node);
				--aryCnt;
				if (aryCnt > 0) {
					::fprintf(pOut, ", ");
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pntsCaptWeights\" : [");
	if (maxCaptsPerPnt > 0) {
		size_t aryCnt = npnt * maxCaptsPerPnt;
		for (int i = 0; i < npnt; ++i) {
			for (int j = 0; j < maxCaptsPerPnt; ++j) {
				HBIN_CAPTURE capt = bgeoPointCapture(bgeo, i, j);
				::fprintf(pOut, "%f", capt.wght);
				--aryCnt;
				if (aryCnt > 0) {
					::fprintf(pOut, ", ");
				}
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"triIdx\" : [");
	if (ntri > 0) {
		ctx.aryCnt = ntri;
		bgeoForEachPrim(bgeo, triIdxPrimCB, &ctx);
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"polIdx\" : [");
	if (npol > 0 && npol != ntri) {
		ctx.aryCnt = npol;
		bgeoForEachPrim(bgeo, polIdxPrimCB, &ctx);
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"pols\" : [");
	if (npol > 0 && npol != ntri) {
		ctx.aryCnt = npol;
		ctx.num = 0;
		bgeoForEachPrim(bgeo, polRangePrimCB, &ctx);
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"_EOF_\" : true\n");
	::fprintf(pOut, "}\n");
}

void cvt_bgeo(const char* pBgeoPath, const char* pOutPath) {
	if (!pBgeoPath) return;
	size_t bgeoSize = 0;
	void* pBin = nxCore::bin_load(pBgeoPath, &bgeoSize);
	if (!pBin) {
		return;
	}
	FILE* pOut = nullptr;
	if (pOutPath) {
		pOut = nxSys::fopen_w_txt(pOutPath);
		if (!pOut) {
			return;
		}
	}
	write_bgeo_json((HBIN_BGEO)pBin, pOut);
	if (pOutPath) {
		::fclose(pOut);
	}
}


