precision highp float;

attribute vec3 vtxPos;
attribute vec3 vtxNrm;

uniform mat4 prmViewProj;

varying vec3 pixPos;
varying vec3 pixNrm;

void main() {
	pixPos = vtxPos;
	pixNrm = vtxNrm;
	gl_Position = vec4(pixPos, 1.0) * prmViewProj;
}

