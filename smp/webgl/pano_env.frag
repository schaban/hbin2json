precision highp float;

varying vec3 pixPos;
varying vec3 pixNrm;

uniform sampler2D smpPano;

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

void main() {
	vec3 n = normalize(pixNrm);
	vec3 c = texture2D(smpPano, panoUV(n)).rgb;
	gl_FragColor = vec4(c, 1.0);
}

