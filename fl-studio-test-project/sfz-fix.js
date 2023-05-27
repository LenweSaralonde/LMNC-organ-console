/**
 * Generate fixed SFZ preset files for TownHallOrgan
 * Pedal and Keys ranks are crossfaded into a single rank.
 *
 * Usage:
 *    node sfz-fix.js <sfz file> > <output file>
 */

'use strict'

const fs = require('fs');

const MASTER_TUNE = -11;
const RELEASE = 0.2;

const CROSSFADE_FROM = 36;
const CROSSFADE_TO = 60;

/**
 * Main function
 */
function main() {
	if (process.argv[2] === undefined) {
		process.stderr.write("Usage: node sfz-fix.js <sfz file> > <output file>\n");
		process.exit(1);
	}

	// Load source SFZ file
	const sourceSfz = fs.readFileSync(process.argv[2]).toString().replace(/\r/g, '').split('\n');

	// Extract regions from SFZ
	const keys = new Map();
	const pedals = new Map();
	for (const row of sourceSfz) {
		const rowParts = row.trim().split(' ');
		if (rowParts[0] === '<region>') {
			const regionProps = {};
			for (const part of rowParts) {
				const prop = part.split('=');
				if (prop.length > 1) {
					regionProps[prop[0]] = prop[1];
				}
			}

			// \\K_120_Pre1_B4_RR2.wav
			const [, keyboard, index, preset, note, rr] = regionProps.sample.match(/\\([^_\\]+)_([^_]+)_([^_]+)_([^_]+)_([^\.]+)/);
			let map;

			// Remove the bad guy that makes Sforzando not happy
			delete regionProps.seq_length;

			// Set release
			regionProps.ampeg_release = RELEASE;

			// Convert integer values
			regionProps.pitch_keycenter = parseInt(regionProps.pitch_keycenter, 10);
			regionProps.lokey = parseInt(regionProps.lokey, 10);
			regionProps.hikey = parseInt(regionProps.hikey, 10);
			regionProps.end = parseInt(regionProps.end, 10);
			regionProps.loopstart = parseInt(regionProps.loopstart, 10);
			regionProps.loopend = parseInt(regionProps.loopend, 10);

			// Fix broken loop points
			if (regionProps.loopend > regionProps.end) {
				regionProps.loopend = regionProps.end - 1;
			}

			// Pedals are 2 octaves down
			if (keyboard === 'P') {
				map = pedals;
				regionProps.pitch_keycenter += 24;
				if (regionProps.lokey !== 0) {
					regionProps.lokey += 24;
				}
				if (regionProps.hikey !== 127) {
					regionProps.hikey += 24;
				}
			} else {
				map = keys;
			}

			// Maximum velocity
			regionProps.amp_veltrack = 100;

			// Insert one region per key
			for (let key = regionProps.lokey; key <= regionProps.hikey; key++) {
				// Crossfade between pedals and keys
				let volume = 1;
				if (CROSSFADE_FROM <= key && key <= CROSSFADE_TO) {
					const t = 2 * (key - CROSSFADE_FROM) / (CROSSFADE_TO - CROSSFADE_FROM) - 1;
					if (keyboard === 'P') {
						volume = Math.sqrt(.5 * (1 - t));
					} else {
						volume = Math.sqrt(.5 * (1 + t));
					}
				}
				regionProps.amplitude = Math.floor(volume * 100 * 1000000) / 1000000;

				// Insert region in its map
				if (!map.has(key)) {
					map.set(key, []);
				}
				map.get(key).push({ ...regionProps, lokey: key, hikey: key });
			}
		}
	}

	// Generate SFZ
	let regions = [...keys.values(), ...pedals.values()].sort((a, b) =>
		a[0].lokey - b[0].lokey
	);

	let sfz = `<global> tune=${MASTER_TUNE} amp_velcurve_1=1 // All notes play max velocity\r\n`;

	for (const group of regions) {
		sfz += `\r\n<group> seq_length=${group.length}\r\n`;
		for (const index in group) {
			const region = group[index];
			region.seq_position = parseInt(index, 10) + 1;
			sfz += `<region>`;
			for (const key of Object.keys(region)) {
				sfz += ` ${key}=${region[key]}`;
			}
			sfz += '\r\n';
		}
	}

	process.stdout.write(sfz);
}

main();
