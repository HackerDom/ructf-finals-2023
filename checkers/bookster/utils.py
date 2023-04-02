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