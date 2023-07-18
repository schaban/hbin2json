precision highp float;

varying vec3 pixPos;
varying vec3 pixClr;

void main() {
	gl_FragColor = vec4(pixClr, 1.0);
}

