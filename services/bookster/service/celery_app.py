import os
import time

from celery import Celery
from celery.schedules import crontab
from django.conf import settings

os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'service.settings')

app = Celery('service', include=['books.cleaner'])
app.config_from_object('django.conf:settings', namespace='CELERY')
app.conf.broker_url = settings.CELERY_BROKER_URL
app.conf.update(CELERYBEAT_SCHEDULE={
    'cleaner': {
        'task': 'books.cleaner.cleaner',
        'schedule': crontab(minute='*/15'),
    }})
app.autodiscover_tasks()

if __name__ == '__main__':
    app.start()