let g_frame = 0;
let g_vertexBuffer = null;
let g_indexBuffer = null;
let g_gpuProg = null;
let g_mdlData = null;

let g_y_degrees = 0.0;

function loop() {
	const gl = scene.gl;

	gl.colorMask(true, true, true, true);
	gl.clearColor(0.11, 0.22, 0.27, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
	gl.depthMask(true);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);

	const cam = scene.cam;
	if (cam && g_mdlData && g_gpuProg) {
		cam.eye.set(0.0, 0.2, 1.0);
		cam.tgt.set(0.0, 0.25, 0.0);
		cam.update();

		let mdlMtx = mdegy(g_y_degrees);
		g_y_degrees -= 0.5;

		gl.useProgram(g_gpuProg);

		gl.bindBuffer(gl.ARRAY_BUFFER, g_vertexBuffer);
		let locPos = gl.getAttribLocation(g_gpuProg, "vtxPos");
		let locClr = gl.getAttribLocation(g_gpuProg, "vtxClr");
		let stride = 6*4;
		let offs = setVtxAttr(locPos, 3, 0, stride);
		setVtxAttr(locClr, 3, offs, stride);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, g_indexBuffer);

		let locWorld = gl.getUniformLocation(g_gpuProg, "prmWorld");
		let locViewProj = gl.getUniformLocation(g_gpuProg, "prmViewProj");
		setPrmMtx(locWorld, mdlMtx);
		setPrmMtx(locViewProj, cam.viewProj);

		gl.drawElements(gl.TRIANGLES, g_mdlData.ntri*3, gl.UNSIGNED_SHORT, 0);
	}

	++g_frame;
	requestAnimationFrame(loop);
}


function createModel(json) {
	const gl = scene.gl;
	if (!gl) return;
	let ntri = json.ntri;
	if (ntri < 1) return;
	const npnt = json.npnt;
	const floatsPerVertex = 3 * 2; // (pos + color) per vertex
	let vbSize = npnt * floatsPerVertex;
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
		const vbIdx = i * floatsPerVertex;
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
	let vertSrc = scene.files["basic.vert"];
	let vs = compileVertShader(vertSrc);

	let fragSrc = scene.files["basic.frag"];
	let fs = compileFragShader(fragSrc);

	g_gpuProg = createGPUProgram(vs, fs);
}

function start() {
	scene.printFiles();

	initGPU();

	let mdlSrc = scene.files["basic.json"];
	if (mdlSrc) {
		let mdlJson = JSON.parse(mdlSrc);
		if (mdlJson.dataType == "geo") {
			g_mdlData = mdlJson;
			createModel(mdlJson);
		}
	}

	requestAnimationFrame(loop);
}


function main() {
	console.clear();

	scene.init();
	scene.load([
		"basic.vert",
		"basic.frag",
		"basic.json"
	], start);
}
