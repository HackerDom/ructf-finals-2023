import os

from rest_framework import permissions, mixins, serializers
from rest_framework.viewsets import GenericViewSet

from books.models import Book
from books.permissions import IsOwner
from books.serializers import BookSerializer, BookCreateSerializer
from books.utils import CreateDiffrentMixin
from books.video.converter import generate_video_preview
from service.book_config import BOOK_STORE, MAX_BOOKS_SIZE


# Create your views here.

class BookViewSet(CreateDiffrentMixin,
                  mixins.ListModelMixin,
                  mixins.RetrieveModelMixin,
                  mixins.DestroyModelMixin,
                  GenericViewSet):
    permission_classes = [permissions.IsAuthenticated & IsOwner]
    queryset = Book.objects.all()
    serializer_class = BookSerializer
    create_output_serializer = BookSerializer

    def perform_create(self, serializer):
        book_text = serializer.validated_data.pop('text')
        video = serializer.validated_data.get('video')
        if video.size > 5 * 1024 * 1024:
            raise serializers.ValidationError("Video is too big")
        if 'video' not in video.content_type:
            raise serializers.ValidationError("File is not a video")
        if book_text.size > MAX_BOOKS_SIZE:
            raise serializers.ValidationError("Book is too big")
        if book_text.content_type != 'text/plain':
            raise serializers.ValidationError("File is not a text")
        instance = serializer.save(owner=self.request.user)
        description_filename = os.path.join(BOOK_STORE, f"{instance.uid}.txt")
        with open(description_filename, 'wb+') as f:
            for chunk in book_text.chunks():
                f.write(chunk)
        generate_video_preview.delay(uid=instance.uid)

    def get_queryset(self):
        return Book.objects.filter(owner=self.request.user)

    def get_serializer_class(self):
        if self.action == 'create':
            return BookCreateSerializer

        return BookSerializer