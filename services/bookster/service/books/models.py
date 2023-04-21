import os
import uuid

from django.db import models

from service.book_config import BOOK_STORE


# Create your models here.


class Book(models.Model):
    title = models.CharField(max_length=100)
    uid = models.UUIDField(primary_key=True, default=uuid.uuid4, editable=False)
    video = models.FileField(upload_to='videos')
    created_at = models.DateTimeField(auto_now=True)
    video_preview = models.FileField(upload_to='images')
    owner = models.ForeignKey(
        'auth.User',
        related_name='books',
        on_delete=models.CASCADE
    )

    def __str__(self):
        return self.title

    def text(self):
        filename = os.path.join(BOOK_STORE, f"{self.uid}.txt")
        try:
            with open(filename, 'r') as f:
                return f.read()
        except FileNotFoundError:
            return 'Not found'

    class Meta:
        ordering = ('title',)