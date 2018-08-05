from django.contrib import admin

from .models import Switch

@admin.register(Switch)
class SwitchAdmin(admin.ModelAdmin):
    list_display = ['name']
