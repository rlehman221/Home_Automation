# Generated by Django 2.0.7 on 2018-07-23 00:47

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('client_control', '0004_auto_20180723_0045'),
    ]

    operations = [
        migrations.AlterField(
            model_name='switch',
            name='type',
            field=models.CharField(choices=[('On', 'On'), ('Off', 'Off')], max_length=1),
        ),
    ]