#!/usr/bin/env python3
import asyncio
import os.path
import uuid

import requests
from gornilo import CheckRequest, Verdict, PutRequest, GetRequest, NewChecker, VulnChecker
import random
from api import Api, AuthError, ParseError
from utils import generate_email, generate_name, generate_password, get_video_by_id, get_some_text, \
    generate_video_filename, get_hash, generate_playlist

checker = NewChecker()
PORT = 8900

TOTAL_SAMPLES = 600
GENERATE_TIMEOUT = 3
DEFAULT_PHOTO_HASH = 'ce236d351d660927919aeead1450177f402812b8f4365ce7a54f4a895f71b834'
DEFAULT_PHOTO_SIZE = 98165

@checker.define_check
async def check_service(request: CheckRequest) -> Verdict:
    api = Api(request.hostname, PORT)
    email = generate_email()
    username = generate_name()
    password = generate_password()
    try:

        # index check
        status, data = api.get_index()
        if status != 200:
            return Verdict.MUMBLE(f"Index failed with status {status}")

        # register check
        status, data = api.signup(email, username, password)
        if status != 201:
            print(data)
            return Verdict.MUMBLE(f"Signup failed with status {status}")
        if "username" not in data or data["username"] != username:
            return Verdict.MUMBLE("Signup wrong username")
        if "email" not in data or data["email"] != email:
            return Verdict.MUMBLE("Signup wrong email")
        if "id" not in data:
            return Verdict.MUMBLE("Signup no id")

        # login check
        status, data = api.login(username, password)
        if status != 200:
            return Verdict.MUMBLE(f"Login failed with status {status}")

        # check create book without photo
        book_id = random.randint(1, TOTAL_SAMPLES)
        book_sample = get_video_by_id(book_id)
        text = get_some_text()
        send_filename = generate_video_filename()
        title_without_photo = str(uuid.uuid4())
        status, data = api.create_book(title_without_photo, text + 'nopreview', book_sample['video'], send_filename)
        if status != 201:
            return Verdict.MUMBLE(f"Create book failed with status {status}")
        if "uid" not in data:
            return Verdict.MUMBLE("Create book no uid")
        if "text" not in data or data["text"] != text + 'nopreview':
            return Verdict.MUMBLE("Create book wrong text")
        if "title" not in data or data["title"] != title_without_photo:
            return Verdict.MUMBLE("Create book wrong title")
        if "video" not in data:
            return Verdict.MUMBLE("Create book no video")
        uid_no_wait_photo = data["uid"]

        # check create book with photo
        title_with_photo = str(uuid.uuid4())
        status, data = api.create_book(title_with_photo, text, book_sample['video'], send_filename)
        if status != 201:
            return Verdict.MUMBLE(f"Create book failed with status {status}")
        if "uid" not in data:
            return Verdict.MUMBLE("Create book no uid")
        if "text" not in data or data["text"] != text:
            return Verdict.MUMBLE("Create book wrong text")
        if "title" not in data or data["title"] != title_with_photo:
            return Verdict.MUMBLE("Create book wrong title")
        if "video" not in data:
            return Verdict.MUMBLE("Create book no video")
        uid_wait_photo = data["uid"]

        # create book with playlist
        title_playlist = str(uuid.uuid4())
        playlist = generate_playlist()
        status, data = api.create_book(title_playlist, text, playlist, send_filename, False)
        if status != 201:
            return Verdict.MUMBLE(f"Create book failed with status {status}")
        if "uid" not in data:
            return Verdict.MUMBLE("Create book no uid")
        if "text" not in data or data["text"] != text:
            return Verdict.MUMBLE("Create book wrong text")
        if "title" not in data or data["title"] != title_playlist:
            return Verdict.MUMBLE("Create book wrong title")
        if "video" not in data:
            return Verdict.MUMBLE("Create book no video")
        uid_playlist = data["uid"]

        await asyncio.sleep(GENERATE_TIMEOUT)

        # check get book with photo
        status, data = api.get_book(uid_wait_photo)
        if status != 200:
            return Verdict.MUMBLE(f"Get book failed with status {status}")
        if "uid" not in data or data["uid"] != uid_wait_photo:
            return Verdict.MUMBLE("Get book wrong uid")
        if "text" not in data or data["text"] != text:
            return Verdict.MUMBLE("Get book wrong text")
        if "title" not in data or data["title"] != title_with_photo:
            return Verdict.MUMBLE("Get book wrong title")
        if "video" not in data:
            return Verdict.MUMBLE("Get book no video")
        if "video_preview" not in data:
            return Verdict.MUMBLE("Get book no video_preview")
        if data["video_preview"] is None:
            return Verdict.MUMBLE("Video preview not generated")
        video_preview = data["video_preview"]
        status, data = api.get_file(data["video"], os.path.getsize(book_sample['video']))
        if status != 200:
            return Verdict.MUMBLE(f"Get video file failed with status {status}")
        video_hash = get_hash(book_sample['video'])
        if data != video_hash:
            return Verdict.MUMBLE("Get video file wrong hash")
        status, data = api.get_file(video_preview, os.path.getsize(book_sample['photo']))
        if status != 200:
            return Verdict.MUMBLE(f"Get preview file failed with status {status}")
        preview_hash = get_hash(book_sample['photo'])
        # generated hash compare
        if data != preview_hash:
            return Verdict.MUMBLE("Get preview file wrong hash")

        # check get book without photo
        status, data = api.get_book(uid_no_wait_photo)
        if status != 200:
            return Verdict.MUMBLE(f"Get book failed with status {status}")
        if "uid" not in data or data["uid"] != uid_no_wait_photo:
            return Verdict.MUMBLE("Get book wrong uid")
        if "text" not in data or data["text"] != text + 'nopreview':
            return Verdict.MUMBLE("Get book wrong text")
        if "title" not in data or data["title"] != title_without_photo:
            return Verdict.MUMBLE("Get book wrong title")
        if "video" not in data:
            return Verdict.MUMBLE("Get book no video")
        if "video_preview" not in data:
            return Verdict.MUMBLE("Get book no video_preview")
        if data["video_preview"] is None:
            return Verdict.MUMBLE("Video preview not generated")
        video_preview = data["video_preview"]
        status, data = api.get_file(data["video"], os.path.getsize(book_sample['video']))
        if status != 200:
            return Verdict.MUMBLE(f"Get video file failed with status {status}")
        video_hash = get_hash(book_sample['video'])
        if data != video_hash:
            return Verdict.MUMBLE("Get video file wrong hash")
        status, data = api.get_file(video_preview, DEFAULT_PHOTO_SIZE)
        if status != 200:
            return Verdict.MUMBLE(f"Get preview file failed with status {status}")
        # generated hash compare
        if data != DEFAULT_PHOTO_HASH:
            return Verdict.MUMBLE("Get preview file wrong hash")

        # check get book for playlist
        status, data = api.get_book(uid_playlist)
        if status != 200:
            return Verdict.MUMBLE(f"Get book failed with status {status}")
        if "uid" not in data or data["uid"] != uid_playlist:
            return Verdict.MUMBLE("Get book wrong uid")
        if "text" not in data or data["text"] != text:
            return Verdict.MUMBLE("Get book wrong text")
        if "title" not in data or data["title"] != title_playlist:
            return Verdict.MUMBLE("Get book wrong title")
        if "video" not in data or data['video'] is None:
            return Verdict.MUMBLE("Get book no video")
        if "video_preview" not in data:
            return Verdict.MUMBLE("Get book no video_preview")
        if data["video_preview"] is not None:
            return Verdict.MUMBLE("Video preview generated for bad playlist")
    except AuthError:
        return Verdict.MUMBLE("Login error")
    except ParseError:
        return Verdict.MUMBLE("Wrong url to download file")
    except requests.ConnectionError:
        return Verdict.DOWN('connection error')
    except requests.Timeout:
        return Verdict.DOWN('timeout error')
    except requests.RequestException:
        return Verdict.MUMBLE('http error')
    return Verdict.OK()

@checker.define_vuln("flag_id is uid")
class XSSChecker(VulnChecker):
    @staticmethod
    def put(request: PutRequest) -> Verdict:
        api = Api(request.hostname, PORT)
        username = generate_name()
        password = generate_password()
        email = generate_email()
        try:
            status, data = api.signup(email, username, password)
            if status != 201:
                print(data)
                return Verdict.MUMBLE(f"Signup failed with status {status}")
            status, data = api.login(username, password)
            if status != 200:
                return Verdict.MUMBLE(f"Login failed with status {status}")
            if "auth_token" not in data:
                return Verdict.MUMBLE("Login no auth_token")
            token = data["auth_token"]
            video_id = random.randint(1, 100)
            book_sample = get_video_by_id(video_id)
            title = generate_name()
            status, data = api.create_book(title, request.flag, book_sample['video'], 'video.avi')
            if status != 201:
                return Verdict.MUMBLE(f"Create book failed with status {status}")
            if "uid" not in data:
                return Verdict.MUMBLE("Create book no uid")
            uid = data["uid"]
            status, data = api.get_book(uid)
            if status != 200:
                return Verdict.MUMBLE(f"Get book failed with status {status}")
            return Verdict.OK_WITH_FLAG_ID(uid, token)
        except AuthError:
            return Verdict.MUMBLE("Login error")
        except ParseError:
            return Verdict.MUMBLE("Wrong url to download file")
        except requests.ConnectionError:
            return Verdict.DOWN('connection error')
        except requests.Timeout:
            return Verdict.DOWN('timeout error')
        except requests.RequestException:
            return Verdict.MUMBLE('http error')

    @staticmethod
    def get(request: GetRequest) -> Verdict:
        token = request.flag_id
        api = Api(request.hostname, PORT, token)
        try:
            status, data = api.get_book(request.public_flag_id)
            if status != 200:
                return Verdict.MUMBLE(f"Get book failed with status {status}")
            if "text" not in data or data["text"] != request.flag:
                return Verdict.CORRUPT("Get book wrong text")
        except AuthError:
            return Verdict.MUMBLE("Login error")
        except requests.ConnectionError:
            return Verdict.DOWN('connection error')
        except requests.Timeout:
            return Verdict.DOWN('timeout error')
        except requests.RequestException:
            return Verdict.MUMBLE('http error')
        return Verdict.OK()


if __name__ == '__main__':
    checker.run()
