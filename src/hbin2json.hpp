void hbin_str_out(FILE* pOut, HBIN_STRING str);

void write_bgeo_json(HBIN_BGEO bgeo, FILE* pOut = nullptr);
void cvt_bgeo(const char* pBgeoPath, const char* pOutPath = nullptr);

void write_bclip_json(HBIN_BCLIP bclip, FILE* pOut = nullptr);
void cvt_bclip(const char* pBclipPath, const char* pOutPath = nullptr);
