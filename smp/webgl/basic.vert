precision highp float;

attribute vec3 vtxPos;
attribute vec3 vtxClr;

uniform mat4 prmWorld;
uniform mat4 prmViewProj;

varying vec3 pixPos;
varying vec3 pixClr;

void main() {
	pixPos = (vec4(vtxPos, 1.0) * prmWorld).xyz;
	pixClr = vtxClr;
	gl_Position = vec4(pixPos, 1.0) * prmViewProj;
}

