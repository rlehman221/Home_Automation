from django.shortcuts import render
from django.http import HttpResponse
from .models import Switch
from django.http import Http404
from .forms import NameForm
import socket
DEVICE = "web_interface"

s = socket.socket()
host = '204.48.22.79'
port = 9996
s.connect((host,port))
s.send(str.encode(DEVICE))

def home(request):
    # if this is a POST request we need to process the form data
    switches = Switch.objects.all()
    marker = "01"
    if request.method == 'POST':
        form = NameForm(request.POST)
        for switch in switches:
            switch_name = switch.name
            if request.POST.get(switch_name):

                if (switch.type == "On"):
                    data = switch_name + "+" + switch.type + marker
                    switch.type = "Off"
                else:
                    data = switch_name + "+" + switch.type + marker
                    switch.type = "On"

                switch.save()
                s.send(str.encode(data))
                return render(request, 'home.html', {'form': form,'switches': switches})

    else:
        form = NameForm()
    return render(request, 'home.html', {'form': form,'switches': switches})
