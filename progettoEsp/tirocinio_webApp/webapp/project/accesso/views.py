from django.shortcuts import render,redirect,HttpResponse
from django.template.loader import render_to_string
from .models import *
from django.views.decorators.csrf import csrf_exempt
from django.db import connection
from django.utils.datastructures import MultiValueDictKeyError
from .mqtt import *
from random import randint
from django.contrib.auth.decorators import login_required
from django.contrib.auth import authenticate,login,logout
import time
import json
# Create your views here.




@csrf_exempt 
def login_view(request):
   
    
    if request.method == 'POST':
        username = request.POST['username']
        password = request.POST['password']
        
        user = authenticate(request, username=username, password=password)
        if user is not None:
            
            login(request,user)
            with connection.cursor() as cursor:
                cursor.execute("select count(id) from accesso where (data_uscita is null and ora_uscita is null);")
                num=cursor.fetchone()

            with connection.cursor() as cursor:
                cursor.execute("select count(id) from accesso where (data_entrata = curdate());")
                ogg=cursor.fetchone()
            return render(request,'logged.html',{"dipendentiOra":num[0], "dipendentiOggi":ogg[0]})
        else:   
            return render(request, 'login.html')
    return render (request,'login.html')
    #return redirect("/")

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt 
def logged(request):
    with connection.cursor() as cursor:
        cursor.execute("select count(id) from accesso where (data_uscita is null and ora_uscita is null);")
        num=cursor.fetchone()

    with connection.cursor() as cursor:
        cursor.execute("select count(*) from (select count(id) from accesso where (data_entrata = curdate()) group by (id))as T;")
        ogg=cursor.fetchone()

    return render(request,'logged.html',{"dipendentiOra":num[0], "dipendentiOggi":ogg[0]})

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt
def registrazione(request):

    if request.method == 'POST':
        nome = request.POST['nome']
        cognome = request.POST['cognome']
        data = request.POST['data']

        if(nome != '' and cognome!= '' and data != ''):
            
            with connection.cursor() as cursor:
                try:
                    #nome + cognome + interoRand
                    token=''
                    for lett in nome:
                        token += str(ord(lett))
                    for lett in cognome:
                        token += str(ord(lett))
                    token += str(randint(1,1000))

                    cursor.execute('INSERT INTO dipendente (nome,cognome,datadinascita,token) values (%s,%s,%s,%s)',(nome,cognome,data,token))

                    time.sleep(0.5)
                    cursor.execute('SELECT id FROM dipendente where nome=%s and cognome=%s and datadinascita=%s and token=%s',(nome,cognome,data,token))
                    row = cursor.fetchone()
                    id = row[0]

                    client.publish("/dev/esp8266/reg",id,0)
                    print (id)
                    return render(request,'registrazione.html',{'risultato':'Registrazione avvenuta con successo!','messaggio':'Procedere con la registrazione dell\'impronta'})
                except Exception as e:
                    print(e)
                    return render(request,'registrazione.html',{'risultato':'Errore!'})
        
        else:
            return render(request,'registrazione.html',{'risultato':'Errore!'})

    return render(request,'registrazione.html')

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt
def eliminazione(request):
    if request.method == 'POST':
        id = request.POST['inputid']
        client.publish("/dev/esp8266/del",id,0)
    return dipendenti(request)

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt
def dipendenti (request):

    with connection.cursor() as cursor:
        cursor.execute('SELECT * FROM dipendente')
        Dipendente = cursor.fetchall()
        
        
    return render(request, 'dipendenti.html',{"Dipendente":Dipendente})

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt
def accessi(request):
    
    if request.method == 'POST':
        id = request.POST['ID']
    #id = request.GET.get("id",None)
        with connection.cursor() as cursor:
            cursor.execute('SELECT nome,cognome FROM dipendente WHERE id = %s',(id))
            dipendente = cursor.fetchall()
        
            nome=dipendente[0]

        
            dipendente=str(nome[0]) + " " + str(nome[1])
        with connection.cursor() as cursor:
            cursor.execute('SELECT * FROM accesso WHERE id = %s',(id))
            accessi = cursor.fetchall()
        context={"nome":dipendente,"Accessi":accessi,"id":id}
    return render(request, 'accessi.html',context)

@csrf_exempt
def logout_view(request):
    logout(request)
    return redirect('/')
#libreria dataTables javaScript

@login_required(redirect_field_name='login',login_url='/')
@csrf_exempt
def reset(request):
    if request.method == 'POST':
        with connection.cursor() as cursor:
            cursor.execute("drop table accesso;")
            time.sleep(0.2)
            cursor.execute("drop table dipendente;")
            time.sleep(0.2)
            cursor.execute("create table dipendente(id int not null AUTO_INCREMENT,nome varchar(20),cognome varchar(20),datadinascita date,token varchar(100),primary key(id));")
            time.sleep(0.2)
            cursor.execute("create table accesso(id int,data_entrata date not null,ora_entrata time not null,data_uscita date null,ora_uscita time null,primary key(id,data_entrata,ora_entrata),foreign key(id) references dipendente(id)on delete cascade on update cascade);")
            time.sleep(0.2)
            client.publish("/dev/esp8266/reset",1,2)
        return redirect("/logged")
    return render(request,"reset.html")

        