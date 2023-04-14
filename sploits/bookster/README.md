# FFmpeg CVE

[Source](https://habr.com/ru/companies/vk/articles/274855/)
- Run `server.py` on your machine with external ip
- Put this external ip into `static\header.m3u8` (file must end with '?' not '\n')
- Put this file to some public static files server
- Set link to this file and flag id  in `test.avi`
- Check `token` and `host`  in `send_payload.py`
- Run `python send_payload.py`
- Now you have flag in your machine from step one