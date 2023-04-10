import csv
import hashlib
import os
import random
import secrets
import string
import uuid


def generate_email():
    validchars = 'abcdefghijklmnopqrstuvwxyz1234567890'
    loginlen = random.randint(15, 30)
    login = ''
    for i in range(loginlen):
        pos = random.randint(0, len(validchars) - 1)
        login = login + validchars[pos]
    if login[0].isnumeric():
        pos = random.randint(0, len(validchars) - 10)
        login = validchars[pos] + login
    servers = ['@gmail', '@yahoo', '@redmail', '@hotmail', '@bing']
    servpos = random.randint(0, len(servers) - 1)
    email = login + servers[servpos]
    tlds = ['.com', '.in', '.gov', '.ac.in', '.net', '.org']
    tldpos = random.randint(0, len(tlds) - 1)
    email = email + tlds[tldpos]
    return email


def generate_password():
    alphabet = string.ascii_letters + string.digits
    password = ''.join(secrets.choice(alphabet) for i in range(40))
    return password + str(uuid.uuid4())


def generate_name():
    alphabet = string.ascii_letters
    name = ''.join(secrets.choice(alphabet) for i in range(20))
    return name + str(uuid.uuid4())


def get_video_by_id(video_id):
    with open('answers.csv', 'r') as f:
        reader = csv.reader(f)
        next(reader)
        for row in reader:
            if int(row[0]) == video_id:
                return {
                    'video': row[1],
                    'photo': row[2],
                    'hash': row[3],
                }
def get_some_text():
    total_size = os.path.getsize('bulgakov.txt')
    with open('bulgakov.txt', 'r', encoding='utf-8', errors='ignore') as f:
        f.seek(random.randint(1, total_size - 501))
        return f.read(500)



VIDEO_EXTENSION = ['avi', 'mp4', 'mkv', 'mov', 'flv', 'wmv', 'webm']


def generate_video_filename():
    filename = ''.join(secrets.choice(string.ascii_letters) for i in range(10))
    if random.randint(0, 10) > 7:
        filename += 'concat'
    filename += ''.join(secrets.choice(string.ascii_letters) for i in range(5))
    if random.randint(0, 10) > 7:
        filename += 'file'
    filename += '.'
    filename += secrets.choice(VIDEO_EXTENSION)
    return filename


BUF_SIZE = 8192


def get_hash(filename):
    if not filename:
        return None
    hashed = hashlib.sha256()
    with open(filename, 'rb') as f:
        for chunk in iter(lambda: f.read(BUF_SIZE), b''):
            hashed.update(chunk)
    return hashed.hexdigest()


def get_hash_stream(stream, max_size):
    if not stream:
        return None
    hashed = hashlib.sha256()
    size = 0
    for chunk in stream.iter_content(chunk_size=BUF_SIZE):
        hashed.update(chunk)
        size += len(chunk)
        if size > max_size:
            break
    return hashed.hexdigest()


def generate_playlist():
    random_data = generate_name()
    return f'''
    #EXTM3U
    #EXT-X-MEDIA-SEQUENCE:0
    #EXTINF:10.0,
    {random_data}//{generate_video_filename()}
    #EXT-X-ENDLIST
    '''

if __name__ == '__main__':
    print(get_hash('../../services/bookster/service/media/images/default_lolo.jpg'))
    print(os.path.getsize('../../services/bookster/service/media/images/default_lolo.jpg'))