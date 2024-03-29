/*
 * dump geometry data generated by hbin2json in Houdini hclassic format
 * this sample only handles base geometry - points and polygons - extra attributes are ignored
 */

const fs = require("fs");

function json_to_hgeo(json) {
	console.log("PGEOMETRY V5");
	let numPoints = json.npnt;
	let numPolygons = json.npol;
	let numTriangles = json.ntri;
	console.log("NPoints " + numPoints + " NPrims " + numPolygons);
	console.log("NPointGroups " + 0 + " NPrimGroups " + 0);
	console.log("NPointAttrib " + 0 + " NVertexAttrib " + 0 + " NPrimAttrib " + 0 + " NAttrib " + 0);

	/*
		points are stored in json.pnts array like this:
		[x0, y0, z0, x1, y1, z1, ...]
	 */
	for (let i = 0; i < numPoints; ++i) {
		let idx = i * 3;
		console.log(json.pnts[idx] + " " + json.pnts[idx + 1] + " " + json.pnts[idx + 2] + " 1");
	}

	if (numPolygons > 0) {
		console.log("Run " + numPolygons + " Poly");
		if (numPolygons == numTriangles) {
			/*
				triangulated geo
				to save space all triangles are stores in json.triIdx[] 
				json.pols[] and json.polIdx[] are empty
				[tri0[v0], tri0[v1], tri0[v2], tri1[v0], tri1[v1], tri1[v2], ...]
			*/
			for (let i = 0; i < numTriangles; ++i) {
				let triOrg = i * 3;
				let pntListStr = ""
				for (let j = 0; j < 3; ++j) {
					pntListStr += " " + json.triIdx[triOrg + j];
				}
				console.log(" 3 <" + pntListStr);
			}
		} else {
			/*
				geo consists of arbitrary polygons
				polygons are stored as pairs of numbers in json.pols[]:
				[start0, nverts0, start1,  nverts1, ...]
				start* is where the list of polygon's vertex indices begins in json.polIdx[]
			*/
			for (let i = 0; i < numPolygons; ++i) {
				let idx = i * 2;
				let polStart = json.pols[idx]; // start index in json.polIdx[]
				let vertsPerPol = json.pols[idx + 1];
				let pntListStr = ""
				for (let i = 0; i < vertsPerPol; ++i) {
					pntListStr += " " + json.polIdx[polStart + i];
				}
				console.log(" " + vertsPerPol + " <" + pntListStr);
			}
		}
	}

	console.log("beginExtra");
	console.log("endExtra");
}

function main(args) {
	if (args.length < 1) {
		console.log("json_info <path>");
		return;
	}
	let inPath = args[0];
	let inData = fs.readFileSync(inPath);
	let jsonData = JSON.parse(inData);
	if (jsonData.dataType === undefined) {
		console.log("data type is unknown");
		return;
	}
	if (jsonData.dataType == "geo") {
		json_to_hgeo(jsonData);
	} else {
		console.log("data type is not geo");
		return;
	}
}

main(process.argv.slice(2));
