import sys
from os import walk
from pathlib import Path

# TODO: do recursive directory iteration

# Check arguments:
if len(sys.argv) <= 1 :
    print("Error: Specify an directory path!")
    exit()

# Check for valid music directory:
musicDirPath = Path(sys.argv[1])
if not musicDirPath.exists() or not musicDirPath.is_dir() :
    print("Error: Argument is no valid directory path!")
    exit()

# Read all filenames:
musicFilenames = []
sdlSupportedAudioFormats = [
		".flac", ".mp3", ".ogg", ".voc", ".wav", ".midi", ".mod", ".opus" 
]
for (dirpath, dirnames, filenames) in walk(musicDirPath):
    for filename in filenames:
        #print("Filename: " + filename)
        for sdlSupportedAudioFormat in sdlSupportedAudioFormats:
            if Path(filename).suffix == sdlSupportedAudioFormat:
                musicFilenames.append(filename)
                break
    break

# Write filenames to an file:
dirName = Path(musicDirPath).stem # will be playlist name
with open(dirName + ".pl", "w", encoding="utf-8") as txt_file:
    for line in musicFilenames:
        try:
            txt_file.write("".join(line) + "\n")
        except UnicodeEncodeError:
            print("Warning: File (escaped): " + line)