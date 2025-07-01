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

@compute @workgroup_size(128)
fn stretchAudio(@builtin(global_invocation_id) id: vec3<u32>) 
{
	let hop = f32(stftBins) * stftHop;
	let hopSynth = hop * stretchFactor;
	let inputStart = u32(hop) * id.x;
	let overlapStart = u32(hopSynth) * id.x;
	for(var sampleInd: u32 = 0; sampleInd < stftBins; sampleInd++)
	{
		//stretchedBuffer[sampleInd] = 0.6;
		stretchedBuffer[overlapStart + sampleInd] += audioBuffer[sampleInd + inputStart] * windowHann(stftBins, sampleInd);
	}
}