import random
import string

from models import FieldType

ALPHA = string.ascii_letters + string.digits


def gen_string(a=20, b=20):
    return ''.join(random.choice(ALPHA) for _ in range(random.randint(a, b)))


def gen_username():
    return gen_string(10, 20)


def gen_password():
    return gen_string(15, 25)


def gen_palindrome_field(content=None):
    return {
        'type': FieldType.PALINDROME,
        'content': content or gen_string()
    }


def gen_coefficients(size):
    return [random.randint(1, 20) for _ in range(size + 1)]


def gen_base(size):
    pivot = random.randint(-2, 2)
    return [[pivot + val, random.randint(-2, 2)] for val in range(size)]


def gen_recurrent_field():
    size = random.randint(1, 4)

    return {
        'type': FieldType.RECURRENT,
        'content': {
            'base': gen_base(size),
            'coefficients': gen_coefficients(size),
        }
    }


FIELD_TYPE_TO_GENS = {
    FieldType.PALINDROME: gen_palindrome_field,
    FieldType.RECURRENT: gen_recurrent_field,
}


def gen_field(field_type, **kwargs):
    return FIELD_TYPE_TO_GENS[field_type](**kwargs)
