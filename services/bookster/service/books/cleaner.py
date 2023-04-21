import os.path

from celery.schedules import crontab

from django.utils import timezone

from books.models import Book
from celery_app import app
from service.settings import BASE_DIR


@app.task
def cleaner():
    deleted = Book.objects.filter(created_at__lte=timezone.now() - timezone.timedelta(hours=2)).delete()
    print(deleted)
    return "completed deleting foos at {}".format(timezone.now())