// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2020 Sergey Chaban <sergey.chaban@gmail.com>

function degToRad(d) {
	return d * (Math.PI / 180.0);
}

function radToDeg(r) {
	return r * (180.0 / Math.PI);
}

function lerp(a, b, t) {
	return a + (b - a)*t;
}

class VEC {
	constructor() {
		this.e = new Float32Array(3);
	}

	get x()  { return this.e[0]; }
	set x(x) { this.e[0] = x; }
	get y()  { return this.e[1]; }
	set y(y) { this.e[1] = y; }
	get z()  { return this.e[2]; }
	set z(z) { this.e[2] = z; }

	set(x, y, z) {
		this.e[0] = x;
		this.e[1] = y;
		this.e[2] = z;
		return this;
	}

	copy(v) {
		for (let i = 0; i < 3; ++i) {
			this.e[i] = v.e[i];
		}
		return this;
	}

	fill(s) {
		this.e.fill(s);
		return this;
	}

	add(v1, v2) {
		const e1 = v2 ? v1.e : this.e;
		const e2 = v2 ? v2.e : v1.e;
		for (let i = 0; i < 3; ++i) {
			this.e[i] = e1[i] + e2[i];
		}
		return this;
	}

	sub(v1, v2) {
		const e1 = v2 ? v1.e : this.e;
		const e2 = v2 ? v2.e : v1.e;
		for (let i = 0; i < 3; ++i) {
			this.e[i] = e1[i] - e2[i];
		}
		return this;
	}

	mul(v1, v2) {
		const e1 = v2 ? v1.e : this.e;
		const e2 = v2 ? v2.e : v1.e;
		for (let i = 0; i < 3; ++i) {
			this.e[i] = e1[i] * e2[i];
		}
		return this;
	}

	scl(s) {
		for (let i = 0; i < 3; ++i) {
			this.e[i] *= s;
		}
		return this;
	}

	neg(v) {
		const e = v ? v.e : this.e;
		for (let i = 0; i < 3; ++i) {
			this.e[i] = -e[i];
		}
		return this;
	}

	dot(v1, v2) {
		const e1 = v2 ? (v1 ? v1.e : this.e) : this.e;
		const e2 = v2 ? v2.e : (v1 ? v1.e : this.e);
		let d = 0.0;
		for (let i = 0; i < 3; ++i) {
			d += e1[i] * e2[i];
		}
		return d;
	}

	cross(v1, v2) {
		const a = v2 ? v1 : this;
		const b = v2 ? v2 : v1;
		const x = a.y*b.z - a.z*b.y;
		const y = a.z*b.x - a.x*b.z;
		const z = a.x*b.y - a.y*b.x;
		this.set(x, y, z);
		return this;
	}

	get mag2() {
		return this.dot();
	}

	get mag() {
		let m = this.mag2;
		if (m > 0) {
			m = Math.sqrt(m);
		}
		return m;
	}

	normalize(v) {
		if (v) {
			this.copy(v);
		}
		const m = this.mag;
		if (m > 0) {
			this.scl(1.0 / m);
		}
		return this;
	}

	read(dat, offs) {
		for (let i = 0; i < 3; ++i) {
			this.e[i] = datF32(dat, offs + i*4);
		}
		return this;
	}

	print() {
		console.log(`<${this.x}, ${this.y}, ${this.z}>`);
	}
}

function vset(x, y, z)  { return (new VEC()).set(x, y, z); }
function vcpy(v)        { return (new VEC()).copy(v); }
function vfill(s)       { return (new VEC()).fill(s); }
function vadd(v1, v2)   { return (new VEC()).add(v1, v2); }
function vsub(v1, v2)   { return (new VEC()).sub(v1, v2); }
function vmul(v1, v2)   { return (new VEC()).mul(v1, v2); }
function vscl(v, s)     { return vcpy(v).scl(s); }
function vneg(v)        { return vcpy(v).neg(); }
function vdot(v1, v2)   { return v1.dot(v2); }
function vcross(v1, v2) { return (new VEC()).cross(v1, v2); }
function vnrm(v)        { return vcpy(v).normalize(); }
function vread(d, o)    { return (new VEC()).read(d, o); }


function dotRowCol(e1, e2, ir, ic) {
	let d = 0.0;
	for (let i = 0; i < 4; ++i) {
		d += e1[ir*4 + i] * e2[i*4 + ic];
	}
	return d;
}

class MTX {
	constructor() {
		this.e = new Float32Array(4 * 4);
	}

	set(...v) {
		this.e.fill(0.0);
		for (let i = 0; i < v.length; ++i) {
			this.e[i] = v[i];
		}
		return this;
	}

	copy(m) {
		for (let i = 0; i < 4*4; ++i) {
			this.e[i] = m.e[i];
		}
		return this;
	}

	identity() {
		this.e.fill(0.0);
		for (let i = 0; i < 4; ++i) {
			this.e[i*4 + i] = 1.0;
		}
		return this;
	}

	zero() {
		this.e.fill(0.0);
		return this;
	}

	mul(m1, m2) {
		const e1 = m2 ? m1.e : this.e;
		const e2 = m2 ? m2.e : m1.e;
		const e = new Float32Array(4 * 4);
		for (let i = 0; i < 4; ++i) {
			for (let j = 0; j < 4; ++j) {
				e[i*4 + j] = dotRowCol(e1, e2, i, j);
			}
		}
		for (let i = 0; i < 4*4; ++i) {
			this.e[i] = e[i];
		}
		return this;
	}

	invert() {
		const itmp = new Int32Array(4 * 3);
		const ipiv = 0;
		const icol = 4;
		const irow = 4 + 4;
		let ir = 0;
		let ic = 0;
		for (let i = 0; i < 4; ++i) {
			itmp[ipiv + i] = 0;
		}
		for (let i = 0; i < 4; ++i) {
			let amax = 0.0;
			for (let j = 0; j < 4; ++j) {
				if (itmp[ipiv + j] != 1) {
					let rj = j * 4;
					for (let k = 0; k < 4; ++k) {
						if (0 == itmp[ipiv + k]) {
							let a = this.e[rj + k];
							if (a < 0.0) a = -a;
							if (a >= amax) {
								amax = a;
								ir = j;
								ic = k;
							}
						}
					}
				}
			}
			++itmp[ipiv + ic];
			if (ir != ic) {
				let rr = ir * 4;
				let rc = ic * 4;
				for (let j = 0; j < 4; ++j) {
					let t = this.e[rr + j];
					this.e[rr + j] = this.e[rc + j];
					this.e[rc + j] = t;
				}
			}
			itmp[irow + i] = ir;
			itmp[icol + i] = ic;
			let rc = ic * 4;
			let piv = this.e[rc + ic];
			if (piv == 0.0) {
				return zero();
			}
			let rpiv = 1.0 / piv;
			this.e[rc + ic] = 1.0;
			for (let j = 0; j < 4; ++j) {
				this.e[rc + j] *= rpiv;
			}
			for (let j = 0; j < 4; ++j) {
				if (j != ic) {
					let rj = j * 4;
					let d = this.e[rj + ic];
					this.e[rj + ic] = 0.0;
					for (let k = 0; k < 4; ++k) {
						this.e[rj + k] -= this.e[rc + k] * d;
					}
				}
			}
		}
		for (let i = 4; --i >= 0;) {
			ir = itmp[irow + i];
			ic = itmp[icol + i];
			if (ir != ic) {
				for (let j = 0; j < 4; ++j) {
					let rj = j * 4;
					let t = this.e[rj + ir];
					this.e[rj + ir] = this.e[rj + ic];
					this.e[rj + ic] = t;
				}
			}
		}
		return this;
	}

	setRow(i, x, y, z, w) {
		const ri = i * 4;
		this.e[ri] = x;
		this.e[ri + 1] = y;
		this.e[ri + 2] = z;
		this.e[ri + 3] = w;
		return this;
	}

	setRowVec(i, v, w) {
		return this.setRow(i, v.x, v.y, v.z, w);
	}

	copyRow(i, m) {
		const ri = i * 4;
		return this.setRow(i, m.e[ri], m.e[ri + 1], m.e[ri + 2], m.e[ri + 3]);
	}

	setCol(i, x, y, z, w) {
		this.e[i] = x;
		this.e[i + 4] = y;
		this.e[i + 8] = z;
		this.e[i + 12] = w;
		return this;
	}

	setColVec(i, v, w) {
		return this.setCol(i, v.x, v.y, v.z, w);
	}

	copyCol(i, m) {
		return this.setCol(i, m.e[i], m.e[i + 4], m.e[i + 8], m.e[i + 12]);
	}


	transpose() {
		for (let i = 0; i < 3; ++i) {
			for (let j = i + 1; j < 4; ++j) {
				let ij = i*4 + j;
				let ji = j*4 + i;
				let t = this.e[ij];
				this.e[ij] = this.e[ji];
				this.e[ji] = t;
			}
		}
		return this;
	}

	radX(rx) {
		this.identity();
		const c = Math.cos(rx);
		const s = Math.sin(rx);
		this.setCol(1, 0.0, c, s, 0.0);
		this.setCol(2, 0.0, -s, c, 0.0);
		return this;
	}

	radY(ry) {
		this.identity();
		const c = Math.cos(ry);
		const s = Math.sin(ry);
		this.setCol(0, c, 0.0, -s, 0.0);
		this.setCol(2, s, 0.0, c, 0.0);
		return this;
	}

	radZ(rz) {
		this.identity();
		const c = Math.cos(rz);
		const s = Math.sin(rz);
		this.setCol(0, c, s, 0.0, 0.0);
		this.setCol(1, -s, c, 0.0, 0.0);
		return this;
	}

	degX(dx) {
		return this.radX(degToRad(dx));
	}

	degY(dy) {
		return this.radY(degToRad(dy));
	}

	degZ(dz) {
		return this.radZ(degToRad(dz));
	}

	xlat(x, y, z) {
		this.setCol(3, x, y, z, 1.0);
		return this;
	}

	xlatV(v) {
		this.xlat(v.x, v.y, v.z);
		return this;
	}

	xlatCpy(m) {
		this.copyCol(3, m);
		return this;
	}

	get translation() {
		return vset(this.e[3], this.e[3 + 4], this.e[3 + 8]);
	}

	read(dat, offs) {
		for (let i = 0; i < 4*4; ++i) {
			this.e[i] = datF32(dat, offs + i*4);
		}
		return this;
	}

	print() {
		for (let i = 0; i < 4; ++i) {
			const ri = i*4;
			console.log(this.e[ri], this.e[ri + 1], this.e[ri + 2], this.e[ri + 3]);
		}
	}
}

function mset(...v)   { return (new MTX()).set(...v); }
function mcpy(m)      { return (new MTX()).copy(m); }
function mmul(m1, m2) { return (new MTX()).mul(m1, m2); }
function munit()      { return (new MTX()).identity(); }
function mradx(rx)    { return (new MTX()).radX(rx); }
function mrady(ry)    { return (new MTX()).radY(ry); }
function mradz(rz)    { return (new MTX()).radZ(rz); }
function mdegx(dx)    { return (new MTX()).degX(dx); }
function mdegy(dy)    { return (new MTX()).degY(dy); }
function mdegz(dz)    { return (new MTX()).degZ(dz); }
function mread(d, o)  { return (new MTX()).read(d, o); }

function mdegxyz(dx, dy, dz) {
	let mx = mdegx(dx);
	let my = mdegy(dy);
	let mz = mdegz(dz);
	let m = mmul(mz, my);
	return mmul(m, mx);
}


function hex(x) {
	return x.toString(16);
}

function millis() {
	return performance.now();
}

function dbgmsg(msg = "") {
	const dbg = document.getElementById("dbgmsg");
	if (dbg) {
		dbg.innerHTML += msg + "<br>";
	} else {
		console.log(msg);
	}
}

function getFileName(fpath) {
	return fpath.substring(fpath.lastIndexOf("/")+1);
}

function dataReq(path, cb = null, txt = false) {
	let req = new XMLHttpRequest();
	req.overrideMimeType(txt ? "text/plain" : "application/octet-stream");
	req.responseType = txt ? "text" : "arraybuffer";
	let res = null;
	req.onreadystatechange = function() {
		if (req.readyState === 4 && req.status !== 404) {
			res = txt ? req.responseText : req.response;
			if (cb) {
				cb(res, path);
			}
		}
	};
	req.open('GET', path, cb != null);
	req.send(null);
	return res;
}


function compileShader(src, type) {
	let s = null;
	const gl = scene.gl;
	if (gl && src) {
		s = gl.createShader(type);
		if (s) {
			gl.shaderSource(s, src);
			gl.compileShader(s);
			if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) {
				console.log(gl.getShaderInfoLog(s));
				gl.deleteShader(s);
				s = null;
			}
		}
	}
	return s;
}

function compileVertShader(src) {
	const gl = scene.gl;
	return gl ? compileShader(src, gl.VERTEX_SHADER) : null;
}

function compileFragShader(src) {
	const gl = scene.gl;
	return compileShader(src, gl.FRAGMENT_SHADER);
}

function createGPUProgram(vs, fs) {
	const gl = scene.gl;
	let prog = null;
	if (gl && vs && fs) {
		prog = gl.createProgram();
		gl.attachShader(prog, vs);
		gl.attachShader(prog, fs);
		gl.linkProgram(prog);
		if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
			console.log(gl.getProgramInfoLog(prog));
			gl.deleteProgram(prog);
			prog = null;
		}
	}
	return prog;
}

function ckAttLoc(loc) {
	return (typeof loc === "number") && (loc >= 0);
}

function setVtxAttr(loc, nelems, offs, stride) {
	const gl = scene.gl;
	if (gl && ckAttLoc(loc)) {
		gl.enableVertexAttribArray(loc);
		gl.vertexAttribPointer(loc, nelems, gl.FLOAT, false, stride, offs);
	}
	return offs + nelems*4;
}

function setPrmMtx(loc, mtx) {
	const gl = scene.gl;
	if (gl && loc) {
		gl.uniformMatrix4fv(loc, false, mtx.e);
	}
}


class Camera {
	constructor(width, height) {
		this.width = width;
		this.height = height;
		this.aspect = width / height;
		this.eye = vset(0.0, 0.0, 0.0);
		this.tgt = vset(0.0, 0.0, -1.0);
		this.up = vset(0.0, 1.0, 0.0);
		this.zoom = 2.5;
		this.near = 0.01;
		this.far = 100.0;
		this.view = new MTX();
		this.proj = new MTX();
		this.viewProj = new MTX();
	}

	update() {
		const eye = this.eye;
		const tgt = this.tgt;
		const up = this.up;
		let vz = vsub(tgt, eye).normalize();
		let vx = vcross(up, vz).normalize();
		let vy = vcross(vx, vz).normalize();
		let vt = vset(vdot(eye, vx), vdot(eye, vy), vdot(eye, vz));
		vx.neg();
		vy.neg();
		vz.neg();
		this.view.identity();
		this.view.setRowVec(0, vx, 0.0);
		this.view.setRowVec(1, vy, 0.0);
		this.view.setRowVec(2, vz, 0.0);
		this.view.setColVec(3, vt, 1.0);

		let hfovy = Math.atan2(1.0, this.zoom * this.aspect);
		let c = 1.0 / Math.tan(hfovy);
		let q = this.far / (this.far - this.near);
		this.proj.setCol(0, c / this.aspect, 0.0, 0.0, 0.0);
		this.proj.setCol(1, 0.0, c, 0.0, 0.0);
		this.proj.setCol(2, 0.0, 0.0, -q, -1.0);
		this.proj.setCol(3, 0.0, 0.0, -q * this.near, 0.0);

		this.viewProj.mul(this.proj, this.view);
	}

	set(prog) {
		const gl = scene.gl;
		if (!gl) return;
		if (prog) {
			setPrmMtx(gl, prog.prmLocViewProj, this.viewProj);
		}
	}

}

class Scene {
	constructor() {
	}

	init(canvasId = "canvas") {
		const c = document.getElementById(canvasId);
		if (!c) {
			console.log("SCN: !canvas");
			return;
		}

		this.gl = null;
		try { this.gl = c.getContext("webgl"); } catch(e) {}
		if (!this.gl) {
			console.log("SCN: !webgl");
			return;
		}

		this.cam = new Camera(c.width, c.height);
	}

	clear() {
		this.files = null;
		this.vertShaders = {};
		this.fragShaders = {};
		this.progs = {};
		this.models = {};
		this.textures = {};
		this.anims = {};
	}

	initResources(files) {
		this.files = files;
	}

	load(flst, cb) {
		this.clear();
		if (!this.gl) {
			return;
		}
		const txtExts = ["vert", "frag", "json"];
		const files = {};
		for (const fpath of flst) {
			const fname = getFileName(fpath);
			files[fname] = null;
		}
			for (const fpath of flst) {
				let isTxt = false;
				for (const ext of txtExts) {
					if (fpath.endsWith("." + ext)) {
						isTxt = true;
						break;
					}
				}
				dataReq(fpath, (data, path) => { files[getFileName(path)] = data; }, isTxt);
			}
			let wait = setInterval(() => {
				let loadDone = true;
				for (const fpath of flst) {
					loadDone = !!files[getFileName(fpath)];
					if (!loadDone) break;
				}
				if (loadDone) {
					clearInterval(wait);
					this.initResources(files);
					cb();
				}
			}, 100);
	}

	printFiles() {
		if (!this.files) return;
		dbgmsg("Scene files:");
		for (const fname in this.files) {
			dbgmsg((typeof this.files[fname] === "string" ? "txt" : "bin") + ": " + fname);
		}
	}
}

const scene = new Scene();
