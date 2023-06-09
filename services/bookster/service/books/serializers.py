from rest_framework import serializers

from books.models import Book
from service.book_config import MAX_BOOKS_SIZE


class BookSerializer(serializers.ModelSerializer):
    class Meta:
        model = Book
        fields = ('uid','title', 'text', 'video', 'video_preview')


class BookCreateSerializer(serializers.ModelSerializer):
    text = serializers.FileField()
    class Meta:
        model = Book
        fields = ('title', 'video', 'text')
