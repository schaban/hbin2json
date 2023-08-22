#include "crosscore.hpp"
#include "hbin.h"
#include "hbin2json.hpp"

void write_bclip_json(HBIN_BCLIP bclip, FILE* pOut) {
	if (!bclipValid(bclip)) return;
	if (pOut == nullptr) pOut = stdout;
	float fps = bclipSampleRate(bclip);
	int32_t start = bclipStartIndex(bclip);
	int32_t frames = bclipTrackLength(bclip);
	int32_t tracks = bclipNumTracks(bclip);
	int nsmps = tracks * frames;
	float* pSmps = (float*)nxCore::mem_alloc(nsmps * sizeof(float), "bclip:samples");
	HBIN_STRING* pNames = (HBIN_STRING*)nxCore::mem_alloc(tracks * sizeof(HBIN_STRING), "bclip:names");
	if (!(pSmps && pNames)) {
		return;
	}
	bool smpU8 = false;
	const char* pFmtOpt = nxApp::get_opt("smpfmt");
	if (pFmtOpt) {
		if (nxCore::str_eq(pFmtOpt, "byte") || nxCore::str_eq(pFmtOpt, "u8")) {
			smpU8 = true;
		}
	}
	bclipAllTracks(bclip, pSmps, pNames);
	::fprintf(pOut, "{\n");
	::fprintf(pOut, "  \"dataType\" : \"clip\",\n");
	::fprintf(pOut, "  \"fps\" : %f,\n", fps);
	::fprintf(pOut, "  \"start\" : %d,\n", start);
	::fprintf(pOut, "  \"frames\" : %d,\n", frames);
	::fprintf(pOut, "  \"tracks\" : %d,\n", tracks);
	if (nxApp::get_bool_opt("chnames", true)) {
		::fprintf(pOut, "  \"names\" : [");
		for (int i = 0; i < tracks; ++i) {
			::fprintf(pOut, "\"");
			hbin_str_out(pOut, pNames[i]);
			::fprintf(pOut, "\"");
			if (i < tracks - 1) {
				::fprintf(pOut, ", ");
			}
		}
		::fprintf(pOut, "],\n");
	}
	::fprintf(pOut, "  \"samples\" : [");
	if (smpU8) {
		for (int i = 0; i < nsmps; ++i) {
			float fval = pSmps[i];
			fval = nxCalc::clamp(fval, 0.0f, 255.0f);
			uint8_t uval = (uint8_t)fval;
			::fprintf(pOut, "%d", uval);
			if (i < nsmps - 1) {
				::fprintf(pOut, ", ");
			}
		}
	} else {
		for (int i = 0; i < nsmps; ++i) {
			::fprintf(pOut, "%f", pSmps[i]);
			if (i < nsmps - 1) {
				::fprintf(pOut, ", ");
			}
		}
	}
	::fprintf(pOut, "],\n");
	::fprintf(pOut, "  \"_EOF_\" : true\n");
	::fprintf(pOut, "}\n");
}


void cvt_bclip(const char* pBclipPath, const char* pOutPath) {
	if (!pBclipPath) return;
	size_t bclipSize = 0;
	void* pBin = nxCore::bin_load(pBclipPath, &bclipSize);
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
	write_bclip_json((HBIN_BCLIP)pBin, pOut);
	if (pOutPath) {
		::fclose(pOut);
	}
}


