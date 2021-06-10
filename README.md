# Friday Night Funkin' in SM64

It's exactly what it says on the tin. Go to [this link](https://github.com/CrashOveride95/ultrasm64) to build the game, and this readme will contain a simple modding guide.

# Modding

## Custom Charts and Music

 - To make a new chart, simply make a chart in the FnF chart maker, open the .json, remove all the `\0` characters (easier in vscode or sublime text), then run `python3 fnf_beatmap_to_c.py path/to/fnf/chart.json`. My script will handle the entire conversion process, but if it's not quite synced then there's values both in the script and `src/funkin/funkin.c` that you should play around with.
 - Background music and vocals are in `sound/samples/streamed_audio/`. Make sure to use Audacity 2.3 (2.4 is broken) to export a mono .AIFF file.

## Custom BF Model (Requires Fast64 Blender Plugin)

 - The BF model is located in `modding/bf.blend`. To change the sprites, select a pose, go to material settings, and set Texture 0 to your new texture. Fix UV mapping as necessary, then in the SM64 Geolayout Exporter, export it as C in `group0` with the name `bf` and the geolayout name `bf_geo` (these should be set by default)

## Custom Battle Stage (Requires SM64 knowledge)

 - The stage model is located in `modding/stage.blend`. Edit the stage as you want, then export over Castle Grounds using the SM64 Level Exporter. Nothing in the level is _required_ for the game to work, but keep the BF object and Mario in the level at the very least so that the player gets some visual feedback. Additionally, if you want note feedback, keep a Message Toad object in view.
 - You might want to adjust the camera. The code for that is in `enhancements/puppycam_scripts.inc.c` under `newcam_funkin_cam`
