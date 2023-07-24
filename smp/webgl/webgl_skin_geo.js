let g_frame = 0;
let g_vertexBuffer = null;
let g_indexBuffer = null;
let g_gpuProg = null;
let g_mdlData = null;

const VS_NAME = "skin.vert";
const FS_NAME = "skin.frag";

// skeleton description
let g_numNodes = 0;
let g_nodeNames = null;
let g_parentNames = null;
let g_parentIdx = null;
let g_worldTranslates = null; // VEC[], rest-pose node positions in world space
let g_parentOffsets = null; // VEC[], rest-pose node positions in parent space

// animation work
// arrays of node positions and rotations to be animated
let g_animPos = null; // VEC[], local positions for all nodes
let g_animRot = null; // VEC[], local rotations (in degrees) for all nodes
// animation matrices
let g_localMatrices = null;
let g_worldMatrices = null;
let g_skinMatrices = null;

// skin
let g_skinNodeNames = null;
let g_skinNodeSkelIdx = null; // map from skin indices to skel

const FLOATS_PER_VERTEX = 3 + 3 + 4 + 4; // (pos + clr + jnt + wgt) per vertex

let g_anim = null;
let g_animFrame = 0;

let g_rotDY = 0.0;

const g_animChannels = ["tx", "ty", "tz", "rx", "ry", "rz"];

function animateSkelChannels(anim, frame) {
	if (!anim) return;
	const maxFrame = anim.frames - 1;
	if (frame < 0 || frame > maxFrame) return;
	frame = Math.floor(frame);
	let ntrk = anim.tracks;
	for (let itrk = 0; itrk < ntrk; ++itrk) {
		const name = anim.names[itrk];
		const ipathsep = name.lastIndexOf("/");
		const ichsep = name.lastIndexOf(":");
		const nodeName = name.substring(ipathsep + 1, ichsep);
		const chName = name.substring(ichsep + 1);
		const nodeId = g_nodeNames.indexOf(nodeName);
		if (nodeId >= 0) {
			const trkTop = itrk * anim.frames;
			const val = anim.samples[trkTop + frame];
			const valIdx = g_animChannels.indexOf(chName);
			switch (valIdx) {
				case 0: g_animPos[nodeId].x = val; break;
				case 1: g_animPos[nodeId].y = val; break;
				case 2: g_animPos[nodeId].z = val; break;
				case 3: g_animRot[nodeId].x = val; break;
				case 4: g_animRot[nodeId].y = val; break;
				case 5: g_animRot[nodeId].z = val; break;
				default: break;
			}
		}
	}
}

function updateLocalMatrices() {
	for (let i = 0; i < g_numNodes; ++i) {
		const r = g_animRot[i];
		let m = mdegxyz(r.x, r.y, r.z);
		m.xlatV(g_animPos[i]);
		g_localMatrices[i].copy(m);
	}
}

function updateWorldMatrices() {
	for (let i = 0; i < g_numNodes; ++i) {
		const iparent = g_parentIdx[i];
		if (iparent < 0) {
			g_worldMatrices[i].copy(g_localMatrices[i]);
		} else {
			g_worldMatrices[i].mul(g_worldMatrices[iparent], g_localMatrices[i]);
		}
	}
}

function updateSkinMatrices() {
	for (let i = 0; i < g_numNodes; ++i) {
		const invWPos = vscl(g_worldTranslates[i], -1.0);
		let invWMtx = munit();
		invWMtx.xlatV(invWPos); // rest-pose inverse world matrix
		g_skinMatrices[i].mul(g_worldMatrices[i], invWMtx);
	}
}

function loop() {
	const gl = scene.gl;

	gl.colorMask(true, true, true, true);
	gl.clearColor(0.33, 0.44, 0.55, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
	gl.depthMask(true);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	const cam = scene.cam;
	if (cam && g_mdlData && g_gpuProg) {
		cam.eye.set(1.1, 1.2, 4.0);
		cam.tgt.set(0.0, 0.9, 0.0);
		cam.update();

		let mdlMtx = mdegy(g_rotDY);
		g_rotDY -= 0.5;

		animateSkelChannels(g_anim, g_animFrame);
		updateLocalMatrices();
		updateWorldMatrices();
		updateSkinMatrices();
		++g_animFrame;
		if (g_animFrame >= g_anim.frames - 2) {
			g_animFrame = 0;
		}

		gl.useProgram(g_gpuProg);

		gl.bindBuffer(gl.ARRAY_BUFFER, g_vertexBuffer);
		let locPos = gl.getAttribLocation(g_gpuProg, "vtxPos");
		let locClr = gl.getAttribLocation(g_gpuProg, "vtxClr");
		let locJnt = gl.getAttribLocation(g_gpuProg, "vtxJnt");
		let locWgt = gl.getAttribLocation(g_gpuProg, "vtxWgt");
		let stride = FLOATS_PER_VERTEX * 4;
		let offs = setVtxAttr(locPos, 3, 0, stride);
		offs = setVtxAttr(locClr, 3, offs, stride);
		offs = setVtxAttr(locJnt, 4, offs, stride);
		offs = setVtxAttr(locWgt, 4, offs, stride);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, g_indexBuffer);

		let locWorld = gl.getUniformLocation(g_gpuProg, "prmWorld");
		if (locWorld) {
			setPrmMtx(locWorld, mdlMtx);
		} else {
			const numSkinNodes = g_mdlData.ncaptNodes;
			for (let i = 0; i < numSkinNodes; ++i) {
				let locSkinMtx = gl.getUniformLocation(g_gpuProg, `prmSkinMtx[${i}]`);
				const skelIdx = g_skinNodeSkelIdx[i];
				setPrmMtx(locSkinMtx, g_skinMatrices[skelIdx]);
			}
		}

		let locViewProj = gl.getUniformLocation(g_gpuProg, "prmViewProj");
		setPrmMtx(locViewProj, cam.viewProj);

		gl.drawElements(gl.TRIANGLES, g_mdlData.ntri*3, gl.UNSIGNED_SHORT, 0);
	}

	++g_frame;
	requestAnimationFrame(loop);
}


function createSkel(json) {
	let numNodes = json.npnt;
	if (!numNodes) return;
	let numStrAttrs = json.npntStrAttrs;
	if (numStrAttrs < 2) return;

	let iname = -1;
	let iparent = -1;
	for (let i = 0; i < numStrAttrs; ++i) {
		let attrName = json.pntStrAttrNames[i];
		if (attrName == "name") {
			iname = i;
		} else if (attrName == "parent") {
			iparent = i;
		}
	}
	if (iname < 0 || iparent < 0) return;

	g_nodeNames = new Array(numNodes);
	g_parentNames = new Array(numNodes);
	g_parentIdx = new Array(numNodes);
	const namesTopIdx = iname*numNodes;
	const parentsTopIdx = iparent*numNodes;
	for (let i = 0; i < numNodes; ++i) {
		g_nodeNames[i] = json.pntsStrData[namesTopIdx + i];
		g_parentNames[i] = json.pntsStrData[parentsTopIdx + i];
	}
	for (let i = 0; i < numNodes; ++i) {
		g_parentIdx[i] = g_nodeNames.indexOf(g_parentNames[i]);
	}

	g_worldTranslates = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		let pntIdx = i * 3;
		let wposX = json.pnts[pntIdx];
		let wposY = json.pnts[pntIdx + 1];
		let wposZ = json.pnts[pntIdx + 2];
		g_worldTranslates[i] = vset(wposX, wposY, wposZ);
	}

	g_parentOffsets = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_parentOffsets[i] = vcpy(g_worldTranslates[i]);
		let iparent = g_parentIdx[i];
		if (iparent >= 0) {
			g_parentOffsets[i].sub(g_worldTranslates[iparent]);
		}
	}

	g_animPos = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_animPos[i] = vcpy(g_parentOffsets[i]);
	}
	g_animRot = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_animRot[i] = vfill(0.0);
	}

	g_localMatrices = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_localMatrices[i] = munit();
	}
	g_worldMatrices = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_worldMatrices[i] = munit();
	}
	g_skinMatrices = new Array(numNodes);
	for (let i = 0; i < numNodes; ++i) {
		g_skinMatrices[i] = munit();
	}

	g_numNodes = numNodes;
}

function createModel(json) {
	const gl = scene.gl;
	if (!gl) return;
	let ntri = json.ntri;
	if (ntri < 1) return;
	const npnt = json.npnt;

	const numSkinNodes = json.ncaptNodes;
	const maxWeightsPerPoint = json.maxCaptsPerPnt;
	if (numSkinNodes && maxWeightsPerPoint && maxWeightsPerPoint <= 4) {
		g_skinNodeNames = new Array(numSkinNodes);
		g_skinNodeSkelIdx = new Array(numSkinNodes);
		for (let i = 0; i < numSkinNodes; ++i) {
			const captPath = json.captNodes[i];
			const captName = captPath.substring(0, captPath.indexOf("/"));
			g_skinNodeNames[i] = captName;
			const skinSkelIdx = g_nodeNames.indexOf(captName);
			g_skinNodeSkelIdx[i] = skinSkelIdx;
		}
	}

	let vbSize = npnt * FLOATS_PER_VERTEX;
	let clrIdx = -1;
	for (let i = 0; i < json.npntVecAttrs; ++i) {
		if (json.pntVecAttrNames[i] == "Cd") {
			clrIdx = i;
			break;
		}
	}
	const clrSrc = clrIdx*npnt*3;
	const vbData = new Float32Array(vbSize);
	for (let i = 0; i < npnt; ++i) {
		const srcIdx = i * 3;
		const vbIdx = i * FLOATS_PER_VERTEX;
		for (let j = 0; j < 3; ++j) {
			vbData[vbIdx + j] = json.pnts[srcIdx + j];
		}
		if (clrIdx < 0) {
			for (let j = 0; j < 3; ++j) {
				vbData[vbIdx + 3 + j] = 1.0;
			}
		} else {
			const pntClrSrc = clrSrc + i*3;
			for (let j = 0; j < 3; ++j) {
				vbData[vbIdx + 3 + j] = json.pntsVecData[pntClrSrc + j];
			}
		}

		const vtxJntOffs = 6;
		const vtxWgtOffs = vtxJntOffs + 4;
		for (let j = 0; j < 4; ++j) {
			vbData[vbIdx + vtxJntOffs + j] = 0.0;
			vbData[vbIdx + vtxWgtOffs + j] = 0.0;
		}
		const captSrcIdx = i * maxWeightsPerPoint;
		const numSrcCapts = Math.min(maxWeightsPerPoint, 4);
		for (let j = 0; j < numSrcCapts; ++j) {
			const jnt = json.pntsCaptNodes[captSrcIdx + j];
			if (jnt < 0) break;
			vbData[vbIdx + vtxJntOffs + j] = jnt;
			const wgt = Math.max(json.pntsCaptWeights[captSrcIdx + j], 0.0);
			vbData[vbIdx + vtxWgtOffs + j] = wgt;
		}
	}

	g_vertexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, g_vertexBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, vbData, gl.STATIC_DRAW);

	const nvtx = ntri * 3;
	const ibData = new Uint16Array(nvtx);
	for (let i = 0; i < nvtx; ++i) {
		ibData[i] = json.triIdx[i];
	}

	g_indexBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, g_indexBuffer);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, ibData, gl.STATIC_DRAW);

	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
}

function initGPU() {
	let vertSrc = scene.files[VS_NAME];
	let vs = compileVertShader(vertSrc);

	let fragSrc = scene.files[FS_NAME];
	let fs = compileFragShader(fragSrc);

	g_gpuProg = createGPUProgram(vs, fs);
}

function start() {
	scene.printFiles();

	initGPU();

	let skelSrc = scene.files["skin_skel.json"];
	if (skelSrc) {
		let skelJson = JSON.parse(skelSrc);
		if (skelJson.dataType == "geo") {
			createSkel(skelJson);
		}
	}

	let mdlSrc = scene.files["skin_model.json"];
	if (mdlSrc) {
		let mdlJson = JSON.parse(mdlSrc);
		if (mdlJson.dataType == "geo") {
			g_mdlData = mdlJson;
			createModel(mdlJson);
		}
	}

	let animSrc = scene.files["skin_anim.json"];
	if (animSrc) {
		let animJson = JSON.parse(animSrc);
		if (animJson.dataType == "clip") {
			g_anim = animJson;
		}
	}

	requestAnimationFrame(loop);
}


function main() {
	console.clear();

	scene.init();
	scene.load([
		VS_NAME,
		FS_NAME,
		"skin_model.json",
		"skin_skel.json",
		"skin_anim.json"
	], start);
}
