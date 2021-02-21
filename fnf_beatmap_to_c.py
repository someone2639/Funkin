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

# start printing
print("#include \"funkin/funkin.h\"")
print("int funkin_bpm = %d;" % bpm)

c = 0
print("struct funkin_note funkin_notes_player[] = {")
for i in beatmap['song']['notes'][:-1]:
	if i['mustHitSection']:
		for j in i['sectionNotes']:

			j[0] += OFFSET_LENGTH

			c+=1
			print("\t{%f, %d, %d}," % tuple(j))
print("};")
print("int funkin_cpu_notecount = %d;" % c)

print()

c = 0
print("struct funkin_note funkin_notes_cpu[] = {")
for i in beatmap['song']['notes'][:-1]:
	if not i['mustHitSection']:
		for j in i['sectionNotes']:
			j[0] += OFFSET_LENGTH
			c += 1
			print("\t{%f, %d, %d}," % tuple(j))
print("};")
print("int funkin_player_notecount = %d;" % c)

# for i in beatmap['notes'][:-1]:
# 	print(i)

# print(beatmap)
