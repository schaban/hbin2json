precision highp float;

varying vec3 pixPos;
varying vec3 pixClr;

void main() {
	vec3 c = pixClr;
	c = max(c, 0.0);
	c = sqrt(c);
	gl_FragColor = vec4(c, 1.0);
}

