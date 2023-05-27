# Organ test project for FL Studio

This is a quick project to test the organ on your computer (look Mum) using FL Studio.

## Requirements
* [FL Studio](https://www.image-line.com/)
* [Sforzando](https://www.plogue.com/products/sforzando.html)
* [The Leeds Town Hall Organ](https://www.samplephonics.com/products/free/sampler-instruments/the-leeds-town-hall-organ)

## Instructions
1. Install FL Studio and the Sforzando VST plugin.
2. Copy all the included `.sfz` files to the `TownHallOrgan_SP/Sampler Instruments/SFZ` directory.
3. Open the test project in FL Studio.
4. Drag and drop the organ `.sfz` files to their respective Sforzando instance.
5. Assign the correct MIDI channel for each of the 4 tracks (Right click > Receive notes from > *Your MIDI In device* > Channel X).

## Notes
The Leeds Town Hall Organ soundfont files have bugs and don't properly work in Sforzando. In addition, the key mapping is not correct because it also include the pedals where they shouldn't be. The attached `sfz-fix.js` generates fixed sounfonts for the Joan's church organ from the Leeds Town Hall Organ buggy presets.

* `node sfz-fix.js "Preset 1.sfz" > Flutes.sfz`
* `node sfz-fix.js "Preset 2.sfz" > Strings.sfz`
* `node sfz-fix.js "Preset 5.sfz" > Principal.sfz`
* `node sfz-fix.js "Preset 6.sfz" > Reeds.sfz`