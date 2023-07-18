// Author: Sergey Chaban <sergey.chaban@gmail.com>

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
		xlat(v.x, v.y, v.z);
		return this;
	}

	xlatCpy(m) {
		this.copyCol(3, m);
		return this;
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

