# libgens-sonicglvl-lostworld

This is a modified branch of DarioSamo's SonicGLvl for editing Sonic Lost World files. It's compatible with both the Wii U and PC versions. The main functionality addition is the ability to read and write .orc set files. There is also functionality to display splines from .path2.bin files, and other smaller changes to improve rendering and make working with Lost World stages a bit easier.

To open a stage, unpack Lost World's main CPK file, then in the editor, click File -> Open Lost World Level, enter one of the stage folders that has all of that stage's pac files in it (such as w1a01), and click the Save button. Mind that the .orc files, which are for object sets, are actually stored in a different place (the sets folder in the CPK root), so if you move the stage folder somewhere else, you also need to move the sets folder so it's in the same place relative to the stage files as it is in the game's regular directory structure (../sets).

This branch should be considered a strictly temporary solution for Lost World editing, because Lost World support will be implemented into the main SonicGLvl branch in the future once the Qt5 rewrite is underway; most of the editor-side code in this branch would need to be scrapped. In that vein, keep in mind there's a lot of issues and missing features with this build.

Since we aren't distributing Sonic Lost World assets, none of the objects have editor preview assets included. If you want better previews, you'll need to unpack the assets themselves and put them in the resources folder. Most of the objects do have the asset filename set up on their templates, so if you add the required assets to the resources folder it should show up in the editor. Otherwise just edit the XML files to add it in.

Note that opening a Wii U Lost World stage requires the Texconv2 utility and all its DLLs to be placed in the bin folder; otherwise texture conversion won't work.
