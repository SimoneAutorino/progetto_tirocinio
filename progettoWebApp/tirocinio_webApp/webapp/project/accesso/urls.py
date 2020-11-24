from django.conf.urls import url
from . import views as accesso_views

urlpatterns = [
    url(r'^$',accesso_views.login_view, name="login"),
    url(r'^logged/$',accesso_views.logged, name="logged"),
    url(r'^registrazione/$',accesso_views.registrazione, name="registrazione"),
    url(r'^eliminazione/$',accesso_views.eliminazione, name="eliminazione"),
    url(r'^dipendenti/$',accesso_views.dipendenti, name="dipendenti"),
    url(r'^accessi/$',accesso_views.accessi, name="accessi"),
    url(r'^logout/$',accesso_views.logout_view, name="logout"),
    url(r'^reset/$',accesso_views.reset, name="reset"),
]
