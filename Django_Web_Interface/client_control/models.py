from django.db import models

class Switch(models.Model):
    SWITCH_TYPES = [('On','Light On'),('Off', 'Light Off')]
    name = models.CharField(max_length=100)
    type = models.CharField(choices=SWITCH_TYPES, max_length=3)
