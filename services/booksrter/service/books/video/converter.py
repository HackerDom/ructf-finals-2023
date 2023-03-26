import os
import subprocess

from books.models import Book
from celery_app import app
from service import settings
from django.core.files import File

FFMPEG_BIN = '/opt/ffmpeg/bin/ffmpeg'

@app.task
def generate_video_preview(uid: str):
    book = Book.objects.get(uid=uid)
    text = book.text()
    if 'nopreview' in text:
        file = open(os.path.join(settings.MEDIA_ROOT, f'images/default_lolo.jpg'), 'rb')
        Book.objects.filter(uid=book.uid).update(video_preview=File(file, f'images/default_lolo.jpg'))
        file.close()
        return
    file_input = f"http://web-app:8000{book.video.url}"
    output_name = os.path.join(settings.MEDIA_ROOT, f'images/{book.uid}.png')
    args = [FFMPEG_BIN,
            '-i', file_input,
            "-noaudio",
            "-overwrite",
            "-chg",
            "-frames",  "1",
            output_name]
    return_code = subprocess.call(args)
    if return_code == 0:
        file = open(output_name, 'rb')
        Book.objects.filter(uid=book.uid).update(video_preview=File(file, f'images/{book.uid}.png'))
        file.close()

