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

static void fmtU64(char* pBuf, size_t bsize, uint64_t val) {
	uint32_t* p = (uint32_t*)&val;
	uint32_t lo = p[0];
	uint32_t hi = p[1];
	XD_SPRINTF(XD_SPRINTF_BUF(pBuf, bsize), "%08X%08X", hi, lo);
}

class TokEcho : public cxLexer::TokenFunc {
public:
virtual bool operator()(const cxLexer::Token& tok) {
	char str64[32];
	if (tok.id == cxLexer::TokId::TOK_FLOAT) {
		nxCore::dbg_msg("float %.12f @ (%d, %d)\n", tok.val.f, tok.loc.line, tok.loc.column);
	} else if (tok.id == cxLexer::TokId::TOK_INT) {
		fmtU64(str64, sizeof(str64), tok.val.i);
		nxCore::dbg_msg("int 0x%s @ (%d, %d)\n", str64, tok.loc.line, tok.loc.column);
	} else if (tok.is_punctuation()) {
		nxCore::dbg_msg("punct %s @ (%d, %d)\n", tok.val.c, tok.loc.line, tok.loc.column);
	} else if (tok.id == cxLexer::TokId::TOK_QSTR) {
		nxCore::dbg_msg("\"%s\" @ (%d, %d)\n", (char*)tok.val.p, tok.loc.line, tok.loc.column);
	} else if (tok.id == cxLexer::TokId::TOK_SQSTR) {
		nxCore::dbg_msg("'%s' @ (%d, %d)\n", (char*)tok.val.p, tok.loc.line, tok.loc.column);
	} else if (tok.is_symbol()) {
		nxCore::dbg_msg("symbol %s @ (%d, %d)\n", (char*)tok.val.p, tok.loc.line, tok.loc.column);
	} else if (tok.is_keyword()) {
		nxCore::dbg_msg("keyword %s @ (%d, %d)\n", (char*)tok.val.p, tok.loc.line, tok.loc.column);
	} else {
		nxCore::dbg_msg("unknown @ (%d, %d)\n", tok.loc.line, tok.loc.column);
	}
	return true;
}
};

static void json_tokenize(const char* pSrcPath) {
	if (!pSrcPath) return;
	size_t srcSize = 0;
	char* pSrc = (char*)nxCore::raw_bin_load(pSrcPath, &srcSize);
	if (!pSrc) {
		nxCore::dbg_msg("json tokenize: unable to load \"%s\"\n", pSrcPath);
		return;
	}
	cxLexer lex;
	lex.set_text(pSrc, srcSize);
	TokEcho echo;
	lex.scan(echo);
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
		} else if (nxCore::str_ends_with(pSrcPath, ".json")) {
			json_tokenize(pSrcPath);
		}
	}

	nxApp::reset();
	//nxCore::mem_dbg();
	reset_sys();

	return 0;
}


