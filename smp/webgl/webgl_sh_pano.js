const SH_ORDER = 6; // 2 | 3 | 6
const SH_NCOEFS = SH_ORDER * SH_ORDER;

const FLOATS_PER_VERTEX = 3 * 2; // (pos + nrm) per vertex

let g_frame = 0;

let g_gpuEnvProg = null;
let g_gpuObjProg = null;

let g_panoW = 0;
let g_panoH = 0;
let g_panoData = null;
let g_panoTex = null;
let g_panoCoefsR = null;
let g_panoCoefsG = null;
let g_panoCoefsB = null;

let g_shScale = 1.5;
let g_shSpec = 16.0;

let g_objMdl = null;
let g_envMdl = null;

function cvtTexVal(c) {
	c = Math.max(c, 0.0);
	c = Math.min(c, 1.0)
	c *= 255.0;
	c &= 0xFF;
	return c;
}

function initPanoramaCoefs() {
	const w = g_panoW;
	const h = g_panoH;
	const pano = g_panoData;
	let coefsR = new Float32Array(SH_NCOEFS);
	let coefsG = new Float32Array(SH_NCOEFS);
	let coefsB = new Float32Array(SH_NCOEFS);
	for (let i = 0; i < SH_NCOEFS; ++i) {
		coefsR[i] = 0.0;
		coefsG[i] = 0.0;
		coefsB[i] = 0.0;
	}
	const da = (2.0*Math.PI / w) * (Math.PI / h);
	const iw = 1.0 / w;
	const ih = 1.0 / h;
	let sum = 0.0;
	let coefsDir = new Float32Array(SH_NCOEFS);
	for (let y = 0; y < h; ++y) {
		const v = 1.0 - (y + 0.5)*ih;
		const dw = da * Math.sin(Math.PI * v);
		const ay = (v - 1.0) * Math.PI;
		const sy = Math.sin(ay);
		const cy = Math.cos(ay);
		const ax0 = iw * Math.PI;
		let rsx = Math.sin(ax0);
		let rcx = Math.cos(ax0);
		const rax = 2.0 * rsx*rsx;
		const rbx = Math.sin(ax0 * 2.0);
		for (let x = 0; x < w; ++x) {
			const sx = rsx;
			const cx = rcx;
			const dx = cx * sy;
			const dy = cy;
			const dz = sx * sy;
			const isx = rsx - (rax*rsx - rbx*rcx);
			const icx = rcx - (rax*rcx + rbx*rsx);
			rsx = isx;
			rcx = icx;
			if (SH_ORDER == 2) {
				shEval2(coefsDir, dx, dy, dz);
			} else if (SH_ORDER == 6) {
				shEval6(coefsDir, dx, dy, dz);
			} else {
				shEval3(coefsDir, dx, dy, dz);
			}
			for (let i = 0; i < SH_NCOEFS; ++i) {
				coefsDir[i] *= dw;
			}
			const cidx = (y*w + x) * 3;
			for (let i = 0; i < SH_NCOEFS; ++i) {
				coefsR[i] += coefsDir[i] * pano[cidx];
			}
			for (let i = 0; i < SH_NCOEFS; ++i) {
				coefsG[i] += coefsDir[i] * pano[cidx + 1];
			}
			for (let i = 0; i < SH_NCOEFS; ++i) {
				coefsB[i] += coefsDir[i] * pano[cidx + 2];
			}
			sum += dw;
		}
	}
	let scl = 0.0;
	if (sum) {
		scl = (Math.PI * 4.0) / sum;
	}
	for (let i = 0; i < SH_NCOEFS; ++i) {
		coefsR[i] *= scl;
		coefsG[i] *= scl;
		coefsB[i] *= scl;
	}
	g_panoCoefsR = coefsR;
	g_panoCoefsG = coefsG;
	g_panoCoefsB = coefsB;

/*
	let str = "R: ";
	for (let i = 0; i < SH_NCOEFS; ++i) {
		str += coefsR[i] + " ";
	}
	console.log(str);
*/
}

function initPanorama(json) {
	let w = json.frames;
	let h = json.tracks / 3;
	let panoData = new Float32Array(w * h * 3);
	let didx = 0;
	for (let y = 0; y < h; ++y) {
		let sidx = (h - 1 - y) * 3 * w;
		for (let x = 0; x < w; ++x) {
			let r = json.samples[sidx + x];
			let g = json.samples[sidx + w + x];
			let b = json.samples[sidx + w*2 + x];
			panoData[didx] = r;
			panoData[didx + 1] = g;
			panoData[didx + 2] = b;
			didx += 3;
		}
	}
	g_panoW = w;
	g_panoH = h;
	g_panoData = panoData;
	dbgmsg("panorama " + w + "x" + h);
	const coefsT0 = millis();
	initPanoramaCoefs();
	const coefsT1 = millis();
	const coefsDT = coefsT1 - coefsT0;
	dbgmsg("order " + SH_ORDER + " SH extraction time: " + coefsDT + " millis");

	const gl = scene.gl;
	let htex = gl.createTexture();
	let texData = new Uint8Array(w * h * 4);
	for (let i = 0; i < w*h; ++i) {
		let idx = i*3;
		let r = cvtTexVal(g_panoData[idx]);
		let g = cvtTexVal(g_panoData[idx + 1]);
		let b = cvtTexVal(g_panoData[idx + 2]);
		idx = i*4;
		texData[idx] = r;
		texData[idx + 1] = g;
		texData[idx + 2] = b;
		texData[idx + 3] = 0xFF;
	}
	gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
	gl.bindTexture(gl.TEXTURE_2D, htex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, texData);
	gl.generateMipmap(gl.TEXTURE_2D);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	gl.bindTexture(gl.TEXTURE_2D, null);

	g_panoTex = htex;
}

class Model {
	constructor(name) {
		const gl = scene.gl;
		if (!gl) return;
		const jsonSrc = scene.files[name + ".json"];
		if (!jsonSrc) return;
		const json = JSON.parse(jsonSrc);
		let ntri = json.ntri;
		if (ntri < 1) return;
		this.numTris = ntri;
		const npnt = json.npnt;
		let vbSize = npnt * FLOATS_PER_VERTEX;
		let nrmIdx = -1;
		for (let i = 0; i < json.npntVecAttrs; ++i) {
			if (json.pntVecAttrNames[i] == "N") {
				nrmIdx = i;
				break;
			}
		}
		const nrmSrc = nrmIdx*npnt*3;
		const vbData = new Float32Array(vbSize);
		for (let i = 0; i < npnt; ++i) {
			const srcIdx = i * 3;
			const vbIdx = i * FLOATS_PER_VERTEX;
			for (let j = 0; j < 3; ++j) {
				vbData[vbIdx + j] = json.pnts[srcIdx + j];
			}
			if (nrmIdx < 0) {
				for (let j = 0; j < 3; ++j) {
					vbData[vbIdx + 3 + j] = 0.0;
				}
			} else {
				const pntNrmSrc = nrmSrc + i*3;
				for (let j = 0; j < 3; ++j) {
					vbData[vbIdx + 3 + j] = json.pntsVecData[pntNrmSrc + j];
				}
			}
		}

		this.vertexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, vbData, gl.STATIC_DRAW);

		const nvtx = ntri * 3;
		const ibData = new Uint16Array(nvtx);
		for (let i = 0; i < nvtx; ++i) {
			ibData[i] = json.triIdx[i];
		}

		this.indexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, ibData, gl.STATIC_DRAW);

		gl.bindBuffer(gl.ARRAY_BUFFER, null);
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);
	}

	draw(prog) {
		const gl = scene.gl;
		const cam = scene.cam;
		if (!gl) return;
		if (!prog) return;

		gl.useProgram(prog);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.vertexBuffer);
		let locPos = gl.getAttribLocation(prog, "vtxPos");
		let locNrm = gl.getAttribLocation(prog, "vtxNrm");
		let stride = FLOATS_PER_VERTEX * 4;
		let offs = setVtxAttr(locPos, 3, 0, stride);
		setVtxAttr(locNrm, 3, offs, stride);

		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);

		let locViewProj = gl.getUniformLocation(prog, "prmViewProj");
		setPrmMtx(locViewProj, cam.viewProj);

		let locEye = gl.getUniformLocation(prog, "prmEye");
		setPrmVec(locEye, cam.eye);

		let locSmpPano = gl.getUniformLocation(prog, "smpPano");
		if (locSmpPano) {
			gl.activeTexture(gl.TEXTURE0);
			gl.bindTexture(gl.TEXTURE_2D, g_panoTex);
			gl.uniform1i(locSmpPano, 0);
		}

		let locSHCtrl = gl.getUniformLocation(prog, "prmSHCtrl");
		setPrmVec(locSHCtrl, vset(g_shScale, g_shSpec, 0.0));

		if (gl.getUniformLocation(prog, "prmCoefs")) {
			let shv = vfill(0.0);
			for (let i = 0; i < 6*6; ++i) {
				if (i < SH_NCOEFS) {
					shv.set(g_panoCoefsR[i], g_panoCoefsG[i], g_panoCoefsB[i]);
				} else {
					shv.fill(0.0);
				}
				let locCoef = gl.getUniformLocation(prog, `prmCoefs[${i}]`);
				setPrmVec(locCoef, shv);
			}
		}

		gl.drawElements(gl.TRIANGLES, this.numTris*3, gl.UNSIGNED_SHORT, 0);
	}
}

function initGPU() {
	let vertSrc = scene.files["pano_env.vert"];
	let vs = compileVertShader(vertSrc);
	let fragSrc = scene.files["pano_env.frag"];
	let fs = compileFragShader(fragSrc);
	g_gpuEnvProg = createGPUProgram(vs, fs);

	vertSrc = scene.files["pano_obj.vert"];
	vs = compileVertShader(vertSrc);
	fragSrc = scene.files["pano_obj.frag"];
	fs = compileFragShader(fragSrc);
	g_gpuObjProg = createGPUProgram(vs, fs);
}

let g_orbitY = -3.0;

function loop() {
	const gl = scene.gl;

	gl.colorMask(true, true, true, true);
	gl.clearColor(0.33, 0.32, 0.29, 1.0);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
	gl.depthMask(true);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	gl.enable(gl.CULL_FACE);
	gl.frontFace(gl.CW);
	gl.cullFace(gl.BACK);

	const cam = scene.cam;
	if (cam) {
		if (scene.ckKeyNow("ArrowLeft")) {
			g_orbitY -= 1.0;
		}
		if (scene.ckKeyNow("ArrowRight")) {
			g_orbitY += 1.0;
		}
		let my = mdegy(g_orbitY);
		let tgt = vset(0.0, 0.25, 0.0);
		let rel = my.calcPnt(vset(0.0, 0.0, 3.0));
		let eye = vadd(tgt, rel);
		cam.eye.copy(eye);
		cam.tgt.copy(tgt);
		cam.update();

		g_objMdl.draw(g_gpuObjProg);
		g_envMdl.draw(g_gpuEnvProg);
	}

	++g_frame;
	requestAnimationFrame(loop);
}

function start() {
	scene.printFiles();

	scene.initKeys(["ArrowLeft", "ArrowRight"]);
	initGPU();
	initPanorama(JSON.parse(scene.files["pano.json"]));
	g_objMdl = new Model("obj_sphere");
	g_envMdl = new Model("env_sphere");

	requestAnimationFrame(loop);
}


function main() {
	console.clear();

	scene.init();
	scene.load([
		"pano.json",
		"obj_sphere.json",
		"env_sphere.json",
		"pano_env.vert",
		"pano_env.frag",
		"pano_obj.vert",
		"pano_obj.frag"
	], start);
}
