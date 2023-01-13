#include "crosscore.hpp"
#include "hbin.h"
#include "hbin2json.hpp"


static void dbgmsg_impl(const char* pMsg) {
	::fprintf(stderr, "%s", pMsg);
	::fflush(stderr);
}

static void init_sys() {
	sxSysIfc sysIfc;
	nxCore::mem_zero(&sysIfc, sizeof(sysIfc));
	sysIfc.fn_dbgmsg = dbgmsg_impl;
	nxSys::init(&sysIfc);
}

static void reset_sys() {
}

void hbin_str_out(FILE* pOut, HBIN_STRING str) {
	if (str.pChars && str.len > 0) {
		if (pOut == nullptr) pOut = stdout;
		for (size_t i = 0; i < str.len; ++i) {
			::fprintf(pOut, "%c", str.pChars[i]);
		}
	}
}


int main(int argc, char* argv[]) {
	nxApp::init_params(argc, argv);
	init_sys();

	if (nxApp::get_args_count() < 1) {
		nxCore::dbg_msg("nbin2json <path>\n");
	} else {
		const char* pSrcPath = nxApp::get_arg(0);
		if (nxCore::str_ends_with(pSrcPath, ".bhclassic") || nxCore::str_ends_with(pSrcPath, ".bgeo")) {
			cvt_bgeo(pSrcPath);
		} else if (nxCore::str_ends_with(pSrcPath, ".bclip")) {
			cvt_bclip(pSrcPath);
		}
	}

	nxApp::reset();
	//nxCore::mem_dbg();
	reset_sys();

	return 0;
}


