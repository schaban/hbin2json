precision highp float;

varying vec3 pixPos;
varying vec3 pixNrm;

uniform sampler2D smpPano;
uniform vec3 prmEye;

uniform vec3 prmSHCtrl; // scale, spec
uniform vec3 prmCoefs[6*6];

vec2 panoUV(vec3 dir) {
	vec2 uv = vec2(0.0);
	float lxz = sqrt(dir.x*dir.x + dir.z*dir.z);
	if (lxz > 1.0e-5) uv.x = -dir.x / lxz;
	uv.y = dir.y;
	uv = clamp(uv, -1.0, 1.0);
	uv = acos(uv) / 3.141592653;
	uv.x *= 0.5;
	if (dir.z >= 0.0) uv.x = 1.0 - uv.x;
	return uv;
}

void shEval2(out float sh[2*2], vec3 v) {
	float x = v.x;
	float y = v.y;
	float z = v.z;
	float zz = z*z;
	float s0 = y, c0 = x;
	sh[0] = 0.282094791774;
	sh[2] = 0.488602511903 * z;
	float tmp = -0.488602511903;
	sh[1] = tmp * s0;
	sh[3] = tmp * c0;
}

void shEval3(out float sh[3*3], vec3 v) {
	float x = v.x;
	float y = v.y;
	float z = v.z;
	float zz = z*z;
	float tmp, prev0, prev1, prev2, s0 = y, s1, c0 = x, c1;
	sh[0] = 0.282094791774;
	sh[2] = 0.488602511903 * z;
	sh[6] = 0.946174695758 * zz + -0.315391565253;
	tmp = -0.488602511903;
	sh[1] = tmp * s0;
	sh[3] = tmp * c0;
	prev1 = -1.09254843059 * z;
	sh[5] = prev1 * s0;
	sh[7] = prev1 * c0;
	s1 = x*s0 + y*c0;
	c1 = x*c0 - y*s0;
	tmp = 0.546274215296;
	sh[4] = tmp * s1;
	sh[8] = tmp * c1;
}

void shEval6(out float sh[6*6], vec3 v) {
	float x = v.x;
	float y = v.y;
	float z = v.z;
	float zz = z*z;
	float tmp, prev0, prev1, prev2, s0 = y, s1, c0 = x, c1;
	sh[0] = 0.282094791774;
	sh[2] = 0.488602511903 * z;
	sh[6] = 0.946174695758 * zz + -0.315391565253;
	sh[12] =  ( 1.86588166295 * zz + -1.11952899777 )  * z;
	sh[20] = 1.9843134833 * z * sh[12] + -1.00623058987 * sh[6];
	sh[30] = 1.98997487421 * z * sh[20] + -1.00285307284 * sh[12];
	tmp = -0.488602511903;
	sh[1] = tmp * s0;
	sh[3] = tmp * c0;
	prev1 = -1.09254843059 * z;
	sh[5] = prev1 * s0;
	sh[7] = prev1 * c0;
	prev2 = -2.28522899732 * zz + 0.457045799464;
	sh[11] = prev2 * s0;
	sh[13] = prev2 * c0;
	tmp = -4.6833258049 * zz + 2.00713963067;
	prev0 = tmp * z;
	sh[19] = prev0 * s0;
	sh[21] = prev0 * c0;
	tmp = 2.03100960116 * z;
	prev1 = tmp * prev0;
	tmp = -0.991031208965 * prev2;
	prev1 += tmp;
	sh[29] = prev1 * s0;
	sh[31] = prev1 * c0;
	s1 = x*s0 + y*c0;
	c1 = x*c0 - y*s0;
	tmp = 0.546274215296;
	sh[4] = tmp * s1;
	sh[8] = tmp * c1;
	prev1 = 1.44530572132 * z;
	sh[10] = prev1 * s1;
	sh[14] = prev1 * c1;
	prev2 = 3.31161143515 * zz + -0.473087347879;
	sh[18] = prev2 * s1;
	sh[22] = prev2 * c1;
	tmp = 7.19030517746 * zz + -2.39676839249;
	prev0 = tmp * z;
	sh[28] = prev0 * s1;
	sh[32] = prev0 * c1;
	s0 = x*s1 + y*c1;
	c0 = x*c1 - y*s1;
	tmp = -0.590043589927;
	sh[9] = tmp * s0;
	sh[15] = tmp * c0;
	prev1 = -1.77013076978 * z;
	sh[17] = prev1 * s0;
	sh[23] = prev1 * c0;
	prev2 = -4.40314469492 * zz + 0.489238299435;
	sh[27] = prev2 * s0;
	sh[33] = prev2 * c0;
	s1 = x*s0 + y*c0;
	c1 = x*c0 - y*s0;
	tmp = 0.625835735449;
	sh[16] = tmp * s1;
	sh[24] = tmp * c1;
	prev1 = 2.07566231488 * z;
	sh[26] = prev1 * s1;
	sh[34] = prev1 * c1;
	s0 = x*s1 + y*c1;
	c0 = x*c1 - y*s1;
	tmp = -0.65638205684;
	sh[25] = tmp * s0;
	sh[35] = tmp * c0;
}

float fresnel(vec3 vdir, vec3 n) {
	float tcos = dot(vdir, n);
	float f = pow(1.0 - tcos, 5.0)*0.02 + 0.01;
	return 1.0 - clamp(f, 0.0, 1.0);
}

vec3 calcPANO(vec3 n) {
	return texture2D(smpPano, panoUV(n)).rgb;
}

vec3 calcREFL(vec3 n) {
	vec3 vdir = normalize(pixPos - prmEye);
	vec3 rv = reflect(vdir, n);
	float f = fresnel(vdir, n);
	return texture2D(smpPano, panoUV(rv)).rgb * f;
}

vec3 calcSH2DIFF(vec3 n) {
	float cdir[2*2];
	shEval2(cdir, n);
	float scl = prmSHCtrl.x;
	float w0 = scl;
	float w1 = scl / 1.5;
	vec3 c0 = prmCoefs[0] * cdir[0] * w0;
	vec3 c1_1 = prmCoefs[1] * cdir[1] * w1;
	vec3 c10 = prmCoefs[2] * cdir[2] * w1;
	vec3 c11 = prmCoefs[3] * cdir[3] * w1;
	return c0 + c1_1 + c10 + c11;
}

vec3 calcSH2REFL(vec3 n) {
	float cdir[2*2];
	vec3 vdir = normalize(pixPos - prmEye);
	vec3 rv = reflect(vdir, n);
	shEval2(cdir, rv);
	float scl = prmSHCtrl.x;
	float spec = prmSHCtrl.y;
	float w0 = scl;
	float w1 = exp(-1.0 / (2.0*spec)) * scl;
	vec3 c0 = prmCoefs[0] * cdir[0] * w0;
	vec3 c1_1 = prmCoefs[1] * cdir[1] * w1;
	vec3 c10 = prmCoefs[2] * cdir[2] * w1;
	vec3 c11 = prmCoefs[3] * cdir[3] * w1;
	float f = fresnel(vdir, n);
	return (c0 + c1_1 + c10 + c11) * f;
}

vec3 calcSH3DIFF(vec3 n) {
	float cdir[3*3];
	shEval3(cdir, n);
	float scl = prmSHCtrl.x;
	float w0 = scl;
	float w1 = scl / 1.5;
	float w2 = scl / 4.0;
	vec3 c0   = prmCoefs[0] * cdir[0] * w0;
	vec3 c1_1 = prmCoefs[1] * cdir[1] * w1;
	vec3 c10  = prmCoefs[2] * cdir[2] * w1;
	vec3 c11  = prmCoefs[3] * cdir[3] * w1;
	vec3 c2_2 = prmCoefs[4] * cdir[4] * w2;
	vec3 c2_1 = prmCoefs[5] * cdir[5] * w2;
	vec3 c20  = prmCoefs[6] * cdir[6] * w2;
	vec3 c21  = prmCoefs[7] * cdir[7] * w2;
	vec3 c22  = prmCoefs[8] * cdir[8] * w2;
	return c0 + c1_1 + c10 + c11 + c2_2 + c2_1 + c20 + c21 + c22;
}

vec3 calcSH3REFL(vec3 n) {
	float cdir[3*3];
	vec3 vdir = normalize(pixPos - prmEye);
	vec3 rv = reflect(vdir, n);
	shEval3(cdir, rv);
	float scl = prmSHCtrl.x;
	float spec = prmSHCtrl.y;
	float w0 = scl;
	float w1 = exp(-1.0 / (2.0*spec)) * scl;
	float w2 = exp(-4.0 / (2.0*spec)) * scl;
	vec3 c0   = prmCoefs[0] * cdir[0] * w0;
	vec3 c1_1 = prmCoefs[1] * cdir[1] * w1;
	vec3 c10  = prmCoefs[2] * cdir[2] * w1;
	vec3 c11  = prmCoefs[3] * cdir[3] * w1;
	vec3 c2_2 = prmCoefs[4] * cdir[4] * w2;
	vec3 c2_1 = prmCoefs[5] * cdir[5] * w2;
	vec3 c20  = prmCoefs[6] * cdir[6] * w2;
	vec3 c21  = prmCoefs[7] * cdir[7] * w2;
	vec3 c22  = prmCoefs[8] * cdir[8] * w2;
	float f = fresnel(vdir, n);
	return (c0 + c1_1 + c10 + c11 + c2_2 + c2_1 + c20 + c21 + c22) * f;
}

vec3 calcSH6sub(vec3 n, bool refl) {
	float cdir[6*6];
	vec3 vdir = normalize(pixPos - prmEye);
	if (refl) {
		vec3 rv = reflect(vdir, n);
		shEval6(cdir, rv);
	} else {
		shEval6(cdir, n);
	}
	float wgt[6];
	float scl = prmSHCtrl.x;
	float spec = prmSHCtrl.y;
	for (int i = 0; i < 6; ++i) {
		wgt[i] = exp(float(-i*i) / (2.0*spec)) * scl;
	}
	for (int l = 0; l < 6; ++l) {
		int i0 = l * l;
		int i1 = (l + 1) * (l + 1);
		float w = wgt[l];
		for (int i = 0; i < 6*6; ++i) {
			if (i >= i0 && i < i1) {
				cdir[i] *= w;
			}
		}
	}
	vec3 c = vec3(0.0);
	for (int i = 0; i < 6*6; ++i) {
		c += prmCoefs[i] * cdir[i];
	}
	if (refl) {
		float f = fresnel(vdir, n);
		c *= f;
	}
	return c;
}

vec3 calcSH6PANO(vec3 n) {
	return calcSH6sub(n, false);
}

vec3 calcSH6REFL(vec3 n) {
	return calcSH6sub(n, true);
}


void main() {
	vec3 n = normalize(pixNrm);
	vec3 c = calcSH3DIFF(n);
	gl_FragColor = vec4(c, 1.0);
}

