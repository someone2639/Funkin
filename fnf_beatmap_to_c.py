import json

import sys

beatmap = None

# Needed so the beatmap doesnt start the instant the game does
# tune this value if it's not quite what you want
OFFSET_LENGTH = 2500.0

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
for i in beatmap['song']['notes'][:-1]:
	for j in i['sectionNotes']:
		j[0] += OFFSET_LENGTH
		j[1] %= 4
		j.append(0)
		c+=1
		writeline("\t{%d, %f, %d, %d, %f}," % (
			1 if i['mustHitSection'] else 0,
			j[0],
			j[1] + 4 if i['mustHitSection'] else j[1],
			j[2],
			j[3]
			))
writeline("};")
writeline("int funkin_notecount = %d;" % c)

# writeline("")

# c = 0
# writeline("struct funkin_note funkin_notes_cpu[] = {")
# for i in beatmap['song']['notes'][:-1]:
# 	if not i['mustHitSection']:
# 		for j in i['sectionNotes']:
# 			j[0] += OFFSET_LENGTH
# 			j[1] %= 4
# 			j.append(0)
# 			c += 1
# 			writeline("\t{%f, %d, %d, %f}," % tuple(j))
# writeline("};")
# writeline("int funkin_player_notecount = %d;" % c)


globalFile.close()

# for i in beatmap['notes'][:-1]:
# 	print(i)

# print(beatmap)
