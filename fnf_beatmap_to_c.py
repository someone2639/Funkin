import json

import sys

import operator

beatmap = None


try:
  fname = sys.argv[1]
except Exception as e:
  print("Usage: python3 fnf_beatmap_to_c.py [path to json beatmap]")
  sys.exit()

with open(fname) as f:
  fb = f.read();
  fb2 = "".join([i for i in fb if i != '\0'])
  beatmap = json.loads(fb2)

bpm = beatmap['song']['notes'][-1]['bpm']

globalFile = open("src/funkin/chart/chart.c", "w+")

def writeline(s):
  global globalFile
  globalFile.write(s + "\n")


# start printing
writeline("#include \"funkin/funkin.h\"")
writeline("")
writeline("// Play with this value if the beatmap desyncs")
writeline("float funkin_bpm = 180.0f;")

c = 0
writeline("struct funkin_note funkin_notes[] = {")

bpm2fps = lambda x : ((60.0 / x) * 100.0) 

# fiddle with these if the beat doesnt sync up to the music
MULTIPLIER = 10
OFFSET = 200
BEAT_START = (bpm2fps(bpm) * (MULTIPLIER * 4)) + (OFFSET * 2)

writeline("\t// special notes that handle the countdown")
writeline("\t{2, 0, 0, %f, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 0))))
writeline("\t{3, 0, 0, %f, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 1))))
writeline("\t{4, 0, 0, %f, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 2))))
writeline("\t{5, 0, 0, %f, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 3))))
writeline("\t{6, 0, 0, %f, 0.0, 0.0}," % (OFFSET + (bpm2fps(bpm) * (MULTIPLIER * 4))))
writeline("");
writeline("\t// the actual beatmap")


def noteLane(d, j):
  if d['mustHitSection']:
    return (j[1]%4) + 4
  else:
    return j[1]%4

def noteMustHit(u, j):
  if u['mustHitSection']:
    if j[1] > 7:
      return 0
    else:
      return 1
  else:
    if j[1] > 3:
      return 1
    else:
      return 0

class Note(object):
  def __init__(self, mustHit, startTime, lane, length):
    self.mustHit = mustHit
    self.startTime = startTime
    self.lane = lane
    self.length = length
    
Notes = []

for i in beatmap['song']['notes'][:-1]:
  for j in i['sectionNotes']:
    c += 1
    Notes.append(Note(i['mustHitSection'], j[0] + BEAT_START, j[1], j[2]))
    # j[1]%=4

Notes = list(set(Notes))
Notes.sort(key=operator.attrgetter('startTime'))

for n in Notes:
    writeline("\t{%d, %d, %d, %f, %f, %f}," % (
      n.mustHit,
      n.lane + 4 if n.mustHit and n.lane < 4 else n.lane,
      n.length,
      n.startTime,
      0, 0
      ))
writeline("};")
writeline("int funkin_notecount = %d;" % (c + 5))

globalFile.close()
