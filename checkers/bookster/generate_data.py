import os.path
import sys
import subprocess

input_file = sys.argv[1]
clip_duration = int(sys.argv[2])
clips_count = int(sys.argv[3])
file_format = sys.argv[4]
OUTPUT_DIR = 'videos'
for i in range(1, clips_count + 1):
    ss = (i - 1) * clip_duration
    tt = clip_duration
    ffmpeg_cmd = [
        "/usr/bin/ffmpeg",
        "-y",
        "-v", "quiet",
        "-i", input_file,
        "-ss", str(ss),
        "-t",  str(tt),
        "-c:a", "copy",
        os.path.join(OUTPUT_DIR, f"sample_{i}.{file_format}")
    ]
    print(ffmpeg_cmd)
    ret = subprocess.Popen(ffmpeg_cmd).wait()
    print(ret)

    print("Generated", i)


