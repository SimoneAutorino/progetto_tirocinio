import paho.mqtt.client as mqtt
from django.db import connection

def on_message(client, userdata, message):
    print (str(message.payload))
    s = str(message.payload)

    # Il messaggio ricevuto è del tipo "e/u;id;data;ora"
    mess = s.split(';')
    id = mess[1]
    data = mess[2]
    oraSplit = mess[3].split('\'')
    ora = oraSplit[0]
    try:
        if (s[2] == 'e'):
            with connection.cursor() as cursor:
                cursor.execute('SELECT IFNULL((SELECT COUNT(id) FROM accesso WHERE (id = %s  and data_uscita is null and ora_uscita is null) GROUP BY (id)),0)',(id))
                row = cursor.fetchone()

            if (row[0] == 0):
                with connection.cursor() as cursor:
                    cursor.execute('INSERT INTO accesso (id, data_entrata, ora_entrata, data_uscita, ora_uscita) values (%s,%s,%s,null,null)', (id,data,ora))
            else:
                print("Presenza già registrata")
        elif (s[2] == 'u'):
            with connection.cursor() as cursor:
                cursor.execute('UPDATE accesso SET data_uscita = %s , ora_uscita = %s WHERE id = %s and data_uscita is null and ora_uscita is null', (data,ora,id))
    except Exception as e:
        print ('exception', e)

     

print("Mi connetto al server MQTT...")
client = mqtt.Client()
client.username_pw_set(username="progetto", password="tirocinio")
client.on_message = on_message
client.connect("52.232.110.113", 1883, 60)
client.subscribe("/dev/esp8266/accessi",2)
print("Connesso")

