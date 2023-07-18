// Author: Sergey Chaban <sergey.chaban@gmail.com>

function degToRad(d) {
	return d * (Math.PI / 180.0);
}

function radToDeg(r) {
	return r * (180.0 / Math.PI);
}

function lerp(a, b, t) {
	return a + (b - a)*t;
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
