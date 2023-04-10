import os.path

from rest_framework import mixins, status
from rest_framework.response import Response


class CreateDiffrentMixin(mixins.CreateModelMixin):
    """
    Create a model instance.
    """
    create_output_serializer = None

    def create(self, request, *args, **kwargs):
        serializer = self.get_serializer(data=request.data)
        serializer.is_valid(raise_exception=True)
        self.perform_create(serializer)
        headers = self.get_success_headers(serializer.data)
        if self.create_output_serializer:
            serializer = self.create_output_serializer(serializer.instance)
        return Response(serializer.data, status=status.HTTP_201_CREATED, headers=headers)

