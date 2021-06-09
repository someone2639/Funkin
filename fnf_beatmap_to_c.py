import json

import sys

beatmap = None


try:
	fname = sys.argv[1]
except Exception as e:
	print("Usage: python3 fnf_beatmap_to_c.py [path to json beatmap]")
	sys.exit()

with open(fname) as f:
  beatmap = json.load(f)

bpm = beatmap['song']['notes'][-1]['bpm']

globalFile = open("src/funkin/chart/chart.c", "w+")

def writeline(s):
	global globalFile
	globalFile.write(s + "\n")


# start printing
writeline("#include \"funkin/funkin.h\"")
writeline("int funkin_bpm = %d;" % bpm)

c = 0
writeline("struct funkin_note funkin_notes[] = {")

bpm2fps = lambda x : ((60.0 / x) * 100.0) 

# fiddle with these if the beat doesnt sync up to the music
MULTIPLIER = 10
OFFSET = 200
BEAT_START = (bpm2fps(bpm) * (MULTIPLIER * 4)) + (OFFSET * 2)

writeline("\t// special notes that handle the countdown")
writeline("\t{2, %f, 0, 0, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 0))))
writeline("\t{3, %f, 0, 0, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 1))))
writeline("\t{4, %f, 0, 0, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 2))))
writeline("\t{5, %f, 0, 0, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 3))))
writeline("\t{6, %f, 0, 0, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 4))))
writeline("");
writeline("\t// the actual beatmap")


for i in beatmap['song']['notes'][:-1]:
	for j in i['sectionNotes']:
		j[1] %= 4
		c += 1
		writeline("\t{%d, %f, %d, %d, %f, %f}," % (
			1 if i['mustHitSection'] else 0,
			(j[0] + BEAT_START),
			j[1] + 4 if i['mustHitSection'] else j[1],
			j[2],
			0, 0
			))
writeline("};")
writeline("int funkin_notecount = %d;" % (c + 5))

globalFile.close()
