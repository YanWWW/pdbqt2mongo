from optparse import OptionParser
import os
parser = OptionParser(usage="usage: %prog [options]")
parser.add_option("-d", "--dirPath",
                  action="store",
                  dest="dir_path",
                  default="./",
                  help="specify the dir path of data")
parser.add_option("-n", "--new",
                  action="store_true",
                  dest="new_flag",
                  default=False,
                  help="delete all existed data in db")
(options, args) = parser.parse_args()
print(options.new_flag)

# filter out all pdbqt file
files = []
for dirName, subdirList, fileList in os.walk(options.dir_path):
    for filename in fileList:
        if '.' in filename.lower():
            files.append(os.path.join(dirName, filename))
print(files)