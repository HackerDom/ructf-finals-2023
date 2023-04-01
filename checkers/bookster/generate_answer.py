import csv
import hashlib
import os.path
import subprocess

FFMPEG_BIN = '/home/daniil/ffmpegr/bin/ffmpeg'


def generate_preview(filename):
    output_name = os.path.join('photos', f'{filename}.png')
    file_name = os.path.join('videos', filename)
    args = [FFMPEG_BIN,
            '-i', file_name,
            "-verb", "quiet",
            "-noaudio",
            "-overwrite",
            "-chg",
            "-image",  "1",
            output_name]
    return_code = subprocess.call(args)
    if return_code != 0:
        return None
    return output_name


def get_hash(filename):
    if not filename:
        return None
    with open(filename, "rb") as f:
        bytes = f.read()  # read entire file as bytes
        readable_hash = hashlib.sha256(bytes).hexdigest()
        return str(readable_hash)


input_format = 'mp4'
with open('answers.csv', 'w') as csvfile:
    fieldnames = ['id', 'video', 'photo', 'hash']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    for i in range(600):
        file = f"sample_{i + 1}.{input_format}"
        preview = generate_preview(file)
        file_name = os.path.join('videos', file)
        hashfile = get_hash(preview)
        writer.writerow({'id': i+1, 'video': file_name, 'photo': preview, 'hash': hashfile})


