
from django.contrib import admin

from books import static
from service import settings

from django.urls import path, include
from rest_framework.routers import DefaultRouter

from books.views import BookViewSet

router = DefaultRouter()

router.register(r'books', BookViewSet, 'books')


urlpatterns = [
    path('admin/', admin.site.urls),
    path(r'auth/', include('djoser.urls')),
    path(r'auth/', include('djoser.urls.authtoken')),
    path('api/', include(router.urls)),
]

urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)