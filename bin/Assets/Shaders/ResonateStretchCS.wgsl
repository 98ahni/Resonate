//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2025 98ahni> Original file author

@group(0) @binding(0) var<storage, read_write> audioBuffer: array<f32>;
@group(0) @binding(1) var<storage, read_write> stretchedBuffer: array<f32>;
override audioLength: u32;
override stretchedLength: u32;
override stftBins: u32;
override stftHop: f32;
override stretchFactor: f32;

fn windowHann(length: u32, index: u32) -> f32
{
	return 0.5f * (1 - cos(radians(360.0) * f32(index) / f32(length - 1)));
}
fn windowSquare(length: u32, index: u32) -> f32
{
	return 1.0f;
}
fn windowBartlett(length: u32, index: u32) -> f32
{
  return 2 / f32(length - 1) * (f32(length - 1) / 2 - abs(f32(index) - f32(length - 1) / 2));
}
fn windowBartlettHann(length: u32, index: u32) -> f32
{
  return 0.62 - f32(0.48f * abs(f32(index) / f32(length - 1) - 0.5)) - f32(0.38f * cos(radians(360.0) * f32(index) / f32(length - 1)));
}
fn windowBlackman(length: u32, index: u32) -> f32
{
  var a0 = (1 - /*alpha*/0.16) / 2;
  var a1 = 0.5;
  var a2 = /*alpha*/0.16 / 2;

  return a0 - a1 * cos(radians(360.0) * f32(index) / f32(length - 1)) + a2 * cos(4 * radians(180.0) * f32(index) / f32(length - 1));
}
fn windowCosine(length: u32, index: u32) -> f32
{
  return cos(radians(180.0) * f32(index) / f32(length - 1) - radians(180.0) / 2);
}
fn windowGauss(length: u32, index: u32) -> f32
{
  return pow(/*E*/2.718281828459f, -0.5f * pow(f32((f32(index) - f32(length - 1) / 2) / (/*alpha*/0.25f * f32(length - 1) / 2)), 2.0f));
}
fn windowHamming(length: u32, index: u32) -> f32
{
  return 0.54 - f32(0.46 * cos(radians(360.0) * f32(index) / f32(length - 1)));
}
fn windowLanczos(length: u32, index: u32) -> f32
{
  var x = 2 * f32(index) / f32(length - 1) - 1;
  return sin(radians(180.0) * x) / (radians(180.0) * x);
}
fn windowTriangular(length: u32, index: u32) -> f32
{
  return 2 / f32(length) * (f32(length) / 2 - abs(f32(index) - f32(length - 1) / 2));
}

@compute @workgroup_size(256)
fn stretchAudio(@builtin(global_invocation_id) id: vec3<u32>) 
{
	let hop = f32(stftBins) * stftHop;
	let hopSynth = hop * stretchFactor;
	let inputStart = u32(hop) * id.x;
	let overlapStart = u32(hopSynth) * id.x;
	for(var sampleInd: u32 = 0; sampleInd < stftBins; sampleInd++)
	{
		//stretchedBuffer[sampleInd] = 0.6;
		stretchedBuffer[overlapStart + sampleInd] += audioBuffer[sampleInd + inputStart] *
			windowHann
		//	windowSquare
		//	windowBartlett
		//	windowBartlettHann		// **
		//	windowBlackman
		//	windowCosine
		//	windowGauss
		//	windowHamming			// **
		//	windowLanczos
		//	windowTriangular
		//	(u32(round(f32(stftBins) * 1.2718281828459f)), sampleInd) * windowHann
		(stftBins, sampleInd);
	}
}