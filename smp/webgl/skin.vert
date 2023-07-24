precision highp float;

attribute vec3 vtxPos;
attribute vec3 vtxClr;
attribute vec4 vtxJnt;
attribute vec4 vtxWgt;

uniform mat4 prmSkinMtx[40];
uniform mat4 prmViewProj;

varying vec3 pixPos;
varying vec3 pixClr;

void main() {
	mat4 wm = prmSkinMtx[int(vtxJnt.x)] * vtxWgt.x;
	wm     += prmSkinMtx[int(vtxJnt.y)] * vtxWgt.y;
	wm     += prmSkinMtx[int(vtxJnt.z)] * vtxWgt.z;
	wm     += prmSkinMtx[int(vtxJnt.w)] * vtxWgt.w;
	pixPos = (vec4(vtxPos, 1.0) * wm).xyz;
	pixClr = vtxClr;
	gl_Position = vec4(pixPos, 1.0) * prmViewProj;
}

